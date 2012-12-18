/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_MultiReader
#define _lucene_index_MultiReader


//#include "SegmentHeader.h"
#include "IndexReader.h"
CL_CLASS_DEF(document,Document)
//#include "Terms.h"
//#include "SegmentMergeQueue.h"

CL_NS_DEF(index)

/** An IndexReader which reads multiple indexes, appending their content.
*/
class CLUCENE_EXPORT MultiReader:public IndexReader{
private:
  class Internal;
  Internal* _internal;
	int32_t readerIndex(const int32_t n) const;
	bool hasNorms(const TCHAR* field);
	uint8_t* fakeNorms();

  void init(const CL_NS(util)::ArrayBase<IndexReader*>* subReaders, bool closeSubReaders);
protected:
	CL_NS(util)::ArrayBase<IndexReader*>* subReaders;
	int32_t* starts;			  // 1st docno for each segment

	void doSetNorm(int32_t n, const TCHAR* field, uint8_t value);
	void doUndeleteAll();
	void doCommit();
	void doClose();
	void doDelete(const int32_t n);
public:
	/**
	* <p>Construct a MultiReader aggregating the named set of (sub)readers.
	* Directory locking for delete, undeleteAll, and setNorm operations is
	* left to the subreaders. </p>
	* @param subReaders set of (sub)readers
  * @param closeSubReaders The subReaders (IndexReader instances) are deleted if true
	* @throws IOException
  * @memory The subReaders array itself belongs to the caller
	*/
	MultiReader(const CL_NS(util)::ArrayBase<IndexReader*>* subReaders, bool closeSubReaders=true);

	~MultiReader();


  /**
   * Tries to reopen the subreaders.
   * <br>
   * If one or more subreaders could be re-opened (i. e. subReader.reopen()
   * returned a new instance != subReader), then a new MultiReader instance
   * is returned, otherwise this instance is returned.
   * <p>
   * A re-opened instance might share one or more subreaders with the old
   * instance. Index modification operations result in undefined behavior
   * when performed before the old instance is closed.
   * (see {@link IndexReader#reopen()}).
   * <p>
   * If subreaders are shared, then the reference count of those
   * readers is increased to ensure that the subreaders remain open
   * until the last referring reader is closed.
   *
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  IndexReader* reopen();

	TermFreqVector* getTermFreqVector(int32_t n, const TCHAR* field=NULL);

  void getTermFreqVector(int32_t docNumber, const TCHAR* field, TermVectorMapper* mapper);
  void getTermFreqVector(int32_t docNumber, TermVectorMapper* mapper);

  /** Return an array of term frequency vectors for the specified document.
  *  The array contains a vector for each vectorized field in the document.
  *  Each vector vector contains term numbers and frequencies for all terms
  *  in a given vectorized field.
  *  If no such fields existed, the method returns null.
  */
  CL_NS(util)::ArrayBase<TermFreqVector*>* getTermFreqVectors(int32_t n);

  bool isOptimized();

	int32_t numDocs();
	int32_t maxDoc() const;
  bool document(int32_t n, CL_NS(document)::Document& doc, const CL_NS(document)::FieldSelector* fieldSelector);
	bool isDeleted(const int32_t n);
	bool hasDeletions() const;
	uint8_t* norms(const TCHAR* field);
	void norms(const TCHAR* field, uint8_t* result);
	TermEnum* terms();
	TermEnum* terms(const Term* term);

	//Returns the document frequency of the current term in the set
	int32_t docFreq(const Term* t=NULL);
	TermDocs* termDocs();
	TermPositions* termPositions();

	/**
	* @see IndexReader#getFieldNames(IndexReader.FieldOption fldOption)
	*/
	void getFieldNames(FieldOption fldOption, StringArrayWithDeletor& retarray);


  /**
   * Checks recursively if all subreaders are up to date.
   */
  bool isCurrent();

  /** Not implemented.
   * @throws UnsupportedOperationException
   */
  int64_t getVersion();

  const CL_NS(util)::ArrayBase<IndexReader*>* getSubReaders() const;

  static const char* getClassName();
  const char* getObjectName() const;
};


CL_NS_END
#endif
