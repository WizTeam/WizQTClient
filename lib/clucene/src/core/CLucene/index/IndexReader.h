/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_IndexReader_
#define _lucene_index_IndexReader_


#include "CLucene/util/Array.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/LuceneThreads.h"

CL_CLASS_DEF(store,Directory)
CL_CLASS_DEF(store,LuceneLock)
CL_CLASS_DEF(document,Document)
CL_CLASS_DEF(document,FieldSelector)

CL_NS_DEF(index)
class SegmentInfos;
class TermFreqVector;
class TermEnum;
class Term;
class TermDocs;
class TermPositions;
class IndexDeletionPolicy;
class TermVectorMapper;

/** IndexReader is an abstract class, providing an interface for accessing an
 index.  Search of an index is done entirely through this abstract interface,
 so that any subclass which implements it is searchable.

 <p> Concrete subclasses of IndexReader are usually constructed with a call to
 one of the static <code>open()</code> methods, e.g. {@link #open(String)}.

 <p> For efficiency, in this API documents are often referred to via
 <i>document numbers</i>, non-negative integers which each name a unique
 document in the index.  These document numbers are ephemeral--they may change
 as documents are added to and deleted from an index.  Clients should thus not
 rely on a given document having the same number between sessions.

 <p> An IndexReader can be opened on a directory for which an IndexWriter is
 opened already, but it cannot be used to delete documents from the index then.

 <p>
 NOTE: for backwards API compatibility, several methods are not listed
 as abstract, but have no useful implementations in this base class and
 instead always throw UnsupportedOperationException.  Subclasses are
 strongly encouraged to override these methods, but in many cases may not
 need to.
 </p>

*/
class CLUCENE_EXPORT IndexReader: public CL_NS(util)::NamedObject{
  bool closed;
protected:
  bool hasChanges;

  /**
  * Legacy Constructor for backwards compatibility.
  *
  * <p>
  * This Constructor should not be used, it exists for backwards
  * compatibility only to support legacy subclasses that did not "own"
  * a specific directory, but needed to specify something to be returned
  * by the directory() method.  Future subclasses should delegate to the
  * no arg constructor and implement the directory() method as appropriate.
  *
  * @param directory Directory to be returned by the directory() method
  * @see #directory()
  * @deprecated - use IndexReader()
  */
  IndexReader(CL_NS(store)::Directory* dir);

  IndexReader();

  /// Implements close.
  virtual void doClose() = 0;

  /** Implements setNorm in subclass.*/
  virtual void doSetNorm(int32_t doc, const TCHAR* field, uint8_t value) = 0;

  /** Implements actual undeleteAll() in subclass. */
  virtual void doUndeleteAll() = 0;

  /** Implements deletion of the document numbered <code>docNum</code>.
  * Applications should call {@link #deleteDocument(int32_t)} or {@link #deleteDocuments(Term*)}.
  */
  virtual void doDelete(const int32_t docNum) = 0;

  /**
  * @throws AlreadyClosedException if this IndexReader is closed
  */
  virtual void ensureOpen();

  /** Does nothing by default. Subclasses that require a write lock for
   *  index modifications must implement this method. */
  virtual void acquireWriteLock();

public:
	//Callback for classes that need to know if IndexReader is closing.
	typedef void (*CloseCallback)(IndexReader*, void*);

  /** Internal use. Implements commit */
  virtual void doCommit() = 0;
  /** Internal use. */
	class Internal;
  /** Internal use. */
	Internal* _internal;


  /**
   * Constants describing field properties, for example used for
   * {@link IndexReader#getFieldNames(FieldOption)}.
   */
	enum FieldOption {
		/** all fields */
		ALL = 1,
		/** all indexed fields */
		INDEXED = 2,
		/** all fields which are not indexed */
		UNINDEXED = 4,
		/** all fields which are indexed with termvectors enables */
		INDEXED_WITH_TERMVECTOR = 8,
		/** all fields which are indexed but don't have termvectors enabled */
		INDEXED_NO_TERMVECTOR = 16,
		/** all fields where termvectors are enabled. Please note that only standard termvector fields are returned */
		TERMVECTOR = 32,
		/** all field with termvectors wiht positions enabled */
		TERMVECTOR_WITH_POSITION = 64,
		/** all fields where termvectors with offset position are set */
		TERMVECTOR_WITH_OFFSET = 128,
		/** all fields where termvectors with offset and position values set */
		TERMVECTOR_WITH_POSITION_OFFSET = 256,
		/** all fields that store payloads */
		STORES_PAYLOADS = 512
	};

  /** Returns an IndexReader reading the index in an FSDirectory in the named
   path.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @param path the path to the index directory */
  static IndexReader* open(const char* path, bool closeDirectoryOnCleanup=true, IndexDeletionPolicy* deletionPolicy=NULL);

  /** Expert: returns an IndexReader reading the index in the given
   * Directory, with a custom {@link IndexDeletionPolicy}.
   * @param directory the index directory
   * @param deletionPolicy a custom deletion policy (only used
   *  if you use this reader to perform deletes or to set
   *  norms); see {@link IndexWriter} for details.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  static IndexReader* open(CL_NS(store)::Directory* directory, bool closeDirectoryOnCleanup=false, IndexDeletionPolicy* deletionPolicy=NULL);


  /**
   * Refreshes an IndexReader if the index has changed since this instance
   * was (re)opened.
   * <p>
   * Opening an IndexReader is an expensive operation. This method can be used
   * to refresh an existing IndexReader to reduce these costs. This method
   * tries to only load segments that have changed or were created after the
   * IndexReader was (re)opened.
   * <p>
   * If the index has not changed since this instance was (re)opened, then this
   * call is a NOOP and returns this instance. Otherwise, a new instance is
   * returned. The old instance <B>is</b> closed (unlink JLucene) and must
   * be deleted<br>
   * <p>
   * You can determine whether a reader was actually reopened by comparing the
   * old instance with the instance returned by this method:
   * <pre>
   * IndexReader* reader = ...
   * ...
   * IndexReader* newreader = r->reopen();
   * if (newreader != reader) {
   *   ...     // reader was reopened
   *   reader->close();
   *   _CLDELETE(reader);
   * }
   * reader = newreader;
   * ...
   * </pre>
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  virtual IndexReader* reopen();

  /**
   * Returns the directory associated with this index.  The Default
   * implementation returns the directory specified by subclasses when
   * delegating to the IndexReader(Directory) constructor, or throws an
   * UnsupportedOperationException if one was not specified.
   * @throws UnsupportedOperationException if no directory
   */
  virtual CL_NS(store)::Directory* directory();

	DEFINE_MUTEX(THIS_LOCK)

  /**
   *
   * @throws IOException
   */
  virtual void flush();

  /**
   * Commit changes resulting from delete, undeleteAll, or
   * setNorm operations
   *
   * If an exception is hit, then either no changes or all
   * changes will have been committed to the index
   * (transactional semantics).
   * @throws IOException if there is a low-level IO error
   */
	CLUCENE_LOCAL_DECL virtual void commit();


	/** Undeletes all documents currently marked as deleted in this index.
   *
   * @throws StaleReaderException if the index has changed
   *  since this reader was opened
   * @throws LockObtainFailedException if another writer
   *  has this index open (<code>write.lock</code> could not
   *  be obtained)
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
	virtual void undeleteAll();

	/**
	* Get a list of unique field names that exist in this index and have the specified
	* field option information.
	* @param fldOption specifies which field option should be available for the returned fields
	* @return Collection of Strings indicating the names of the fields.
	* @see IndexReader.FieldOption
	*/
	virtual void getFieldNames(FieldOption fldOption, StringArrayWithDeletor& retarray) = 0;

	/** Returns the byte-encoded normalization factor for the named field of
	* every document.  This is used by the search code to score documents.
	*
	* The number of bytes returned is the size of the IndexReader->maxDoc()
	*
	* @see Field#setBoost(float_t)
	* @memory The values are cached, so don't delete the returned byte array.
	*/
	virtual uint8_t* norms(const TCHAR* field) = 0;


	/** Reads the byte-encoded normalization factor for the named field of every
	*  document.  This is used by the search code to score documents.
	*
	* The size of bytes must be the size of the IndexReader->maxDoc()
	*
	* @see Field#setBoost(float_t)
	*/
	virtual void norms(const TCHAR* field, uint8_t* bytes) = 0;

  /** Expert: Resets the normalization factor for the named field of the named
  * document.
  *
  * @see #norms(TCHAR*)
  * @see Similarity#decodeNorm(uint8_t)
  *
  * @throws StaleReaderException if the index has changed
  *  since this reader was opened
  * @throws CorruptIndexException if the index is corrupt
  * @throws LockObtainFailedException if another writer
  *  has this index open (<code>write.lock</code> could not
  *  be obtained)
  * @throws IOException if there is a low-level IO error
  */
  void setNorm(int32_t doc, const TCHAR* field, float_t value);

  /** Expert: Resets the normalization factor for the named field of the named
  * document.  The norm represents the product of the field's {@link
  * Field#setBoost(float_t) boost} and its {@link Similarity#lengthNorm(TCHAR*,
  * int32_t) length normalization}.  Thus, to preserve the length normalization
  * values when resetting this, one should base the new value upon the old.
  *
  * @see #norms(TCHAR*)
  * @see Similarity#decodeNorm(uint8_t)
  * @throws StaleReaderException if the index has changed
  *  since this reader was opened
  * @throws CorruptIndexException if the index is corrupt
  * @throws LockObtainFailedException if another writer
  *  has this index open (<code>write.lock</code> could not
  *  be obtained)
  * @throws IOException if there is a low-level IO error
  */
  void setNorm(int32_t doc, const TCHAR* field, uint8_t value);

  /// Release the write lock, if needed.
  virtual ~IndexReader();

	/**
	* Returns the time the index in the named directory was last modified.
	* Do not use this to check whether the reader is still up-to-date, use
	* {@link #isCurrent()} instead.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
	*/
	static uint64_t lastModified(const char* directory);

	/**
	* Returns the time the index in the named directory was last modified.
	* Do not use this to check whether the reader is still up-to-date, use
	* {@link #isCurrent()} instead.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
	*/
	static uint64_t lastModified(CL_NS(store)::Directory* directory);


	/**
	* Reads version number from segments files. The version number is
	* initialized with a timestamp and then increased by one for each change of
	* the index.
	*
	* @param directory where the index resides.
	* @return version number.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
	*/
	static int64_t getCurrentVersion(CL_NS(store)::Directory* directory);

	/**
   * Reads version number from segments files. The version number is
   * initialized with a timestamp and then increased by one for each change of
   * the index.
   *
   * @param directory where the index resides.
   * @return version number.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
	static int64_t getCurrentVersion(const char* directory);

  /**
   * Version number when this IndexReader was opened. Not implemented in the IndexReader base class.
   * @throws UnsupportedOperationException unless overridden in subclass
   */
	virtual int64_t getVersion();

  /**<p>For IndexReader implementations that use
   * TermInfosReader to read terms, this sets the
   * indexDivisor to subsample the number of indexed terms
   * loaded into memory.  This has the same effect as {@link
   * IndexWriter#setTermIndexInterval} except that setting
   * must be done at indexing time while this setting can be
   * set per reader.  When set to N, then one in every
   * N*termIndexInterval terms in the index is loaded into
   * memory.  By setting this to a value > 1 you can reduce
   * memory usage, at the expense of higher latency when
   * loading a TermInfo.  The default value is 1.</p>
   *
   * <b>NOTE:</b> you must call this before the term
   * index is loaded.  If the index is already loaded,
   * an IllegalStateException is thrown.
   * @throws IllegalStateException if the term index has already been loaded into memory
   */
  void setTermInfosIndexDivisor(int32_t indexDivisor);

  /** <p>For IndexReader implementations that use
   *  TermInfosReader to read terms, this returns the
   *  current indexDivisor.
   *  @see #setTermInfosIndexDivisor */
  int32_t getTermInfosIndexDivisor();

  /**
   * Check whether this IndexReader is still using the
   * current (i.e., most recently committed) version of the
   * index.  If a writer has committed any changes to the
   * index since this reader was opened, this will return
   * <code>false</code>, in which case you must open a new
   * IndexReader in order to see the changes.  See the
   * description of the <a href="IndexWriter.html#autoCommit"><code>autoCommit</code></a>
   * flag which controls when the {@link IndexWriter}
   * actually commits changes to the index.
   *
   * <p>
   * Not implemented in the IndexReader base class.
   * </p>
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @throws UnsupportedOperationException unless overridden in subclass
   */
	virtual bool isCurrent();

  /**
   * Checks is the index is optimized (if it has a single segment and
   * no deletions).  Not implemented in the IndexReader base class.
   * @return <code>true</code> if the index is optimized; <code>false</code> otherwise
   * @throws UnsupportedOperationException unless overridden in subclass
   */
  virtual bool isOptimized();

  /**
   *  Return an array of term frequency vectors for the specified document.
   *  The array contains a vector for each vectorized field in the document.
   *  Each vector contains terms and frequencies for all terms in a given vectorized field.
   *  If no such fields existed, the method returns null. The term vectors that are
   * returned my either be of type TermFreqVector or of type TermPositionsVector if
   * positions or offsets have been stored.
   *
   * @param docNumber document for which term frequency vectors are returned
   * @return array of term frequency vectors. May be null if no term vectors have been
   *  stored for the specified document.
   * @throws IOException if index cannot be accessed
   * @see org.apache.lucene.document.Field.TermVector
   */
   virtual CL_NS(util)::ArrayBase<TermFreqVector*>* getTermFreqVectors(int32_t docNumber) = 0;

  /**
  *  Return a term frequency vector for the specified document and field. The
  *  returned vector contains terms and frequencies for the terms in
  *  the specified field of this document, if the field had the storeTermVector
  *  flag set. If termvectors had been stored with positions or offsets, a
  *  TermPositionsVector is returned.
  *
  * @param docNumber document for which the term frequency vector is returned
  * @param field field for which the term frequency vector is returned.
  * @return term frequency vector May be null if field does not exist in the specified
  * document or term vector was not stored.
  * @throws IOException if index cannot be accessed
  * @see org.apache.lucene.document.Field.TermVector
  */
  virtual TermFreqVector* getTermFreqVector(int32_t docNumber, const TCHAR* field=NULL) = 0;

  /**
   * Load the Term Vector into a user-defined data structure instead of relying on the parallel arrays of
   * the {@link TermFreqVector}.
   * @param docNumber The number of the document to load the vector for
   * @param field The name of the field to load
   * @param mapper The {@link TermVectorMapper} to process the vector.  Must not be null
   * @throws IOException if term vectors cannot be accessed or if they do not exist on the field and doc. specified.
   *
   */
  virtual void getTermFreqVector(int32_t docNumber, const TCHAR* field, TermVectorMapper* mapper) =0;

  /**
   * Map all the term vectors for all fields in a Document
   * @param docNumber The number of the document to load the vector for
   * @param mapper The {@link TermVectorMapper} to process the vector.  Must not be null
   * @throws IOException if term vectors cannot be accessed or if they do not exist on the field and doc. specified.
   */
  virtual void getTermFreqVector(int32_t docNumber, TermVectorMapper* mapper) =0;

	/**
	* Returns <code>true</code> if an index exists at the specified directory.
	* If the directory does not exist or if there is no index in it.
	* @param  directory the directory to check for an index
	* @return <code>true</code> if an index exists; <code>false</code> otherwise
	*/
	static bool indexExists(const char* directory);

    /**
	* Returns <code>true</code> if an index exists at the specified directory.
	* If the directory does not exist or if there is no index in it.
	* @param  directory the directory to check for an index
	* @return <code>true</code> if an index exists; <code>false</code> otherwise
	* @throws IOException if there is a problem with accessing the index
	*/
	static bool indexExists(const CL_NS(store)::Directory* directory);

	/** Returns the number of documents in this index. */
  	virtual int32_t numDocs() = 0;

	/** Returns one greater than the largest possible document number.
	* This may be used to, e.g., determine how big to allocate an array which
	* will have an element for every document number in an index.
	*/
	virtual int32_t maxDoc() const = 0;

  /**
   * Get the {@link org.apache.lucene.document.Document} at the <code>n</code><sup>th</sup> position. The {@link org.apache.lucene.document.FieldSelector}
   * may be used to determine what {@link org.apache.lucene.document.Field}s to load and how they should be loaded.
   * The fields are not cleared before retrieving the document, so the
   * object should be new or just cleared.
   *
   * <b>NOTE:</b> If this Reader (more specifically, the underlying <code>FieldsReader</code>) is closed before the lazy {@link org.apache.lucene.document.Field} is
   * loaded an exception may be thrown.  If you want the value of a lazy {@link org.apache.lucene.document.Field} to be available after closing you must
   * explicitly load it or fetch the Document again with a new loader.
   *
   *
   * @param n Get the document at the <code>n</code><sup>th</sup> position
   * @param fieldSelector The {@link org.apache.lucene.document.FieldSelector} to use to determine what Fields should be loaded on the Document.  May be null, in which case all Fields will be loaded.
   * @return The stored fields of the {@link org.apache.lucene.document.Document} at the nth position
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   *
   * @see org.apache.lucene.document.Field
   * @see org.apache.lucene.document.FieldSelector
   * @see org.apache.lucene.document.SetBasedFieldSelector
   * @see org.apache.lucene.document.LoadFirstFieldSelector
   */
  //When we convert to JDK 1.5 make this Set<String>
	virtual bool document(int32_t n, CL_NS(document)::Document& doc, const CL_NS(document)::FieldSelector* fieldSelector) =0;

  /** Gets the stored fields of the <code>n</code><sup>th</sup>
  * <code>Document</code> in this index.
  * The fields are not cleared before retrieving the document, so the
  * object should be new or just cleared.
  * @throws CorruptIndexException if the index is corrupt
  * @throws IOException if there is a low-level IO error
  */
  bool document(int32_t n, CL_NS(document)::Document& doc);

	_CL_DEPRECATED( document(i, Document&) ) bool document(int32_t n, CL_NS(document)::Document*);

	_CL_DEPRECATED( document(i, document) ) CL_NS(document)::Document* document(const int32_t n);

	/** Returns true if document <i>n</i> has been deleted */
  	virtual bool isDeleted(const int32_t n) = 0;

	/** Returns true if any documents have been deleted */
	virtual bool hasDeletions() const = 0;

	/** Returns true if there are norms stored for this field. */
	virtual bool hasNorms(const TCHAR* field);

/** Returns an enumeration of all the terms in the index. The
  * enumeration is ordered by Term.compareTo(). Each term is greater
  * than all that precede it in the enumeration. Note that after
  * calling terms(), {@link TermEnum#next()} must be called
  * on the resulting enumeration before calling other methods such as
  * {@link TermEnum#term()}.
  * @throws IOException if there is a low-level IO error
	* @memory Caller must clean up
	*/
	virtual TermEnum* terms() = 0;

/** Returns an enumeration of all terms starting at a given term. If
  * the given term does not exist, the enumeration is positioned at the
  * first term greater than the supplied therm. The enumeration is
  * ordered by Term.compareTo(). Each term is greater than all that
  * precede it in the enumeration.
  * @throws IOException if there is a low-level IO error
	* @memory Caller must clean up
	*/
	virtual TermEnum* terms(const Term* t) = 0;

  /** Returns the number of documents containing the term <code>t</code>.
   * @throws IOException if there is a low-level IO error
   */
	virtual int32_t docFreq(const Term* t) = 0;

	/* Returns an unpositioned TermPositions enumerator.
   * @throws IOException if there is a low-level IO error
	 * @memory Caller must clean up
	 */
	virtual TermPositions* termPositions() = 0;

    /** Returns an enumeration of all the documents which contain
	* <code>term</code>.  For each document, in addition to the document number
	* and frequency of the term in that document, a list of all of the ordinal
	* positions of the term in the document is available.  Thus, this method
	* implements the mapping:
	*
	* <p><ul>
	* Term &nbsp;&nbsp; =&gt; &nbsp;&nbsp; &lt;docNum, freq,
	* &lt;pos<sub>1</sub>, pos<sub>2</sub>, ...
	* pos<sub>freq-1</sub>&gt;
	* &gt;<sup>*</sup>
	* </ul>
  * <p> This positional information facilitates phrase and proximity searching.
	* <p>The enumeration is ordered by document number.  Each document number is
	* greater than all that precede it in the enumeration.
  * @throws IOException if there is a low-level IO error
  * @memory Caller must clean up
	*/
	TermPositions* termPositions(Term* term);

	/** Returns an unpositioned {@link TermDocs} enumerator.
   * @throws IOException if there is a low-level IO error
	 * @memory Caller must clean up
	 */
	virtual TermDocs* termDocs() = 0;

	/** Returns an enumeration of all the documents which contain
	* <code>term</code>. For each document, the document number, the frequency of
	* the term in that document is also provided, for use in search scoring.
	* Thus, this method implements the mapping:
	* <p><ul>Term &nbsp;&nbsp; =&gt; &nbsp;&nbsp; &lt;docNum, freq&gt;<sup>*</sup></ul>
	* <p>The enumeration is ordered by document number.  Each document number
	* is greater than all that precede it in the enumeration.
  * @throws IOException if there is a low-level IO error
  * @memory Caller must clean up
	*/
	TermDocs* termDocs(Term* term);

	/** Deletes the document numbered <code>docNum</code>.  Once a document is
	* deleted it will not appear in TermDocs or TermPostitions enumerations.
	* Attempts to read its field with the {@link #document}
	* method will result in an error.  The presence of this document may still be
	* reflected in the {@link #docFreq} statistic, though
	* this will be corrected eventually as the index is further modified.
  * @throws StaleReaderException if the index has changed
  * since this reader was opened
  * @throws CorruptIndexException if the index is corrupt
  * @throws LockObtainFailedException if another writer
  *  has this index open (<code>write.lock</code> could not
  *  be obtained)
  * @throws IOException if there is a low-level IO error
	*/
	void deleteDocument(const int32_t docNum);

	///@deprecated. Use deleteDocument instead.
	_CL_DEPRECATED( deleteDocument ) void deleteDoc(const int32_t docNum);

	/** Deletes all documents that have a given <code>term</code>.
	* This is useful if one uses a document field to hold a unique ID string for
	* the document.  Then to delete such a document, one merely constructs a
	* term with the appropriate field and the unique ID string as its text and
	* passes it to this method.
	* See {@link #deleteDocument(int)} for information about when this deletion will
	* become effective.
	* @return the number of documents deleted
  * @throws StaleReaderException if the index has changed
  *  since this reader was opened
  * @throws CorruptIndexException if the index is corrupt
  * @throws LockObtainFailedException if another writer
  *  has this index open (<code>write.lock</code> could not
  *  be obtained)
  * @throws IOException if there is a low-level IO error
	*/
	int32_t deleteDocuments(Term* term);

	///@deprecated. Use deleteDocuments instead.
	_CL_DEPRECATED( deleteDocuments ) int32_t deleteTerm(Term* term);

	/**
	* Closes files associated with this index and also saves any new deletions to disk.
  * No other methods should be called after this has been called.
  * @throws IOException if there is a low-level IO error
  */
	void close();

  /**
   * Returns <code>true</code> iff the index in the named directory is
   * currently locked.
   * @param directory the directory to check for a lock
   * @throws IOException if there is a low-level IO error
   */
	static bool isLocked(CL_NS(store)::Directory* directory);

  /**
   * Returns <code>true</code> iff the index in the named directory is
   * currently locked.
   * @param directory the directory to check for a lock
   * @throws IOException if there is a low-level IO error
   */
	static bool isLocked(const char* directory);


	///Forcibly unlocks the index in the named directory.
	///Caution: this should only be used by failure recovery code,
	///when it is known that no other process nor thread is in fact
	///currently accessing this index.
	static void unlock(CL_NS(store)::Directory* directory);
	static void unlock(const char* path);

	 /** Returns the directory this index resides in. */
	_CL_DEPRECATED( directory() ) CL_NS(store)::Directory* getDirectory();

	/** Returns true if the file is a lucene filename (based on extension or filename) */
	static bool isLuceneFile(const char* filename);

	/**
	* For classes that need to know when the IndexReader closes (such as caches, etc),
	* should pass their callback function to this.
	*/
	void addCloseCallback(CloseCallback callback, void* parameter);

  friend class SegmentReader;
  friend class MultiReader;
  friend class IndexWriter;
  friend class MultiSegmentReader;
};

CL_NS_END
#endif


