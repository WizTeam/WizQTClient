/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_IndexWriter_
#define _lucene_index_IndexWriter_

#include "CLucene/util/VoidList.h"
#include "CLucene/util/Array.h"
CL_CLASS_DEF(search,Similarity)
CL_CLASS_DEF(store,Lock)
CL_CLASS_DEF(analysis,Analyzer)
CL_CLASS_DEF(store,Directory)
CL_CLASS_DEF(store,LuceneLock)
CL_CLASS_DEF(document,Document)

#include "MergePolicy.h"
#include "CLucene/LuceneThreads.h"

CL_NS_DEF(index)
class SegmentInfo;
class SegmentInfos;
class MergePolicy;
class IndexReader;
class SegmentReader;
class MergeScheduler;
class DocumentsWriter;
class IndexFileDeleter;
class LogMergePolicy;
class IndexDeletionPolicy;
class Term;

/**
  An <code>IndexWriter</code> creates and maintains an index.

  <p>The <code>create</code> argument to the
  <a href="#IndexWriter(org.apache.lucene.store.Directory, org.apache.lucene.analysis.Analyzer, boolean)"><b>constructor</b></a>
  determines whether a new index is created, or whether an existing index is
  opened.  Note that you
  can open an index with <code>create=true</code> even while readers are
  using the index.  The old readers will continue to search
  the "point in time" snapshot they had opened, and won't
  see the newly created index until they re-open.  There are
  also <a href="#IndexWriter(org.apache.lucene.store.Directory, org.apache.lucene.analysis.Analyzer)"><b>constructors</b></a>
  with no <code>create</code> argument which
  will create a new index if there is not already an index at the
  provided path and otherwise open the existing index.</p>

  <p>In either case, documents are added with <a
  href="#addDocument(org.apache.lucene.document.Document)"><b>addDocument</b></a>
  and removed with <a
  href="#deleteDocuments(org.apache.lucene.index.Term)"><b>deleteDocuments</b></a>.
  A document can be updated with <a href="#updateDocument(org.apache.lucene.index.Term, org.apache.lucene.document.Document)"><b>updateDocument</b></a>
  (which just deletes and then adds the entire document).
  When finished adding, deleting and updating documents, <a href="#close()"><b>close</b></a> should be called.</p>

  <p>These changes are buffered in memory and periodically
  flushed to the {@link Directory} (during the above method
  calls).  A flush is triggered when there are enough
  buffered deletes (see {@link #setMaxBufferedDeleteTerms})
  or enough added documents since the last flush, whichever
  is sooner.  For the added documents, flushing is triggered
  either by RAM usage of the documents (see {@link
  #setRAMBufferSizeMB}) or the number of added documents.
  The default is to flush when RAM usage hits 16 MB.  For
  best indexing speed you should flush by RAM usage with a
  large RAM buffer.  You can also force a flush by calling
  {@link #flush}.  When a flush occurs, both pending deletes
  and added documents are flushed to the index.  A flush may
  also trigger one or more segment merges which by default
  run with a background thread so as not to block the
  addDocument calls (see <a href="#mergePolicy">below</a>
  for changing the {@link MergeScheduler}).</p>

  <a name="autoCommit"></a>
  <p>The optional <code>autoCommit</code> argument to the
  <a href="#IndexWriter(org.apache.lucene.store.Directory, boolean, org.apache.lucene.analysis.Analyzer)"><b>constructors</b></a>
  controls visibility of the changes to {@link IndexReader} instances reading the same index.
  When this is <code>false</code>, changes are not
  visible until {@link #close()} is called.
  Note that changes will still be flushed to the
  {@link org.apache.lucene.store.Directory} as new files,
  but are not committed (no new <code>segments_N</code> file
  is written referencing the new files) until {@link #close} is
  called.  If something goes terribly wrong (for example the
  JVM crashes) before {@link #close()}, then
  the index will reflect none of the changes made (it will
  remain in its starting state).
  You can also call {@link #abort()}, which closes the writer without committing any
  changes, and removes any index
  files that had been flushed but are now unreferenced.
  This mode is useful for preventing readers from refreshing
  at a bad time (for example after you've done all your
  deletes but before you've done your adds).
  It can also be used to implement simple single-writer
  transactional semantics ("all or none").</p>

  <p>When <code>autoCommit</code> is <code>true</code> then
  every flush is also a commit ({@link IndexReader}
  instances will see each flush as changes to the index).
  This is the default, to match the behavior before 2.2.
  When running in this mode, be careful not to refresh your
  readers while optimize or segment merges are taking place
  as this can tie up substantial disk space.</p>

  <p>Regardless of <code>autoCommit</code>, an {@link
  IndexReader} or {@link org.apache.lucene.search.IndexSearcher} will only see the
  index as of the "point in time" that it was opened.  Any
  changes committed to the index after the reader was opened
  are not visible until the reader is re-opened.</p>

  <p>If an index will not have more documents added for a while and optimal search
  performance is desired, then the <a href="#optimize()"><b>optimize</b></a>
  method should be called before the index is closed.</p>

  <p>Opening an <code>IndexWriter</code> creates a lock file for the directory in use. Trying to open
  another <code>IndexWriter</code> on the same directory will lead to a
  {@link LockObtainFailedException}. The {@link LockObtainFailedException}
  is also thrown if an IndexReader on the same directory is used to delete documents
  from the index.</p>

  <a name="deletionPolicy"></a>
  <p>Expert: <code>IndexWriter</code> allows an optional
  {@link IndexDeletionPolicy} implementation to be
  specified.  You can use this to control when prior commits
  are deleted from the index.  The default policy is {@link
  KeepOnlyLastCommitDeletionPolicy} which removes all prior
  commits as soon as a new commit is done (this matches
  behavior before 2.2).  Creating your own policy can allow
  you to explicitly keep previous "point in time" commits
  alive in the index for some time, to allow readers to
  refresh to the new commit without having the old commit
  deleted out from under them.  This is necessary on
  filesystems like NFS that do not support "delete on last
  close" semantics, which Lucene's "point in time" search
  normally relies on. </p>

  <a name="mergePolicy"></a> <p>Expert:
  <code>IndexWriter</code> allows you to separately change
  the {@link MergePolicy} and the {@link MergeScheduler}.
  The {@link MergePolicy} is invoked whenever there are
  changes to the segments in the index.  Its role is to
  select which merges to do, if any, and return a {@link
  MergePolicy.MergeSpecification} describing the merges.  It
  also selects merges to do for optimize().  (The default is
  {@link LogByteSizeMergePolicy}.  Then, the {@link
  MergeScheduler} is invoked with the requested merges and
  it decides when and how to run the merges.  The default is
  {@link ConcurrentMergeScheduler}. </p>
 */
/*
 * Clarification: Check Points (and commits)
 * Being able to set autoCommit=false allows IndexWriter to flush and
 * write new index files to the directory without writing a new segments_N
 * file which references these new files. It also means that the state of
 * the in memory SegmentInfos object is different than the most recent
 * segments_N file written to the directory.
 *
 * Each time the SegmentInfos is changed, and matches the (possibly
 * modified) directory files, we have a new "check point".
 * If the modified/new SegmentInfos is written to disk - as a new
 * (generation of) segments_N file - this check point is also an
 * IndexCommitPoint.
 *
 * With autoCommit=true, every checkPoint is also a CommitPoint.
 * With autoCommit=false, some checkPoints may not be commits.
 *
 * A new checkpoint always replaces the previous checkpoint and
 * becomes the new "front" of the index. This allows the IndexFileDeleter
 * to delete files that are referenced only by stale checkpoints.
 * (files that were created since the last commit, but are no longer
 * referenced by the "front" of the index). For this, IndexFileDeleter
 * keeps track of the last non commit checkpoint.
 */
class CLUCENE_EXPORT IndexWriter:LUCENE_BASE {
  bool isOpen; //indicates if the writers is open - this way close can be called multiple times

  // how to analyze text
  CL_NS(analysis)::Analyzer* analyzer;

  CL_NS(search)::Similarity* similarity; // how to normalize

  bool closeDir;
  bool closed;
  bool closing;

  // Holds all SegmentInfo instances currently involved in
  // merges
  typedef CL_NS(util)::CLHashSet<SegmentInfo*, CL_NS(util)::Compare::Void<SegmentInfo> > MergingSegmentsType;
  MergingSegmentsType* mergingSegments;
  MergePolicy* mergePolicy;
  MergeScheduler* mergeScheduler;

  typedef  CL_NS(util)::CLLinkedList<MergePolicy::OneMerge*,
  CL_NS(util)::Deletor::Object<MergePolicy::OneMerge> > PendingMergesType;
  PendingMergesType* pendingMerges;

  typedef CL_NS(util)::CLHashSet<MergePolicy::OneMerge*,
  CL_NS(util)::Compare::Void<MergePolicy::OneMerge>,
  CL_NS(util)::Deletor::Object<MergePolicy::OneMerge> > RunningMergesType;
  RunningMergesType* runningMerges;

  typedef CL_NS(util)::CLArrayList<MergePolicy::OneMerge*> MergeExceptionsType;
  MergeExceptionsType* mergeExceptions;
  int64_t mergeGen;
  bool stopMerges;


  /** If non-null, information about merges will be printed to this.
   */
  std::ostream* infoStream;
  static std::ostream* defaultInfoStream;



  bool commitPending; // true if segmentInfos has changes not yet committed
  SegmentInfos* rollbackSegmentInfos;      // segmentInfos we will fallback to if the commit fails

  SegmentInfos* localRollbackSegmentInfos;      // segmentInfos we will fallback to if the commit fails
  bool localAutoCommit;                // saved autoCommit during local transaction
  bool autoCommit;              // false if we should commit only on close

  DocumentsWriter* docWriter;
  IndexFileDeleter* deleter;

  typedef std::vector<SegmentInfo*> SegmentsToOptimizeType;
  SegmentsToOptimizeType* segmentsToOptimize;           // used by optimize to note those needing optimization


  CL_NS(store)::LuceneLock* writeLock;

  void init(CL_NS(store)::Directory* d, CL_NS(analysis)::Analyzer* a, bool closeDir, IndexDeletionPolicy* deletionPolicy, bool autoCommit);
  void init(CL_NS(store)::Directory* d, CL_NS(analysis)::Analyzer* a, bool create, bool closeDir, IndexDeletionPolicy* deletionPolicy, bool autoCommit);
  void deinit(bool releaseWriteLock = true) throw();

	// where this index resides
	CL_NS(store)::Directory* directory;
  bool bOwnsDirectory;


  int32_t getSegmentsCounter();
  int32_t maxFieldLength;
  int32_t mergeFactor;
  int32_t minMergeDocs;
  int32_t maxMergeDocs;
  int32_t termIndexInterval;

  int64_t writeLockTimeout;
  int64_t commitLockTimeout;

  // The normal read buffer size defaults to 1024, but
  // increasing this during merging seems to yield
  // performance gains.  However we don't want to increase
  // it too much because there are quite a few
  // BufferedIndexInputs created during merging.  See
  // LUCENE-888 for details.
  static const int32_t MERGE_READ_BUFFER_SIZE;

  // Used for printing messages
  STATIC_DEFINE_MUTEX(MESSAGE_ID_LOCK)
  static int32_t MESSAGE_ID;
  int32_t messageID;
  mutable bool hitOOM;

public:
  DEFINE_MUTEX(THIS_LOCK)
  DEFINE_CONDITION(THIS_WAIT_CONDITION)

	// Release the write lock, if needed.
	SegmentInfos* segmentInfos;

	// Release the write lock, if needed.
	virtual ~IndexWriter();

	/**
	*  The Java implementation of Lucene silently truncates any tokenized
	*  field if the number of tokens exceeds a certain threshold.  Although
	*  that threshold is adjustable, it is easy for the client programmer
	*  to be unaware that such a threshold exists, and to become its
	*  unwitting victim.
	*  CLucene implements a less insidious truncation policy.  Up to
	*  DEFAULT_MAX_FIELD_LENGTH tokens, CLucene behaves just as JLucene
	*  does.  If the number of tokens exceeds that threshold without any
	*  indication of a truncation preference by the client programmer,
	*  CLucene raises an exception, prompting the client programmer to
	*  explicitly set a truncation policy by adjusting maxFieldLength.
	*/
	LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_MAX_FIELD_LENGTH = 10000);
	LUCENE_STATIC_CONSTANT(int32_t, FIELD_TRUNC_POLICY__WARN = -1);

  /**
   * Returns the maximum number of terms that will be
   * indexed for a single field in a document.
   * @see #setMaxFieldLength
   */
  int32_t getMaxFieldLength();
  /**
   * The maximum number of terms that will be indexed for a single field in a
   * document.  This limits the amount of memory required for indexing, so that
   * collections with very large files will not crash the indexing process by
   * running out of memory.  This setting refers to the number of running terms,
   * not to the number of different terms.<p/>
   * <strong>Note:</strong> see {@link DEFAULT_MAX_FIELD_LENGTH} for an important
   * note regarding field lengths.
   * @see #DEFAULT_MAX_FIELD_LENGTH
   */
  void setMaxFieldLength(int32_t val);

  /** Determines the minimal number of documents required before the buffered
   * in-memory documents are merging and a new Segment is created.
   * Since Documents are merged in a {@link RAMDirectory},
   * large value gives faster indexing.  At the same time, mergeFactor limits
   * the number of files open in a FSDirectory.
   *
   * <p> The default value is DEFAULT_MAX_BUFFERED_DOCS.*/
  void setMaxBufferedDocs(int32_t val);
  /**
   * @see #setMaxBufferedDocs
   */
  int32_t getMaxBufferedDocs();

  /**
   * Default value for the write lock timeout (1,000).
   * @see #setDefaultWriteLockTimeout
   */
  static int64_t WRITE_LOCK_TIMEOUT;
  /**
   * Sets the maximum time to wait for a write lock (in milliseconds).
   */
  void setWriteLockTimeout(int64_t writeLockTimeout);
  /**
   * @see #setWriteLockTimeout
   */
  int64_t getWriteLockTimeout();

  /**
   * Sets the maximum time to wait for a commit lock (in milliseconds).
   */
  void setCommitLockTimeout(int64_t commitLockTimeout);
  /**
   * @see #setCommitLockTimeout
   */
  int64_t getCommitLockTimeout();

  /**
   * Name of the write lock in the index.
   */
  static const char* WRITE_LOCK_NAME; //"write.lock";

  /**
   * @deprecated
   * @see LogMergePolicy#DEFAULT_MERGE_FACTOR
   */
  static const int32_t DEFAULT_MERGE_FACTOR ;

  /**
   * Value to denote a flush trigger is disabled
   */
  static const int32_t DISABLE_AUTO_FLUSH;

  /**
   * Disabled by default (because IndexWriter flushes by RAM usage
   * by default). Change using {@link #setMaxBufferedDocs(int)}.
   */
  static const int32_t DEFAULT_MAX_BUFFERED_DOCS;

  /**
   * Default value is 16 MB (which means flush when buffered
   * docs consume 16 MB RAM).  Change using {@link #setRAMBufferSizeMB}.
   */
  static const float_t DEFAULT_RAM_BUFFER_SIZE_MB;

  /**
   * Disabled by default (because IndexWriter flushes by RAM usage
   * by default). Change using {@link #setMaxBufferedDeleteTerms(int)}.
   */
  static const int32_t DEFAULT_MAX_BUFFERED_DELETE_TERMS;

  /**
   * @deprecated
   * @see LogDocMergePolicy#DEFAULT_MAX_MERGE_DOCS
   */
  static const int32_t DEFAULT_MAX_MERGE_DOCS;

  /**
   * Absolute hard maximum length for a term.  If a term
   * arrives from the analyzer longer than this length, it
   * is skipped and a message is printed to infoStream, if
   * set (see {@link #setInfoStream}).
   */
  static const int32_t MAX_TERM_LENGTH;


  /* Determines how often segment indices are merged by addDocument().  With
   *  smaller values, less RAM is used while indexing, and searches on
   *  unoptimized indices are faster, but indexing speed is slower.  With larger
   *  values more RAM is used while indexing and searches on unoptimized indices
   *  are slower, but indexing is faster.  Thus larger values (> 10) are best
   *  for batched index creation, and smaller values (< 10) for indices that are
   *  interactively maintained.
   *
   * <p>This must never be less than 2.  The default value is 10.
   */
  int32_t getMergeFactor() const;
  void setMergeFactor(int32_t val);


  /** Expert: The fraction of terms in the "dictionary" which should be stored
   *   in RAM.  Smaller values use more memory, but make searching slightly
   *   faster, while larger values use less memory and make searching slightly
   *   slower.  Searching is typically not dominated by dictionary lookup, so
   *   tweaking this is rarely useful.
   */
  LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_TERM_INDEX_INTERVAL = 128);
  /** Expert: Set the interval between indexed terms.  Large values cause less
   * memory to be used by IndexReader, but slow random-access to terms.  Small
   * values cause more memory to be used by an IndexReader, and speed
   * random-access to terms.
   *
   * This parameter determines the amount of computation required per query
   * term, regardless of the number of documents that contain that term.  In
   * particular, it is the maximum number of other terms that must be
   * scanned before a term is located and its frequency and position information
   * may be processed.  In a large index with user-entered query terms, query
   * processing time is likely to be dominated not by term lookup but rather
   * by the processing of frequency and positional data.  In a small index
   * or when many uncommon query terms are generated (e.g., by wildcard
   * queries) term lookup may become a dominant cost.
   *
   * In particular, <code>numUniqueTerms/interval</code> terms are read into
   * memory by an IndexReader, and, on average, <code>interval/2</code> terms
   * must be scanned for each random term access.
   *
   * @see #DEFAULT_TERM_INDEX_INTERVAL
   */
  void setTermIndexInterval(int32_t interval);
  /** Expert: Return the interval between indexed terms.
   *
   * @see #setTermIndexInterval(int)
   */
  int32_t getTermIndexInterval();

  /**Determines the largest number of documents ever merged by addDocument().
   *  Small values (e.g., less than 10,000) are best for interactive indexing,
   *  as this limits the length of pauses while indexing to a few seconds.
   *  Larger values are best for batched indexing and speedier searches.
   *
   *  <p>The default value is {@link Integer#MAX_VALUE}.
   */
  int32_t getMaxMergeDocs() const;
  void setMaxMergeDocs(int32_t val);

  /**
   * Constructs an IndexWriter for the index in <code>path</code>.
   * Text will be analyzed with <code>a</code>.  If <code>create</code>
   * is true, then a new, empty index will be created in
   * <code>path</code>, replacing the index already there, if any.
   *
   * @param path the path to the index directory
   * @param a the analyzer to use
   * @param create <code>true</code> to create the index or overwrite
   *  the existing one; <code>false</code> to append to the existing
   *  index
   * @throws CorruptIndexException if the index is corrupt
   * @throws LockObtainFailedException if another writer
   *  has this index open (<code>write.lock</code> could not
   *  be obtained)
   * @throws IOException if the directory cannot be read/written to, or
   *  if it does not exist and <code>create</code> is
   *  <code>false</code> or if there is any other low-level
   *  IO error
   */
  explicit IndexWriter(const char* path, CL_NS(analysis)::Analyzer* a, const bool create);

  /**
   * Constructs an IndexWriter for the index in <code>d</code>.
   * Text will be analyzed with <code>a</code>.  If <code>create</code>
   * is true, then a new, empty index will be created in
   * <code>d</code>, replacing the index already there, if any.
   *
   * @param d the index directory
   * @param a the analyzer to use
   * @param create <code>true</code> to create the index or overwrite
   *  the existing one; <code>false</code> to append to the existing
   *  index
   * @throws CorruptIndexException if the index is corrupt
   * @throws LockObtainFailedException if another writer
   *  has this index open (<code>write.lock</code> could not
   *  be obtained)
   * @throws IOException if the directory cannot be read/written to, or
   *  if it does not exist and <code>create</code> is
   *  <code>false</code> or if there is any other low-level
   *  IO error
   */
  explicit IndexWriter(CL_NS(store)::Directory* d, CL_NS(analysis)::Analyzer* a, const bool create, const bool closeDirOnShutdown=false);

  /**
   * Expert: constructs an IndexWriter with a custom {@link
   * IndexDeletionPolicy}, for the index in <code>d</code>,
   * first creating it if it does not already exist.  Text
   * will be analyzed with <code>a</code>.
   *
   * @param d the index directory
   * @param autoCommit see <a href="#autoCommit">above</a>
   * @param a the analyzer to use
   * @param deletionPolicy see <a href="#deletionPolicy">above</a>
   * @throws CorruptIndexException if the index is corrupt
   * @throws LockObtainFailedException if another writer
   *  has this index open (<code>write.lock</code> could not
   *  be obtained)
   * @throws IOException if the directory cannot be
   *  read/written to or if there is any other low-level
   *  IO error
   */
  explicit IndexWriter(CL_NS(store)::Directory* d, const bool autoCommit, CL_NS(analysis)::Analyzer* a, IndexDeletionPolicy* deletionPolicy = NULL, const bool closeDirOnShutdown=false);

  /**
   * Expert: constructs an IndexWriter with a custom {@link
   * IndexDeletionPolicy}, for the index in <code>d</code>.
   * Text will be analyzed with <code>a</code>.  If
   * <code>create</code> is true, then a new, empty index
   * will be created in <code>d</code>, replacing the index
   * already there, if any.
   *
   * @param d the index directory
   * @param autoCommit see <a href="#autoCommit">above</a>
   * @param a the analyzer to use
   * @param create <code>true</code> to create the index or overwrite
   *  the existing one; <code>false</code> to append to the existing
   *  index
   * @param deletionPolicy see <a href="#deletionPolicy">above</a>
   * @throws CorruptIndexException if the index is corrupt
   * @throws LockObtainFailedException if another writer
   *  has this index open (<code>write.lock</code> could not
   *  be obtained)
   * @throws IOException if the directory cannot be read/written to, or
   *  if it does not exist and <code>create</code> is
   *  <code>false</code> or if there is any other low-level
   *  IO error
   */
  explicit IndexWriter(CL_NS(store)::Directory* d, const bool autoCommit, CL_NS(analysis)::Analyzer* a, const bool create, IndexDeletionPolicy* deletionPolicy = NULL, const bool closeDirOnShutdown=false);

  /**Returns the number of documents currently in this index.
   *  synchronized
   */
  int32_t docCount();


  /** Returns the directory this index resides in. */
  CL_NS(store)::Directory* getDirectory();

  /** Get the current setting of whether to use the compound file format.
   *  Note that this just returns the value you set with setUseCompoundFile(boolean)
   *  or the default. You cannot use this to query the status of an existing index.
   *  @see #setUseCompoundFile(boolean)
   */
  bool getUseCompoundFile();

  /** Setting to turn on usage of a compound file. When on, multiple files
   *  for each segment are merged into a single file once the segment creation
   *  is finished. This is done regardless of what directory is in use.
   */
  void setUseCompoundFile(bool value);


  /** Expert: Set the Similarity implementation used by this IndexWriter.
   *
   * @see Similarity#setDefault(Similarity)
   */
  void setSimilarity(CL_NS(search)::Similarity* similarity);

  /** Expert: Return the Similarity implementation used by this IndexWriter.
   *
   * <p>This defaults to the current value of {@link Similarity#getDefault()}.
   */
  CL_NS(search)::Similarity* getSimilarity();

  /** Returns the analyzer used by this index. */
  CL_NS(analysis)::Analyzer* getAnalyzer();

  // synchronized
  std::string newSegmentName();

  /**
   * Prints a message to the infoStream (if non-null),
   * prefixed with the identifying information for this
   * writer and the thread that's calling it.
   */
  void message(std::string message);

  /**
   * Returns the current default infoStream for newly
   * instantiated IndexWriters.
   * @see #setDefaultInfoStream
   */
  static std::ostream* getDefaultInfoStream();

  /**
   * Returns the current infoStream in use by this writer.
   * @see #setInfoStream
   */
  std::ostream* getInfoStream();

  /**
   * Returns the number of buffered deleted terms that will
   * trigger a flush if enabled.
   * @see #setMaxBufferedDeleteTerms
   */
  int32_t getMaxBufferedDeleteTerms();

  /**
   * Expert: returns the current MergePolicy in use by this writer.
   * @see #setMergePolicy
   */
  MergePolicy* getMergePolicy();

  /**
   * Expert: returns the current MergePolicy in use by this
   * writer.
   * @see #setMergePolicy
   */
  MergeScheduler* getMergeScheduler();

  /**
   * Returns the value set by {@link #setRAMBufferSizeMB} if enabled.
   */
  float_t getRAMBufferSizeMB();

  /** If non-null, this will be the default infoStream used
   * by a newly instantiated IndexWriter.
   * @see #setInfoStream
   */
  static void setDefaultInfoStream(std::ostream* infoStream);\

  /** If non-null, information about merges, deletes and a
   * message when maxFieldLength is reached will be printed
   * to this.
   */
  void setInfoStream(std::ostream* infoStream);

  /**
   * <p>Determines the minimal number of delete terms required before the buffered
   * in-memory delete terms are applied and flushed. If there are documents
   * buffered in memory at the time, they are merged and a new segment is
   * created.</p>

   * <p>Disabled by default (writer flushes by RAM usage).</p>
   *
   * @throws IllegalArgumentException if maxBufferedDeleteTerms
   * is enabled but smaller than 1
   * @see #setRAMBufferSizeMB
   */
  void setMaxBufferedDeleteTerms(int32_t maxBufferedDeleteTerms);

  /**
   * Expert: set the merge policy used by this writer.
   */
  void setMergePolicy(MergePolicy* mp);

  /**
   * Expert: set the merge scheduler used by this writer.
   */
  void setMergeScheduler(MergeScheduler* mergeScheduler);

  /** Determines the amount of RAM that may be used for
   * buffering added documents before they are flushed as a
   * new Segment.  Generally for faster indexing performance
   * it's best to flush by RAM usage instead of document
   * count and use as large a RAM buffer as you can.
   *
   * <p>When this is set, the writer will flush whenever
   * buffered documents use this much RAM.  Pass in {@link
   * #DISABLE_AUTO_FLUSH} to prevent triggering a flush due
   * to RAM usage.  Note that if flushing by document count
   * is also enabled, then the flush will be triggered by
   * whichever comes first.</p>
   *
   * <p> The default value is {@link #DEFAULT_RAM_BUFFER_SIZE_MB}.</p>
   *
   * @throws IllegalArgumentException if ramBufferSize is
   * enabled but non-positive, or it disables ramBufferSize
   * when maxBufferedDocs is already disabled
   */
  void setRAMBufferSizeMB(float_t mb);


  /** Expert: the {@link MergeScheduler} calls this method
   *  to retrieve the next merge requested by the
   *  MergePolicy */
  MergePolicy::OneMerge* getNextMerge();

  /**
   * Merges the indicated segments, replacing them in the stack with a
   * single segment.
   */
  void merge(MergePolicy::OneMerge* merge);

  /**
   * Deletes the document(s) containing <code>term</code>.
   * @param term the term to identify the documents to be deleted
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void deleteDocuments(Term* term);

  /**
   * Deletes the document(s) containing any of the
   * terms. All deletes are flushed at the same time.
   * @param terms array of terms to identify the documents
   * to be deleted
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void deleteDocuments(const CL_NS(util)::ArrayBase<Term*>* terms);

  /**
   * Updates a document by first deleting the document(s)
   * containing <code>term</code> and then adding the new
   * document.  The delete and then add are atomic as seen
   * by a reader on the same index (flush may happen only after
   * the add).
   * @param term the term to identify the document(s) to be
   * deleted
   * @param doc the document to be added
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void updateDocument(Term* term, CL_NS(document)::Document* doc);

  /**
   * Updates a document by first deleting the document(s)
   * containing <code>term</code> and then adding the new
   * document.  The delete and then add are atomic as seen
   * by a reader on the same index (flush may happen only after
   * the add).
   * @param term the term to identify the document(s) to be
   * deleted
   * @param doc the document to be added
   * @param analyzer the analyzer to use when analyzing the document
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void updateDocument(Term* term, CL_NS(document)::Document* doc, CL_NS(analysis)::Analyzer* analyzer);

  /**
   * Returns default write lock timeout for newly
   * instantiated IndexWriters.
   * @see #setDefaultWriteLockTimeout
   */
  int64_t getDefaultWriteLockTimeout();

  /**
   * Sets the default (for any instance of IndexWriter) maximum time to wait for a write lock (in
   * milliseconds).
   */
  void setDefaultWriteLockTimeout(int64_t writeLockTimeout);

  std::string segString();

  /**
   * Closes the index with or without waiting for currently
   * running merges to finish.  This is only meaningful when
   * using a MergeScheduler that runs merges in background
   * threads.
   * @param waitForMerges if true, this call will block
   * until all merges complete; else, it will ask all
   * running merges to abort, wait until those merges have
   * finished (which should be at most a few seconds), and
   * then return.
   *
   * <p> If an Exception is hit during close, eg due to disk
   * full or some other reason, then both the on-disk index
   * and the internal state of the IndexWriter instance will
   * be consistent.  However, the close will not be complete
   * even though part of it (flushing buffered documents)
   * may have succeeded, so the write lock will still be
   * held.</p>
   *
   * <p> If you can correct the underlying cause (eg free up
   * some disk space) then you can call close() again.
   * Failing that, if you want to force the write lock to be
   * released (dangerous, because you may then lose buffered
   * docs in the IndexWriter instance) then you can do
   * something like this:</p>
   *
   * <pre>
   * try {
   *   writer.close();
   * } finally {
   *   if (IndexReader.isLocked(directory)) {
   *     IndexReader.unlock(directory);
   *   }
   * }
   * </pre>
   *
   * after which, you must be certain not to use the writer
   * instance anymore.</p>
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void close(bool waitForMerges=true);

  /**
   * Requests an "optimize" operation on an index, priming the index
   * for the fastest available search. Traditionally this has meant
   * merging all segments into a single segment as is done in the
   * default merge policy, but individaul merge policies may implement
   * optimize in different ways.
   *
   * @see LogMergePolicy#findMergesForOptimize
   *
   * <p>It is recommended that this method be called upon completion of indexing.  In
   * environments with frequent updates, optimize is best done during low volume times, if at all.
   *
   * </p>
   * <p>See http://www.gossamer-threads.com/lists/lucene/java-dev/47895 for more discussion. </p>
   *
   * <p>Note that this can require substantial temporary free
   * space in the Directory (see <a target="_top"
   * href="http://issues.apache.org/jira/browse/LUCENE-764">LUCENE-764</a>
   * for details):</p>
   *
   * <ul>
   * <li>
   *
   * <p>If no readers/searchers are open against the index,
   * then free space required is up to 1X the total size of
   * the starting index.  For example, if the starting
   * index is 10 GB, then you must have up to 10 GB of free
   * space before calling optimize.</p>
   *
   * <li>
   *
   * <p>If readers/searchers are using the index, then free
   * space required is up to 2X the size of the starting
   * index.  This is because in addition to the 1X used by
   * optimize, the original 1X of the starting index is
   * still consuming space in the Directory as the readers
   * are holding the segments files open.  Even on Unix,
   * where it will appear as if the files are gone ("ls"
   * won't list them), they still consume storage due to
   * "delete on last close" semantics.</p>
   *
   * <p>Furthermore, if some but not all readers re-open
   * while the optimize is underway, this will cause > 2X
   * temporary space to be consumed as those new readers
   * will then hold open the partially optimized segments at
   * that time.  It is best not to re-open readers while
   * optimize is running.</p>
   *
   * </ul>
   *
   * <p>The actual temporary usage could be much less than
   * these figures (it depends on many factors).</p>
   *
   * <p>In general, once the optimize completes, the total size of the
   * index will be less than the size of the starting index.
   * It could be quite a bit smaller (if there were many
   * pending deletes) or just slightly smaller.</p>
   *
   * <p>If an Exception is hit during optimize(), for example
   * due to disk full, the index will not be corrupt and no
   * documents will have been lost.  However, it may have
   * been partially optimized (some segments were merged but
   * not all), and it's possible that one of the segments in
   * the index will be in non-compound format even when
   * using compound file format.  This will occur when the
   * Exception is hit during conversion of the segment into
   * compound format.</p>
   *
   * <p>This call will optimize those segments present in
   * the index when the call started.  If other threads are
   * still adding documents and flushing segments, those
   * newly created segments will not be optimized unless you
   * call optimize again.</p>
   *
   * @param doWait Specifies whether the call should block
   *  until the optimize completes.  This is only meaningful
   *  with a {@link MergeScheduler} that is able to run merges
   *  in background threads.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void optimize(bool doWait=true);

  /**
   * Optimize the index down to <= maxNumSegments.  If
   * maxNumSegments==1 then this is the same as {@link
   * #optimize()}.
   * @param maxNumSegments maximum number of segments left
   * in the index after optimization finishes
   * @param doWait Specifies whether the call should block
   *  until the optimize completes.  This is only meaningful
   *  with a {@link MergeScheduler} that is able to run merges
   *   in background threads.
   */
  void optimize(int32_t maxNumSegments, bool doWait=true);

  /**
   * Flush all in-memory buffered updates (adds and deletes)
   * to the Directory.
   * <p>Note: if <code>autoCommit=false</code>, flushed data would still
   * not be visible to readers, until {@link #close} is called.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void flush();

  /**
   * Adds a document to this index.  If the document contains more than
   * {@link #setMaxFieldLength(int)} terms for a given field, the remainder are
   * discarded (depending on the policy, see #FIELD_TRUNC_POLICY__WARN)
   *
   * <p> Note that if an Exception is hit (for example disk full)
   * then the index will be consistent, but this document
   * may not have been added.  Furthermore, it's possible
   * the index will have one segment in non-compound format
   * even when using compound files (when a merge has
   * partially succeeded).</p>
   *
   * <p> This method periodically flushes pending documents
   * to the Directory (every {@link #setMaxBufferedDocs}),
   * and also periodically merges segments in the index
   * (every {@link #setMergeFactor} flushes).  When this
   * occurs, the method will take more time to run (possibly
   * a long time if the index is large), and will require
   * free temporary space in the Directory to do the
   * merging.</p>
   *
   * <p>The amount of free space required when a merge is triggered is
   * up to 1X the size of all segments being merged, when no
   * readers/searchers are open against the index, and up to 2X the
   * size of all segments being merged when readers/searchers are open
   * against the index (see {@link #optimize()} for details). The
   * sequence of primitive merge operations performed is governed by
   * the merge policy.
   *
   * <p>Note that each term in the document can be no longer
   * than 16383 characters, otherwise an
   * IllegalArgumentException will be thrown.</p>
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @param analyzer use the provided analyzer instead of the
   * value of {@link #getAnalyzer()}
   */
  void addDocument(CL_NS(document)::Document* doc, CL_NS(analysis)::Analyzer* analyzer=NULL);


  /**
   * Expert: asks the mergePolicy whether any merges are
   * necessary now and if so, runs the requested merges and
   * then iterate (test again if merges are needed) until no
   * more merges are returned by the mergePolicy.
   *
   * Explicit calls to maybeMerge() are usually not
   * necessary. The most common case is when merge policy
   * parameters have changed.
   */
  void maybeMerge();

  /**
   * Close the <code>IndexWriter</code> without committing
   * any of the changes that have occurred since it was
   * opened. This removes any temporary files that had been
   * created, after which the state of the index will be the
   * same as it was when this writer was first opened.  This
   * can only be called when this IndexWriter was opened
   * with <code>autoCommit=false</code>.
   * @throws IllegalStateException if this is called when
   *  the writer was opened with <code>autoCommit=true</code>.
   * @throws IOException if there is a low-level IO error
   */
  void abort();


  /**
   * Merges all segments from an array of indexes into this index.
   * <p>
   * This is similar to addIndexes(Directory[]). However, no optimize()
   * is called either at the beginning or at the end. Instead, merges
   * are carried out as necessary.
   *
   * <p><b>NOTE:</b> the index in each Directory must not be
   * changed (opened by a writer) while this method is
   * running.  This method does not acquire a write lock in
   * each input Directory, so it is up to the caller to
   * enforce this.
   *
   * <p><b>NOTE:</b> while this is running, any attempts to
   * add or delete documents (with another thread) will be
   * paused until this method completes.
   *
   * <p>
   * This requires this index not be among those to be added, and the
   * upper bound* of those segment doc counts not exceed maxMergeDocs.
   *
   * <p>See {@link #addIndexes(Directory[])} for
   * details on transactional semantics, temporary free
   * space required in the Directory, and non-CFS segments
   * on an Exception.</p>
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void addIndexesNoOptimize(CL_NS(util)::ArrayBase<CL_NS(store)::Directory*>& dirs);

  /** Merges the provided indexes into this index.
   * <p>After this completes, the index is optimized. </p>
   * <p>The provided IndexReaders are not closed.</p>

   * <p><b>NOTE:</b> the index in each Directory must not be
   * changed (opened by a writer) while this method is
   * running.  This method does not acquire a write lock in
   * each input Directory, so it is up to the caller to
   * enforce this.
   *
   * <p><b>NOTE:</b> while this is running, any attempts to
   * add or delete documents (with another thread) will be
   * paused until this method completes.
   *
   * <p>See {@link #addIndexes(Directory[])} for
   * details on transactional semantics, temporary free
   * space required in the Directory, and non-CFS segments
   * on an Exception.</p>
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  //NOT IMPLEMENTED YET: void addIndexes(CL_NS(util)::ArrayBase<IndexReader*>& readers);

  /** Merges all segments from an array of indexes into this index.
   *
   * <p>This may be used to parallelize batch indexing.  A large document
   * collection can be broken into sub-collections.  Each sub-collection can be
   * indexed in parallel, on a different thread, process or machine.  The
   * complete index can then be created by merging sub-collection indexes
   * with this method.
   *
   * <p><b>NOTE:</b> the index in each Directory must not be
   * changed (opened by a writer) while this method is
   * running.  This method does not acquire a write lock in
   * each input Directory, so it is up to the caller to
   * enforce this.
   *
   * <p><b>NOTE:</b> while this is running, any attempts to
   * add or delete documents (with another thread) will be
   * paused until this method completes.
   *
   * <p>After this completes, the index is optimized.
   *
   * <p>This method is transactional in how Exceptions are
   * handled: it does not commit a new segments_N file until
   * all indexes are added.  This means if an Exception
   * occurs (for example disk full), then either no indexes
   * will have been added or they all will have been.</p>
   *
   * <p>If an Exception is hit, it's still possible that all
   * indexes were successfully added.  This happens when the
   * Exception is hit when trying to build a CFS file.  In
   * this case, one segment in the index will be in non-CFS
   * format, even when using compound file format.</p>
   *
   * <p>Also note that on an Exception, the index may still
   * have been partially or fully optimized even though none
   * of the input indexes were added. </p>
   *
   * <p>Note that this requires temporary free space in the
   * Directory up to 2X the sum of all input indexes
   * (including the starting index).  If readers/searchers
   * are open against the starting index, then temporary
   * free space required will be higher by the size of the
   * starting index (see {@link #optimize()} for details).
   * </p>
   *
   * <p>Once this completes, the final size of the index
   * will be less than the sum of all input index sizes
   * (including the starting index).  It could be quite a
   * bit smaller (if there were many pending deletes) or
   * just slightly smaller.</p>
   *
   * <p>See <a target="_top"
   * href="http://issues.apache.org/jira/browse/LUCENE-702">LUCENE-702</a>
   * for details.</p>
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  void addIndexes(CL_NS(util)::ArrayBase<CL_NS(store)::Directory*>& dirs);

  /** Expert:  Return the total size of all index files currently cached in memory.
   * Useful for size management with flushRamDocs()
   */
  int64_t ramSizeInBytes();

  /** Expert:  Return the number of documents whose segments are currently cached in memory.
   * Useful when calling flush()
   */
  int32_t numRamDocs();

  /** for testing only */
  virtual bool testPoint(const char* name);
private:
  class LockWith2;
  class LockWithCFS;
  friend class LockWith2;
  friend class LockWithCFS;
  friend class DocumentsWriter;

  /** Merges all RAM-resident segments. */
  void flushRamSegments();

  /** Incremental segment merger. */
  void maybeMergeSegments();

  /** Pops segments off of segmentInfos stack down to minSegment, merges them,
   * 	and pushes the merged index onto the top of the segmentInfos stack.
   */
  void mergeSegments(const uint32_t minSegment);

  /** Merges the named range of segments, replacing them in the stack with a
   * single segment. */
  void mergeSegments(const uint32_t minSegment, const uint32_t end);

  void deleteFiles(std::vector<std::string>& files);
  void readDeleteableFiles(std::vector<std::string>& files);
  void writeDeleteableFiles(std::vector<std::string>& files);

  /*
   * Some operating systems (e.g. Windows) don't permit a file to be deleted
   * while it is opened for read (e.g. by another process or thread). So we
   * assume that when a delete fails it is because the file is open in another
   * process, and queue the file for subsequent deletion.
   */
  void deleteSegments(CL_NS(util)::CLVector<SegmentReader*>* segments);
  void deleteFiles(std::vector<std::string>& files, CL_NS(store)::Directory* directory);
  void deleteFiles(std::vector<std::string>& files, std::vector<std::string>& deletable);

  /**
   * Casts current mergePolicy to LogMergePolicy, and throws
   * an exception if the mergePolicy is not a LogMergePolicy.
   */
  LogMergePolicy* getLogMergePolicy() const;

  void setMessageID();

  void closeInternal(bool waitForMerges);
  void messageState();

  /**
   * If we are flushing by doc count (not by RAM usage), and
   * using LogDocMergePolicy then push maxBufferedDocs down
   * as its minMergeDocs, to keep backwards compatibility.
   */
  void pushMaxBufferedDocs();

  void finishMerges(bool waitForMerges);

  /** Tells the docWriter to close its currently open shared
   *  doc stores (stored fields & vectors files).
   *  Return value specifices whether new doc store files are compound or not.
   */
  bool flushDocStores();

  //for test purposes
protected:
  int32_t getDocCount(int32_t i);
  int32_t getNumBufferedDocuments();
  int32_t getSegmentCount();
  int32_t getBufferedDeleteTermsSize();
  int32_t getNumBufferedDeleteTerms();
  virtual SegmentInfo* newestSegment();

private:
  void waitForClose();
  void deletePartialSegmentsFile();

  /** Returns true if any merges in pendingMerges or
   *  runningMerges are optimization merges. */
  bool optimizeMergesPending();

  void resetMergeExceptions();

  void updatePendingMerges(int32_t maxNumSegmentsOptimize, bool optimize);

  /*
   * Begin a transaction.  During a transaction, any segment
   * merges that happen (or ram segments flushed) will not
   * write a new segments file and will not remove any files
   * that were present at the start of the transaction.  You
   * must make a matched (try/finally) call to
   * commitTransaction() or rollbackTransaction() to finish
   * the transaction.
   *
   * Note that buffered documents and delete terms are not handled
   * within the transactions, so they must be flushed before the
   * transaction is started.
   */
  void startTransaction();

  /*
   * Rolls back the transaction and restores state to where
   * we were at the start.
   */
  void rollbackTransaction();

  /*
   * Commits the transaction.  This will write the new
   * segments file and remove and pending deletions we have
   * accumulated during the transaction
   */
  void commitTransaction();


  void maybeMerge(bool optimize);
  void maybeMerge(int32_t maxNumSegmentsOptimize, bool optimize);
  /** Does initial setup for a merge, which is fast but holds
   *  the synchronized lock on IndexWriter instance.  */
  void mergeInit(MergePolicy::OneMerge* _merge);
  void _mergeInit(MergePolicy::OneMerge* _merge);

  /* If any of our segments are using a directory != ours
   * then copy them over.  Currently this is only used by
   * addIndexesNoOptimize(). */
  void copyExternalSegments();


  /*
   * Called whenever the SegmentInfos has been updated and
   * the index files referenced exist (correctly) in the
   * index directory->  If we are in autoCommit mode, we
   * commit the change immediately.  Else, we mark
   * commitPending.
   */
  void checkpoint();

  bool doFlush(bool flushDocStores);

  /* FIXME if we want to support non-contiguous segment merges */
  bool commitMerge(MergePolicy::OneMerge* merge);

  int32_t ensureContiguousMerge(MergePolicy::OneMerge* merge);

  void decrefMergeSegments(MergePolicy::OneMerge* _merge);

  /** Does fininishing for a merge, which is fast but holds
   *  the synchronized lock on IndexWriter instance. */
  void mergeFinish(MergePolicy::OneMerge* _merge);

  /** Does the actual (time-consuming) work of the merge,
   *  but without holding synchronized lock on IndexWriter
   *  instance */
  int32_t mergeMiddle(MergePolicy::OneMerge* _merge);

  void addMergeException(MergePolicy::OneMerge* _merge);

  /** Checks whether this merge involves any segments
   *  already participating in a merge.  If not, this merge
   *  is "registered", meaning we record that its segments
   *  are now participating in a merge, and true is
   *  returned.  Else (the merge conflicts) false is
   *  returned. */
  bool registerMerge(MergePolicy::OneMerge* _merge);

  // Called during flush to apply any buffered deletes.  If
  // flushedNewSegment is true then a new segment was just
  // created and flushed from the ram segments, so we will
  // selectively apply the deletes to that new segment.
  void applyDeletes(bool flushedNewSegment);

  class Internal;
  Internal* _internal;
protected:
  // This is called after pending added and deleted
  // documents have been flushed to the Directory but before
  // the change is committed (_CLNEW segments_N file written).
  virtual void doAfterFlush();

  /**
   * Used internally to throw an {@link
   * AlreadyClosedException} if this IndexWriter has been
   * closed.
   * @throws AlreadyClosedException if this IndexWriter is
   */
  void ensureOpen();

  /**
   * Flush all in-memory buffered udpates (adds and deletes)
   * to the Directory.
   * @param triggerMerge if true, we may merge segments (if
   *  deletes or docs were flushed) if necessary
   * @param flushDocStores if false we are allowed to keep
   *  doc stores open to share with the next segment
   */
  void flush(bool triggerMerge, bool flushDocStores);
};

CL_NS_END
#endif
