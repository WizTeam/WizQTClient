/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_IndexFileDeleter_
#define _lucene_index_IndexFileDeleter_

#include "CLucene/util/Equators.h"
#include "IndexDeletionPolicy.h"

CL_CLASS_DEF(store,Directory)
CL_NS_DEF(index)
class SegmentInfos;
class DocumentsWriter;
class IndexDeletionPolicy;

/*
 * This class keeps track of each SegmentInfos instance that
 * is still "live", either because it corresponds to a 
 * segments_N file in the Directory (a "commit", i.e. a 
 * committed SegmentInfos) or because it's the in-memory SegmentInfos 
 * that a writer is actively updating but has not yet committed 
 * (currently this only applies when autoCommit=false in IndexWriter).
 * This class uses simple reference counting to map the live
 * SegmentInfos instances to individual files in the Directory. 
 * 
 * The same directory file may be referenced by more than
 * one IndexCommitPoints, i.e. more than one SegmentInfos.
 * Therefore we count how many commits reference each file.
 * When all the commits referencing a certain file have been
 * deleted, the refcount for that file becomes zero, and the
 * file is deleted.
 *
 * A separate deletion policy interface
 * (IndexDeletionPolicy) is consulted on creation (onInit)
 * and once per commit (onCommit), to decide when a commit
 * should be removed.
 * 
 * It is the business of the IndexDeletionPolicy to choose
 * when to delete commit points.  The actual mechanics of
 * file deletion, retrying, etc, derived from the deletion
 * of commit points is the business of the IndexFileDeleter.
 * 
 * The current default deletion policy is {@link
 * KeepOnlyLastCommitDeletionPolicy}, which removes all
 * prior commits when a new commit has completed.  This
 * matches the behavior before 2.2.
 *
 * Note that you must hold the write.lock before
 * instantiating this class.  It opens segments_N file(s)
 * directly with no retry logic.
 */
class IndexFileDeleter {
private:
  /**
  * Tracks the reference count for a single index file:
  */
  class RefCount {
  public:
	  int count;
	  int IncRef() {
		  return ++count;
	  }
	  int DecRef() {
		  return --count;
	  }
  };
	
  /**
   * Holds details for each commit point.  This class is
   * also passed to the deletion policy.  Note: this class
   * has a natural ordering that is inconsistent with
   * equals.
   */
  class CommitPoint: public IndexCommitPoint, public CL_NS(util)::Comparable {
    int64_t gen;
    std::string segmentsFileName;
    IndexFileDeleter* _this;
  public:
		std::vector<std::string> files;
    bool deleted;

    CommitPoint(IndexFileDeleter* _this, SegmentInfos* segmentInfos);
    virtual ~CommitPoint();

    /**
     * Get the segments_N file for this commit point.
     */
    std::string getSegmentsFileName();

    const std::vector<std::string>& getFileNames();

    /**
     * Called only be the deletion policy, to remove this
     * commit point from the index.
     */
    void deleteCommitPoint();

    int32_t compareTo(NamedObject* obj);

		static const char* getClassName();
		const char* getObjectName() const;
    static bool sort(IndexCommitPoint* elem1, IndexCommitPoint* elem2);
  };

private:
  /* Files that we tried to delete but failed (likely
   * because they are open and we are running on Windows),
   * so we will retry them again later: */
  std::vector<std::string> deletable;

  typedef CL_NS(util)::CLHashMap<char*, RefCount*,
	  CL_NS(util)::Compare::Char,
	  CL_NS(util)::Equals::Char,
	  CL_NS(util)::Deletor::acArray,
	  CL_NS(util)::Deletor::Object<RefCount> > RefCountsType;
  /* Reference count for all files in the index.  
   * Counts how many existing commits reference a file.
   * Maps String to RefCount (class below) instances: */
  RefCountsType refCounts;

  typedef CL_NS(util)::CLVector<IndexCommitPoint*, CL_NS(util)::Deletor::Object<IndexCommitPoint> > CommitsType;
  /* Holds all commits (segments_N) currently in the index.
   * This will have just 1 commit if you are using the
   * default delete policy (KeepOnlyLastCommitDeletionPolicy).
   * Other policies may leave commit points live for longer
   * in which case this list would be longer than 1: */
   CommitsType commits;

  /* Holds files we had incref'd from the previous
   * non-commit checkpoint: */
  std::vector<std::string> lastFiles;

  /* Commits that the IndexDeletionPolicy have decided to delete: */ 
  CL_NS(util)::CLArrayList<CommitPoint*> commitsToDelete;

  std::ostream* infoStream;
  CL_NS(store)::Directory* directory;
  IndexDeletionPolicy* policy;
  DocumentsWriter* docWriter;


public:
  void deletePendingFiles();

  void setInfoStream(std::ostream* infoStream);
  void message(std::string message);
  void decRef(const std::string& fileName);
  RefCount* getRefCount(const char* fileName);

  /**
   * Remove the CommitPoints in the commitsToDelete List by
   * DecRef'ing all files from each SegmentInfos.
   */
  void deleteCommits();

  /** Change to true to see details of reference counts when
   *  infoStream != null */
  static bool VERBOSE_REF_COUNTS;

  /**
   * Initialize the deleter: find all previous commits in
   * the Directory, incref the files they reference, call
   * the policy to let it delete commits.  The incoming
   * segmentInfos must have been loaded from a commit point
   * and not yet modified.  This will remove any files not
   * referenced by any of the commits.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  IndexFileDeleter(CL_NS(store)::Directory* directory, IndexDeletionPolicy* policy, SegmentInfos* segmentInfos, std::ostream* infoStream, DocumentsWriter* docWriter);
  ~IndexFileDeleter();

  /**
   * Writer calls this when it has hit an error and had to
   * roll back, to tell us that there may now be
   * unreferenced files in the filesystem.  So we re-list
   * the filesystem and delete such files.  If segmentName
   * is non-null, we will only delete files corresponding to
   * that segment.
   */
  void refresh(const char* segmentName);
  void refresh();
  void close();

  /**
   * For definition of "check point" see IndexWriter comments:
   * "Clarification: Check Points (and commits)".
   * 
   * Writer calls this when it has made a "consistent
   * change" to the index, meaning new files are written to
   * the index and the in-memory SegmentInfos have been
   * modified to point to those files.
   *
   * This may or may not be a commit (segments_N may or may
   * not have been written).
   *
   * We simply incref the files referenced by the new
   * SegmentInfos and decref the files we had previously
   * seen (if any).
   *
   * If this is a commit, we also call the policy to give it
   * a chance to remove other commits.  If any commits are
   * removed, we decref their files as well.
   */
  void checkpoint(SegmentInfos* segmentInfos, bool isCommit);


  void CLUCENE_LOCAL_DECL incRef(SegmentInfos* segmentInfos, bool isCommit);
  void CLUCENE_LOCAL_DECL incRef(const std::vector<std::string>& files);
  void CLUCENE_LOCAL_DECL decRef(const std::vector<std::string>& files) ;
  void CLUCENE_LOCAL_DECL decRef(SegmentInfos* segmentInfos);
  void CLUCENE_LOCAL_DECL deleteFiles(std::vector<std::string>& files);

  /** Delets the specified files, but only if they are new
   *  (have not yet been incref'd). */
  void CLUCENE_LOCAL_DECL deleteNewFiles(const std::vector<std::string>& files);
  void CLUCENE_LOCAL_DECL deleteFile(const char* fileName);
};

CL_NS_END
#endif
