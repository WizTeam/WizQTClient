/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_MultiSegmentReader
#define _lucene_index_MultiSegmentReader


#include "DirectoryIndexReader.h"
#include "IndexReader.h"
CL_CLASS_DEF(document,Document)
//#include "Terms.h"
#include "_SegmentHeader.h"

CL_NS_DEF(index)
class SegmentMergeQueue;

class MultiSegmentReader:public DirectoryIndexReader{
  static int32_t readerIndex(const int32_t n, int32_t* starts, int32_t numSubReaders);
public:
  typedef CL_NS(util)::CLHashtable<TCHAR*,uint8_t*,
    CL_NS(util)::Compare::TChar,
    CL_NS(util)::Equals::TChar,
    CL_NS(util)::Deletor::tcArray,
    CL_NS(util)::Deletor::vArray<uint8_t> > NormsCacheType;
private:
	int32_t readerIndex(int32_t n) const;
	bool hasNorms(const TCHAR* field);
	uint8_t* fakeNorms();

  void startCommit();
  void rollbackCommit();

  bool _hasDeletions;
  uint8_t* ones;
  NormsCacheType normsCache;
  int32_t _maxDoc;
  int32_t _numDocs;

protected:
	CL_NS(util)::ArrayBase<IndexReader*>* subReaders;
	int32_t* starts;			  // 1st docno for each segment

	void doSetNorm(int32_t n, const TCHAR* field, uint8_t value);
	void doUndeleteAll();
	void commitChanges();
	// synchronized
	void doClose();

	// synchronized
	void doDelete(const int32_t n);
  DirectoryIndexReader* doReopen(SegmentInfos* infos);

  void initialize( CL_NS(util)::ArrayBase<IndexReader*>* subReaders);

public:

  /** Construct reading the named set of readers. */
  MultiSegmentReader(CL_NS(store)::Directory* directory, SegmentInfos* sis, bool closeDirectory);

  /** This contructor is only used for {@link #reopen()} */
  CLUCENE_LOCAL_DECL MultiSegmentReader(
      CL_NS(store)::Directory* directory,
      SegmentInfos* sis,
      bool closeDirectory,
      CL_NS(util)::ArrayBase<IndexReader*>* oldReaders,
      int32_t* oldStarts,
      NormsCacheType* oldNormsCache);

	virtual ~MultiSegmentReader();

	/** Return an array of term frequency vectors for the specified document.
	*  The array contains a vector for each vectorized field in the document.
	*  Each vector vector contains term numbers and frequencies for all terms
	*  in a given vectorized field.
	*  If no such fields existed, the method returns null.
	*/
  TermFreqVector* getTermFreqVector(int32_t docNumber, const TCHAR* field=NULL);
  CL_NS(util)::ArrayBase<TermFreqVector*>* getTermFreqVectors(int32_t docNumber);
  void getTermFreqVector(int32_t docNumber, const TCHAR* field, TermVectorMapper* mapper);
  void getTermFreqVector(int32_t docNumber, TermVectorMapper* mapper);
  bool isOptimized();

	// synchronized
	int32_t numDocs();

	int32_t maxDoc() const;

  bool document(int32_t n, CL_NS(document)::Document& doc, const CL_NS(document)::FieldSelector* fieldSelector);

	bool isDeleted(const int32_t n);
	bool hasDeletions() const;

	// synchronized
	uint8_t* norms(const TCHAR* field);
	void norms(const TCHAR* field, uint8_t* result);

	TermEnum* terms();
	TermEnum* terms(const Term* term);

	//Returns the document frequency of the current term in the set
	int32_t docFreq(const Term* t=NULL);
	TermDocs* termDocs();
	TermPositions* termPositions();

  void getFieldNames (FieldOption fldOption, StringArrayWithDeletor& retarray);
	static void getFieldNames(FieldOption fldOption, StringArrayWithDeletor& retarray, CL_NS(util)::ArrayBase<IndexReader*>* subReaders);

  void setTermInfosIndexDivisor(int32_t indexDivisor);
  int32_t getTermInfosIndexDivisor();

  const CL_NS(util)::ArrayBase<IndexReader*>* getSubReaders() const;

  friend class MultiReader;
  friend class SegmentReader;
  friend class DirectoryIndexReader;

  static const char* getClassName();
  const char* getObjectName() const;
};


class MultiTermDocs:public virtual TermDocs {
protected:
  CL_NS(util)::ArrayBase<TermDocs*>* readerTermDocs;

  CL_NS(util)::ArrayBase<IndexReader*>* subReaders;
  const int32_t* starts;
  Term* term;

  int32_t base;
  size_t pointer;

  TermDocs* current;              // == segTermDocs[pointer]
  TermDocs* termDocs(const int32_t i); //< internal use only
  virtual TermDocs* termDocs(IndexReader* reader);
  void init(CL_NS(util)::ArrayBase<IndexReader*>* subReaders, const int32_t* starts);
public:
  MultiTermDocs();
  MultiTermDocs(CL_NS(util)::ArrayBase<IndexReader*>* subReaders, const int32_t* s);
  virtual ~MultiTermDocs();

  int32_t doc() const;
  int32_t freq() const;

  void seek(TermEnum* termEnum);
  void seek(Term* tterm);
  bool next();

  /** Optimized implementation. */
  int32_t read(int32_t* docs, int32_t* freqs, int32_t length);

   /* A Possible future optimization could skip entire segments */
  bool skipTo(const int32_t target);

  void close();

  virtual TermPositions* __asTermPositions();
};


//MultiTermEnum represents the enumeration of all terms of all readers
class MultiTermEnum:public TermEnum {
private:
  SegmentMergeQueue* queue;

  Term* _term;
  int32_t _docFreq;
public:
  //Constructor
  //Opens all enumerations of all readers
  MultiTermEnum(CL_NS(util)::ArrayBase<IndexReader*>* subReaders, const int32_t* starts, const Term* t);

  //Destructor
  ~MultiTermEnum();

  //Move the current term to the next in the set of enumerations
  bool next();

  //Returns a pointer to the current term of the set of enumerations
  Term* term(bool pointer=true);

  //Returns the document frequency of the current term in the set
  int32_t docFreq() const;

  //Closes the set of enumerations in the queue
  void close();


  const char* getObjectName() const;
  static const char* getClassName();
};


#ifdef _MSC_VER
    #pragma warning(disable : 4250)
#endif
class MultiTermPositions:public MultiTermDocs,public TermPositions {
protected:
  TermDocs* termDocs(IndexReader* reader);
public:
  MultiTermPositions(CL_NS(util)::ArrayBase<IndexReader*>* subReaders, const int32_t* s);
  virtual ~MultiTermPositions() {};
  int32_t nextPosition();

  /**
  * Not implemented.
  * @throws UnsupportedOperationException
  */
  int32_t getPayloadLength() const;

  /**
  * Not implemented.
  * @throws UnsupportedOperationException
  */
  uint8_t* getPayload(uint8_t* data);

  /**
  *
  * @return false
  */
  // TODO: Remove warning after API has been finalized
  bool isPayloadAvailable() const;

  virtual TermDocs* __asTermDocs();
  virtual TermPositions* __asTermPositions();
};

CL_NS_END
#endif
