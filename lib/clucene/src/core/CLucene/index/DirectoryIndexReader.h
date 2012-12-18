/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_DirectoryIndexReader_
#define _lucene_index_DirectoryIndexReader_

#include "IndexReader.h"

CL_CLASS_DEF(store,LuceneLock)

CL_NS_DEF(index)
class IndexDeletionPolicy;

/**
 * IndexReader implementation that has access to a Directory.
 * Instances that have a SegmentInfos object (i. e. segmentInfos != null)
 * "own" the directory, which means that they try to acquire a write lock
 * whenever index modifications are performed.
 */
class CLUCENE_EXPORT DirectoryIndexReader: public IndexReader {
private:
  IndexDeletionPolicy* deletionPolicy;

  SegmentInfos* segmentInfos;
  CL_NS(store)::LuceneLock* writeLock;
  bool stale;

  /** Used by commit() to record pre-commit state in case
   * rollback is necessary */
  bool rollbackHasChanges;
  SegmentInfos* rollbackSegmentInfos;

  class FindSegmentsFile_Open;
  class FindSegmentsFile_Reopen;
  friend class FindSegmentsFile_Open;
  friend class FindSegmentsFile_Reopen;

protected:
  CL_NS(store)::Directory* _directory;
  bool closeDirectory;
  DirectoryIndexReader();

  /**
   * Re-opens the index using the passed-in SegmentInfos
   */
  virtual DirectoryIndexReader* doReopen(SegmentInfos* infos) = 0;


  void doClose();

  /**
   * Commit changes resulting from delete, undeleteAll, or
   * setNorm operations
   *
   * If an exception is hit, then either no changes or all
   * changes will have been committed to the index
   * (transactional semantics).
   * @throws IOException if there is a low-level IO error
   */
  void doCommit();

  virtual void commitChanges() = 0;

  /**
   * Tries to acquire the WriteLock on this directory->
   * this method is only valid if this IndexReader is directory owner.
   *
   * @throws StaleReaderException if the index has changed
   * since this reader was opened
   * @throws CorruptIndexException if the index is corrupt
   * @throws LockObtainFailedException if another writer
   *  has this index open (<code>write.lock</code> could not
   *  be obtained)
   * @throws IOException if there is a low-level IO error
   */
  void acquireWriteLock();

public:
  virtual ~DirectoryIndexReader();
  void init(CL_NS(store)::Directory* directory, SegmentInfos* segmentInfos, bool closeDirectory);

  CLUCENE_LOCAL_DECL DirectoryIndexReader(CL_NS(store)::Directory* directory, SegmentInfos* segmentInfos, bool closeDirectory);
  CLUCENE_LOCAL_DECL static DirectoryIndexReader* open(CL_NS(store)::Directory* directory, bool closeDirectory, IndexDeletionPolicy* deletionPolicy);

  IndexReader* reopen();

  void setDeletionPolicy(IndexDeletionPolicy* deletionPolicy);

  /** Returns the directory this index resides in.
   */
  CL_NS(store)::Directory* directory();

  /**
   * Version number when this IndexReader was opened.
   */
  int64_t getVersion();

  /**
   * Check whether this IndexReader is still using the
   * current (i.e., most recently committed) version of the
   * index.  If a writer has committed any changes to the
   * index since this reader was opened, this will return
   * <code>false</code>, in which case you must open a _CLNEW
   * IndexReader in order to see the changes.  See the
   * description of the <a href="IndexWriter.html#autoCommit"><code>autoCommit</code></a>
   * flag which controls when the {@link IndexWriter}
   * actually commits changes to the index.
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  bool isCurrent();

  /**
   * Checks is the index is optimized (if it has a single segment and no deletions)
   * @return <code>true</code> if the index is optimized; <code>false</code> otherwise
   */
  bool isOptimized();

  /**
   * Should internally checkpoint state that will change
   * during commit so that we can rollback if necessary.
   */
  CLUCENE_LOCAL_DECL void startCommit();

  /**
   * Rolls back state to just before the commit (this is
   * called by commit() if there is some exception while
   * committing).
   */
  CLUCENE_LOCAL_DECL void rollbackCommit();

};

CL_NS_END
#endif
