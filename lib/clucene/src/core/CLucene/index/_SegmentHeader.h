/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SegmentHeader_
#define _lucene_index_SegmentHeader_

#include "_SegmentInfos.h"
#include "CLucene/util/BitSet.h"
//#include "CLucene/util/VoidMap.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/store/IndexOutput.h"
#include "CLucene/index/IndexReader.h"
#include "Term.h"
#include "Terms.h"
#include "_TermInfo.h"
//#include "FieldInfos.h"
#include "_FieldsReader.h"
#include "_TermVector.h"
//#include "IndexReader.h"
#include "_TermInfosReader.h"
#include "_CompoundFile.h"
#include "DirectoryIndexReader.h"
#include "_SkipListReader.h"
#include "CLucene/util/_ThreadLocal.h"

CL_NS_DEF(index)
class SegmentReader;

class SegmentTermDocs:public virtual TermDocs {
protected:
  const SegmentReader* parent;
  CL_NS(store)::IndexInput* freqStream;
  int32_t count;
  int32_t df;
  CL_NS(util)::BitSet* deletedDocs;
  int32_t _doc;
  int32_t _freq;

private:
  int32_t skipInterval;
  int32_t maxSkipLevels;
  DefaultSkipListReader* skipListReader;

  int64_t freqBasePointer;
  int64_t proxBasePointer;

  int64_t skipPointer;
  bool haveSkipped;

protected:
  bool currentFieldStoresPayloads;

public:
  ///\param Parent must be a segment reader
  SegmentTermDocs( const SegmentReader* Parent);
  virtual ~SegmentTermDocs();

  virtual void seek(Term* term);
  virtual void seek(TermEnum* termEnum);
  virtual void seek(const TermInfo* ti,Term* term);

  virtual void close();
  virtual int32_t doc()const;
  virtual int32_t freq()const;

  virtual bool next();

  /** Optimized implementation. */
  virtual int32_t read(int32_t* docs, int32_t* freqs, int32_t length);

  /** Optimized implementation. */
  virtual bool skipTo(const int32_t target);

  virtual TermPositions* __asTermPositions();

protected:
  virtual void skippingDoc(){}
  virtual void skipProx(const int64_t /*proxPointer*/, const int32_t /*payloadLength*/){}
};


class SegmentTermPositions: public SegmentTermDocs, public TermPositions {
private:
  CL_NS(store)::IndexInput* proxStream;
  int32_t proxCount;
  int32_t position;

  // the current payload length
  int32_t payloadLength;
  // indicates whether the payload of the currend position has
  // been read from the proxStream yet
  bool needToLoadPayload;

  // these variables are being used to remember information
  // for a lazy skip
  int64_t lazySkipPointer;
  int32_t lazySkipProxCount;

public:
  ///\param Parent must be a segment reader
  SegmentTermPositions(const SegmentReader* Parent);
  virtual ~SegmentTermPositions();

private:
  void seek(const TermInfo* ti, Term* term);

public:
  void close();

  int32_t nextPosition();
private:
  int32_t readDeltaPosition();

protected:
  void skippingDoc();

public:
  bool next();
  int32_t read(int32_t* docs, int32_t* freqs, int32_t length);

protected:
  /** Called by super.skipTo(). */
  void skipProx(const int64_t proxPointer, const int32_t _payloadLength);

private:
  void skipPositions( int32_t n );
  void skipPayload();

  // It is not always neccessary to move the prox pointer
  // to a new document after the freq pointer has been moved.
  // Consider for example a phrase query with two terms:
  // the freq pointer for term 1 has to move to document x
  // to answer the question if the term occurs in that document. But
  // only if term 2 also matches document x, the positions have to be
  // read to figure out if term 1 and term 2 appear next
  // to each other in document x and thus satisfy the query.
  // So we move the prox pointer lazily to the document
  // as soon as positions are requested.
  void lazySkip();

public:
  int32_t getPayloadLength() const;

  uint8_t* getPayload(uint8_t* data);

  bool isPayloadAvailable() const;

private:
  virtual TermDocs* __asTermDocs();
  virtual TermPositions* __asTermPositions();

  //resolve SegmentTermDocs/TermPositions ambiguity
  void seek(Term* term){ SegmentTermDocs::seek(term); }
  void seek(TermEnum* termEnum){ SegmentTermDocs::seek(termEnum); }
  int32_t doc() const{ return SegmentTermDocs::doc(); }
  int32_t freq() const{ return SegmentTermDocs::freq(); }
  bool skipTo(const int32_t target){ return SegmentTermDocs::skipTo(target); }
};




/**
 * An IndexReader responsible for reading 1 segment of an index
 */
class SegmentReader: public DirectoryIndexReader {
  /**
   * The class Norm represents the normalizations for a field.
   * These normalizations are read from an IndexInput in into an array of bytes called bytes
   */
  class Norm :LUCENE_BASE{
    int32_t number;
    int64_t normSeek;
    SegmentReader* _this;
    const char* segment; ///< pointer to segment name
    volatile int32_t refCount;
    bool useSingleNormStream;
    bool rollbackDirty;


    /** Closes the underlying IndexInput for this norm.
     * It is still valid to access all other norm properties after close is called.
     * @throws IOException
     */
    void close();
  public:
    DEFINE_MUTEX(THIS_LOCK)

    CL_NS(store)::IndexInput* in;
    uint8_t* bytes;
    bool dirty;
    //Constructor
    Norm(CL_NS(store)::IndexInput* instrm, bool useSingleNormStream, int32_t number, int64_t normSeek, SegmentReader* reader, const char* segment);
    //Destructor
    ~Norm();

    void reWrite(SegmentInfo* si);

    void incRef();
    void decRef();
    friend class SegmentReader;

    static void doDelete(Norm* norm);
  };
  friend class SegmentReader::Norm;

  //Holds the name of the segment that is being read
  std::string segment;
  SegmentInfo* si;
  int32_t readBufferSize;

  //Indicates if there are documents marked as deleted
  bool deletedDocsDirty;
  bool normsDirty;
  bool undeleteAll;

  bool rollbackDeletedDocsDirty;
  bool rollbackNormsDirty;
  bool rollbackUndeleteAll;


  //Holds all norms for all fields in the segment
  typedef CL_NS(util)::CLHashtable<const TCHAR*,Norm*,
    CL_NS(util)::Compare::TChar, CL_NS(util)::Equals::TChar,
    CL_NS(util)::Deletor::Dummy,
    Norm > NormsType;
  NormsType _norms;

  uint8_t* ones;
  uint8_t* fakeNorms();

  // optionally used for the .nrm file shared by multiple norms
  CL_NS(store)::IndexInput* singleNormStream;

  // Compound File Reader when based on a compound file segment
  CompoundFileReader* cfsReader;
  CompoundFileReader* storeCFSReader;

  ///Reads the Field Info file
  FieldsReader* fieldsReader;
  TermVectorsReader* termVectorsReaderOrig;
  CL_NS(util)::ThreadLocal<TermVectorsReader*,
  CL_NS(util)::Deletor::Object<TermVectorsReader> >termVectorsLocal;

  void initialize(SegmentInfo* si, int32_t readBufferSize, bool doOpenStores, bool doingReopen);

  /**
   * Create a clone from the initial TermVectorsReader and store it in the ThreadLocal.
   * @return TermVectorsReader
   */
  TermVectorsReader* getTermVectorsReader();

  FieldsReader* getFieldsReader();
  FieldInfos* getFieldInfos();

protected:
  ///Marks document docNum as deleted
  void doDelete(const int32_t docNum);
  void doUndeleteAll();
  void commitChanges();
  void doSetNorm(int32_t doc, const TCHAR* field, uint8_t value);

  // can return null if norms aren't stored
  uint8_t* getNorms(const TCHAR* field);

  /**
   * Decrements the RC of the norms this reader is using
   */
  void decRefNorms();


  DirectoryIndexReader* doReopen(SegmentInfos* infos);

public:
  /**
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  static SegmentReader* get(SegmentInfo* si, bool doOpenStores=true);

  /**
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  static SegmentReader* get(SegmentInfo* si, int32_t readBufferSize, bool doOpenStores=true);

  /**
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  static SegmentReader* get(SegmentInfos* sis, SegmentInfo* si, bool closeDir);

  /**
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   * @param readBufferSize defaults to BufferedIndexInput::BUFFER_SIZE
   */
  static SegmentReader* get(CL_NS(store)::Directory* dir, SegmentInfo* si,
      SegmentInfos* sis,
      bool closeDir, bool ownDir,
      int32_t readBufferSize=-1,
      bool doOpenStores=true);



  SegmentReader();
  ///Destructor.
  virtual ~SegmentReader();

  ///Closes all streams to the files of a single segment
  void doClose();

  ///Checks if a segment managed by SegmentInfo si has deletions
  static bool hasDeletions(const SegmentInfo* si);
  bool hasDeletions() const;
  bool hasNorms(const TCHAR* field);

  ///Returns all file names managed by this SegmentReader
  void files(std::vector<std::string>& retarray);
  ///Returns an enumeration of all the Terms and TermInfos in the set.
  TermEnum* terms();
  ///Returns an enumeration of terms starting at or after the named term t
  TermEnum* terms(const Term* t);

  ///Gets the document identified by n
  bool document(int32_t n, CL_NS(document)::Document& doc, const CL_NS(document)::FieldSelector* fieldSelector);

  ///Checks if the n-th document has been marked deleted
  bool isDeleted(const int32_t n);

  ///Returns an unpositioned TermDocs enumerator.
  TermDocs* termDocs();
  ///Returns an unpositioned TermPositions enumerator.
  TermPositions* termPositions();

  ///Returns the number of documents which contain the term t
  int32_t docFreq(const Term* t);

  ///Returns the actual number of documents in the segment
  int32_t numDocs();
  ///Returns the number of  all the documents in the segment including the ones that have
  ///been marked deleted
  int32_t maxDoc() const;


  void setTermInfosIndexDivisor(int32_t indexDivisor);

  int32_t getTermInfosIndexDivisor();

  ///Returns the bytes array that holds the norms of a named field.
  ///Returns fake norms if norms aren't available
  uint8_t* norms(const TCHAR* field);

  ///Reads the Norms for field from disk
  void norms(const TCHAR* field, uint8_t* bytes);

  ///concatenating segment with ext and x
  std::string SegmentName(const char* ext, const int32_t x=-1);
  ///Creates a filename in buffer by concatenating segment with ext and x
  void SegmentName(char* buffer,int32_t bufferLen,const char* ext, const int32_t x=-1 );

  /**
   * @see IndexReader#getFieldNames(IndexReader.FieldOption fldOption)
   */
  void getFieldNames(FieldOption fldOption, StringArrayWithDeletor& retarray);

  static bool usesCompoundFile(SegmentInfo* si);

  /** Return a term frequency vector for the specified document and field. The
   *  vector returned contains term numbers and frequencies for all terms in
   *  the specified field of this document, if the field had storeTermVector
   *  flag set.  If the flag was not set, the method returns null.
   * @throws IOException
   */
  TermFreqVector* getTermFreqVector(int32_t docNumber, const TCHAR* field=NULL);

  void getTermFreqVector(int32_t docNumber, const TCHAR* field, TermVectorMapper* mapper);
  void getTermFreqVector(int32_t docNumber, TermVectorMapper* mapper);

  /** Return an array of term frequency vectors for the specified document.
   *  The array contains a vector for each vectorized field in the document.
   *  Each vector vector contains term numbers and frequencies for all terms
   *  in a given vectorized field.
   *  If no such fields existed, the method returns null.
   * @throws IOException
   */
  CL_NS(util)::ArrayBase<TermFreqVector*>* getTermFreqVectors(int32_t docNumber);

  static const char* getClassName();
  const char* getObjectName() const;

  // for testing only
  bool normsClosed();

private:
  //Open all norms files for all fields
  void openNorms(CL_NS(store)::Directory* cfsDir, int32_t readBufferSize);

  ///a bitVector that manages which documents have been deleted
  CL_NS(util)::BitSet* deletedDocs;
  ///an IndexInput to the frequency file
  CL_NS(store)::IndexInput* freqStream;
  ///For reading the fieldInfos file
  FieldInfos* _fieldInfos;
  ///For reading the Term Dictionary .tis file
  TermInfosReader* tis;
  ///an IndexInput to the prox file
  CL_NS(store)::IndexInput* proxStream;

  static bool hasSeparateNorms(SegmentInfo* si);
  static uint8_t* createFakeNorms(int32_t size);

  void loadDeletedDocs();
  SegmentReader* reopenSegment(SegmentInfo* si);

  /** Returns the field infos of this segment */
  FieldInfos* fieldInfos();

  /**
   * Return the name of the segment this reader is reading.
   */
  const char* getSegmentName();

  /**
   * Return the SegmentInfo of the segment this reader is reading.
   */
  SegmentInfo* getSegmentInfo();
  void setSegmentInfo(SegmentInfo* info);
  void startCommit();
  void rollbackCommit();

  //allow various classes to access the internals of this. this allows us to have
  //a more tight idea of the package
  friend class IndexReader;
  friend class IndexWriter;
  friend class SegmentTermDocs;
  friend class SegmentTermPositions;
  friend class MultiReader;
  friend class MultiSegmentReader;
  friend class SegmentMerger;
};

CL_NS_END
#endif
