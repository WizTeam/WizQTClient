/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_MergePolicy_
#define _lucene_index_MergePolicy_

#include "CLucene/util/VoidList.h"
CL_CLASS_DEF(store,Directory)
CL_NS_DEF(index)

class SegmentInfo;
class SegmentInfos;
class IndexWriter;

/**
 * <p>Expert: a MergePolicy determines the sequence of
 * primitive merge operations to be used for overall merge
 * and optimize operations.</p>
 *
 * <p>Whenever the segments in an index have been altered by
 * {@link IndexWriter}, either the addition of a newly
 * flushed segment, addition of many segments from
 * addIndexes* calls, or a previous merge that may now need
 * to cascade, {@link IndexWriter} invokes {@link
 * #findMerges} to give the MergePolicy a chance to pick
 * merges that are now required.  This method returns a
 * {@link MergeSpecification} instance describing the set of
 * merges that should be done, or null if no merges are
 * necessary.  When IndexWriter.optimize is called, it calls
 * {@link #findMergesForOptimize} and the MergePolicy should
 * then return the necessary merges.</p>
 *
 * <p>Note that the policy can return more than one merge at
 * a time.  In this case, if the writer is using {@link
 * SerialMergeScheduler}, the merges will be run
 * sequentially but if it is using {@link
 * ConcurrentMergeScheduler} they will be run concurrently.</p>
 *
 * <p>The default MergePolicy is {@link
 * LogByteSizeMergePolicy}.</p>
 * <p><b>NOTE:</b> This API is new and still experimental
 * (subject to change suddenly in the next release)</p>
 */
class CLUCENE_EXPORT MergePolicy: public CL_NS(util)::NamedObject {
public:
  /** OneMerge provides the information necessary to perform
   *  an individual primitive merge operation, resulting in
   *  a single new segment.  The merge spec includes the
   *  subset of segments to be merged as well as whether the
   *  new segment should use the compound file format. */
  class CLUCENE_EXPORT OneMerge: public CL_NS(util)::NamedObject {
  public:
    DEFINE_MUTEX(THIS_LOCK)
    SegmentInfo* info;               // used by IndexWriter
    bool mergeDocStores;         // used by IndexWriter
    bool optimize;               // used by IndexWriter
    SegmentInfos* segmentsClone;     // used by IndexWriter
    bool increfDone;             // used by IndexWriter
    bool registerDone;           // used by IndexWriter
    int64_t mergeGen;                  // used by IndexWriter
    bool isExternal;             // used by IndexWriter
    int32_t maxNumSegmentsOptimize;     // used by IndexWriter

    SegmentInfos* segments;
    const bool useCompoundFile;
    bool aborted;
    CLuceneError error;

    /**
    * Constructor
    * @memory, segments object is consumed. The SegmentInfo objects within are referenced
    */
    OneMerge(SegmentInfos* segments, bool _useCompoundFile);
    ~OneMerge();

    /** Record that an exception occurred while executing
     *  this merge */
    void setException(CLuceneError& error);

    /** Retrieve previous exception set by {@link
     *  #setException}. */
    const CLuceneError& getException();

    /** Mark this merge as aborted.  If this is called
     *  before the merge is committed then the merge will
     *  not be committed. */
    void abort();

    /** Returns true if this merge was aborted. */
    bool isAborted();

    void checkAborted(CL_NS(store)::Directory* dir);

    std::string segString(CL_NS(store)::Directory* dir) const;

		static const char* getClassName();
		virtual const char* getObjectName() const;
  };

  /**
   * A MergeSpecification instance provides the information
   * necessary to perform multiple merges.  It simply
   * contains a list of {@link OneMerge} instances.
   */

  class CLUCENE_EXPORT MergeSpecification {
  public:
    MergeSpecification();
    ~MergeSpecification();

    /**
     * The subset of segments to be included in the primitive merge.
     */
    CL_NS(util)::CLArrayList<OneMerge*>* merges;

    void add(OneMerge* merge);

    std::string segString(CL_NS(store)::Directory* dir);
  };


  /**
   * Determine what set of merge operations are now
   * necessary on the index.  The IndexWriter calls this
   * whenever there is a change to the segments.  This call
   * is always synchronized on the IndexWriter instance so
   * only one thread at a time will call this method.
   *
   * @param segmentInfos the total set of segments in the index
   * @param writer IndexWriter instance
   */
  virtual MergeSpecification* findMerges(SegmentInfos* segmentInfos,
                                         IndexWriter* writer) = 0;

  /**
   * Determine what set of merge operations are necessary in
   * order to optimize the index.  The IndexWriter calls
   * this when its optimize() method is called.  This call
   * is always synchronized on the IndexWriter instance so
   * only one thread at a time will call this method.
   *
   * @param segmentInfos the total set of segments in the index
   * @param writer IndexWriter instance
   * @param maxSegmentCount requested maximum number of
   *   segments in the index (currently this is always 1)
   * @param segmentsToOptimize contains the specific
   *   SegmentInfo instances that must be merged away.  This
   *   may be a subset of all SegmentInfos.
   */
  virtual MergeSpecification* findMergesForOptimize(SegmentInfos* segmentInfos,
                                                    IndexWriter* writer,
                                                    int32_t maxSegmentCount,
                                                    std::vector<SegmentInfo*>& segmentsToOptimize) = 0;

  /**
   * Release all resources for the policy.
   */
  virtual void close() = 0;

  /**
   * Returns true if a newly flushed (not from merge)
   * segment should use the compound file format.
   */
  virtual bool useCompoundFile(SegmentInfos* segments, SegmentInfo* newSegment) = 0;

  /**
   * Returns true if the doc store files should use the
   * compound file format.
   */
  virtual bool useCompoundDocStore(SegmentInfos* segments) = 0;
};













/** <p>This class implements a {@link MergePolicy} that tries
 *  to merge segments into levels of exponentially
 *  increasing size, where each level has < mergeFactor
 *  segments in it.  Whenever a given levle has mergeFactor
 *  segments or more in it, they will be merged.</p>
 *
 * <p>This class is abstract and requires a subclass to
 * define the {@link #size} method which specifies how a
 * segment's size is determined.  {@link LogDocMergePolicy}
 * is one subclass that measures size by document count in
 * the segment.  {@link LogByteSizeMergePolicy} is another
 * subclass that measures size as the total byte size of the
 * file(s) for the segment.</p>
 */
class CLUCENE_EXPORT LogMergePolicy: public MergePolicy {

  int32_t mergeFactor;

  int32_t maxMergeDocs;

  bool _useCompoundFile;
  bool _useCompoundDocStore;
  IndexWriter* writer;

  void message(const std::string& message);

  bool isOptimized(SegmentInfos* infos, IndexWriter* writer, int32_t maxNumSegments, std::vector<SegmentInfo*>& segmentsToOptimize);

  /** Returns true if this single nfo is optimized (has no
   *  pending norms or deletes, is in the same dir as the
   *  writer, and matches the current compound file setting */
  bool isOptimized(IndexWriter* writer, SegmentInfo* info);


protected:
  virtual int64_t size(SegmentInfo* info) = 0;
  int64_t minMergeSize;
  uint64_t maxMergeSize;

public:
  LogMergePolicy();

  /** Defines the allowed range of log(size) for each
   *  level.  A level is computed by taking the max segment
   *  log size, minuse LEVEL_LOG_SPAN, and finding all
   *  segments falling within that range. */
  static const float_t LEVEL_LOG_SPAN;

  /** Default merge factor, which is how many segments are
   *  merged at a time */
  LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_MERGE_FACTOR = 10);

  /** Default maximum segment size.  A segment of this size
   *  or larger will never be merged.  @see setMaxMergeDocs */
  static const int32_t DEFAULT_MAX_MERGE_DOCS;

  /** <p>Returns the number of segments that are merged at
   * once and also controls the total number of segments
   * allowed to accumulate in the index.</p> */
  int32_t getMergeFactor();

  /** Determines how often segment indices are merged by
   * addDocument().  With smaller values, less RAM is used
   * while indexing, and searches on unoptimized indices are
   * faster, but indexing speed is slower.  With larger
   * values, more RAM is used during indexing, and while
   * searches on unoptimized indices are slower, indexing is
   * faster.  Thus larger values (> 10) are best for batch
   * index creation, and smaller values (< 10) for indices
   * that are interactively maintained. */
  void setMergeFactor(int32_t mergeFactor);

  // Javadoc inherited
  bool useCompoundFile(SegmentInfos* infos, SegmentInfo* info);

  /** Sets whether compound file format should be used for
   *  newly flushed and newly merged segments. */
  void setUseCompoundFile(bool useCompoundFile);

  /** Returns true if newly flushed and newly merge segments
   *  are written in compound file format. @see
   *  #setUseCompoundFile */
  bool getUseCompoundFile();

  // Javadoc inherited
  bool useCompoundDocStore(SegmentInfos* infos);

  /** Sets whether compound file format should be used for
   *  newly flushed and newly merged doc store
   *  segment files (term vectors and stored fields). */
  void setUseCompoundDocStore(bool useCompoundDocStore);

  /** Returns true if newly flushed and newly merge doc
   *  store segment files (term vectors and stored fields)
   *  are written in compound file format. @see
   *  #setUseCompoundDocStore */
  bool getUseCompoundDocStore();

  void close();


  /** Returns the merges necessary to optimize the index.
   *  This merge policy defines "optimized" to mean only one
   *  segment in the index, where that segment has no
   *  deletions pending nor separate norms, and it is in
   *  compound file format if the current useCompoundFile
   *  setting is true.  This method returns multiple merges
   *  (mergeFactor at a time) so the {@link MergeScheduler}
   *  in use may make use of concurrency. */
  MergeSpecification* findMergesForOptimize(SegmentInfos* segmentInfos,
                                                    IndexWriter* writer,
                                                    int32_t maxSegmentCount,
                                                    std::vector<SegmentInfo*>& segmentsToOptimize);

  /** Checks if any merges are now necessary and returns a
   *  {@link MergePolicy.MergeSpecification} if so.  A merge
   *  is necessary when there are more than {@link
   *  #setMergeFactor} segments at a given level.  When
   *  multiple levels have too many segments, this method
   *  will return multiple merges, allowing the {@link
   *  MergeScheduler} to use concurrency. */
  MergeSpecification* findMerges(SegmentInfos* infos, IndexWriter* writer);

  /** <p>Determines the largest segment (measured by
   * document count) that may be merged with other segments.
   * Small values (e.g., less than 10,000) are best for
   * interactive indexing, as this limits the length of
   * pauses while indexing to a few seconds.  Larger values
   * are best for batched indexing and speedier
   * searches.</p>
   *
   * <p>The default value is {@link Integer#MAX_VALUE}.</p>
   *
   * <p>The default merge policy ({@link
   * LogByteSizeMergePolicy}) also allows you to set this
   * limit by net size (in MB) of the segment, using {@link
   * LogByteSizeMergePolicy#setMaxMergeMB}.</p>
   */
  void setMaxMergeDocs(int32_t maxMergeDocs);

  /** Returns the largest segment (measured by document
   *  count) that may be merged with other segments.
   *  @see #setMaxMergeDocs */
  int32_t getMaxMergeDocs();


  virtual bool instanceOf(const char* otherobject) const;
  static const char* getClassName();
  virtual const char* getObjectName() const;
};






/** This is a {@link LogMergePolicy} that measures size of a
 *  segment as the number of documents (not taking deletions
 *  into account). */
class CLUCENE_EXPORT LogDocMergePolicy: public LogMergePolicy {
public:

  /** Default minimum segment size.  @see setMinMergeDocs */
  LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_MIN_MERGE_DOCS = 1000);

  LogDocMergePolicy();

  /** Sets the minimum size for the lowest level segments.
   * Any segments below this size are considered to be on
   * the same level (even if they vary drastically in size)
   * and will be merged whenever there are mergeFactor of
   * them.  This effectively truncates the "int64_t tail" of
   * small segments that would otherwise be created into a
   * single level.  If you set this too large, it could
   * greatly increase the merging cost during indexing (if
   * you flush many small segments). */
  void setMinMergeDocs(int32_t minMergeDocs);

  /** Get the minimum size for a segment to remain
   *  un-merged.
   *  @see #setMinMergeDocs **/
  int32_t getMinMergeDocs();


  static const char* getClassName();
  virtual const char* getObjectName() const;
protected:
  int64_t size(SegmentInfo* info);
};


/** This is a {@link LogMergePolicy} that measures size of a
 *  segment as the total byte size of the segment's files. */
class CLUCENE_EXPORT LogByteSizeMergePolicy: public LogMergePolicy {
protected:
  int64_t size(SegmentInfo* info);

public:
  /** Default minimum segment size.  @see setMinMergeMB */
  static const float_t DEFAULT_MIN_MERGE_MB;

  /** Default maximum segment size.  A segment of this size
   *  or larger will never be merged.  @see setMaxMergeMB */
  static const float_t DEFAULT_MAX_MERGE_MB;

  LogByteSizeMergePolicy();

  /** <p>Determines the largest segment (measured by total
   *  byte size of the segment's files, in MB) that may be
   *  merged with other segments.  Small values (e.g., less
   *  than 50 MB) are best for interactive indexing, as this
   *  limits the length of pauses while indexing to a few
   *  seconds.  Larger values are best for batched indexing
   *  and speedier searches.</p>
   *
   *  <p>Note that {@link #setMaxMergeDocs} is also
   *  used to check whether a segment is too large for
   *  merging (it's either or).</p>*/
  void setMaxMergeMB(float_t mb);

  /** Returns the largest segment (meaured by total byte
   *  size of the segment's files, in MB) that may be merged
   *  with other segments.
   *  @see #setMaxMergeMB */
  float_t getMaxMergeMB();

  /** Sets the minimum size for the lowest level segments.
   * Any segments below this size are considered to be on
   * the same level (even if they vary drastically in size)
   * and will be merged whenever there are mergeFactor of
   * them.  This effectively truncates the "long tail" of
   * small segments that would otherwise be created into a
   * single level.  If you set this too large, it could
   * greatly increase the merging cost during indexing (if
   * you flush many small segments). */
  void setMinMergeMB(float_t mb);

  /** Get the minimum size for a segment to remain
   *  un-merged.
   *  @see #setMinMergeMB **/
  float_t getMinMergeMB();

  static const char* getClassName();
  virtual const char* getObjectName() const;
};



CL_NS_END
#endif

