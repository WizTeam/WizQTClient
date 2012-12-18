/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/util/Misc.h"
#include "_SegmentHeader.h"
#include "_MultiSegmentReader.h"
#include "_FieldInfos.h"
#include "_FieldsReader.h"
#include "IndexReader.h"
#include "_TermInfosReader.h"
#include "Terms.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/util/PriorityQueue.h"
#include "_SegmentMerger.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(document)
CL_NS_USE(search)
CL_NS_DEF(index)

 SegmentReader::Norm::Norm(IndexInput* instrm, bool _useSingleNormStream, int32_t n, int64_t ns, SegmentReader* r, const char* seg):
	number(n),
	normSeek(ns),
	_this(r),
	segment(seg),
    useSingleNormStream(_useSingleNormStream),
	in(instrm),
	bytes(NULL),
	dirty(false){
  //Func - Constructor
  //Pre  - instrm is a valid reference to an IndexInput
  //Post - A Norm instance has been created with an empty bytes array

    refCount = 1;
	  bytes = NULL;
    dirty = false;
  }

  SegmentReader::Norm::~Norm() {
  //Func - Destructor
  //Pre  - true
  //Post - The IndexInput in has been deleted (and closed by its destructor)
  //       and the array too.

      //Close and destroy the inputstream in-> The inputstream will be closed
      // by its destructor. Note that the IndexInput 'in' actually is a pointer!!!!!
      if ( in != _this->singleNormStream )
    	  _CLDELETE(in);

	  //Delete the bytes array
      _CLDELETE_ARRAY(bytes);

  }
  void SegmentReader::Norm::doDelete(Norm* norm){
    if ( norm->refCount == 0 ){
      _CLLDELETE(norm);
    }
  }

  void SegmentReader::Norm::close(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    if (in != NULL && !useSingleNormStream) {
      in->close();
      _CLDELETE(in);
    }
    in = NULL;
  }

  void SegmentReader::Norm::incRef() {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    assert (refCount > 0);
    refCount++;
  }

  void SegmentReader::Norm::decRef(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    assert (refCount > 0);
    if (refCount == 1) {
      close();
    }
    refCount--;
  }
  void SegmentReader::Norm::reWrite(SegmentInfo* si){
      // NOTE: norms are re-written in regular directory, not cfs
      si->advanceNormGen(this->number);
      IndexOutput* out = _this->directory()->createOutput(si->getNormFileName(this->number).c_str());
      try {
        out->writeBytes(bytes, _this->maxDoc());
      }_CLFINALLY(
        out->close();
        _CLDELETE(out)
      );
      this->dirty = false;
    }

  void SegmentReader::initialize(SegmentInfo* si, int32_t readBufferSize, bool doOpenStores, bool doingReopen){
    //Pre  - si-> is a valid reference to SegmentInfo instance
    //       identified by si->
    //Post - All files of the segment have been read

    this->deletedDocs      = NULL;
    this->ones			   = NULL;
    //There are no documents yet marked as deleted
    this->deletedDocsDirty = false;

    this->normsDirty=false;
    this->undeleteAll=false;

    this->rollbackDeletedDocsDirty = false;
    this->rollbackNormsDirty = false;
    this->rollbackUndeleteAll = false;

    //Duplicate the name of the segment from SegmentInfo to segment
    this->segment          = si->name;

    // make sure that all index files have been read or are kept open
    // so that if an index update removes them we'll still have them
    this->freqStream       = NULL;
    this->proxStream       = NULL;
    this->singleNormStream = NULL;
    this->termVectorsReaderOrig = NULL;
    this->_fieldInfos = NULL;
    this->tis = NULL;
    this->fieldsReader = NULL;
    this->cfsReader = NULL;
    this->storeCFSReader = NULL;

    this->segment = si->name;
    this->si = si;
    this->readBufferSize = readBufferSize;

    if ( doingReopen ) return; // the rest is done in the reopen code...

    bool success = false;

    try {
      // Use compound file directory for some files, if it exists
      Directory* cfsDir = directory();
      if (si->getUseCompoundFile()) {
        cfsReader = _CLNEW CompoundFileReader(directory(), (segment + "." + IndexFileNames::COMPOUND_FILE_EXTENSION).c_str(), readBufferSize);
        cfsDir = cfsReader;
      }

      Directory* storeDir;

      if (doOpenStores) {
        if (si->getDocStoreOffset() != -1) {
          if (si->getDocStoreIsCompoundFile()) {
            storeCFSReader = _CLNEW CompoundFileReader(directory(), (si->getDocStoreSegment() + "." + IndexFileNames::COMPOUND_FILE_STORE_EXTENSION).c_str(), readBufferSize);
            storeDir = storeCFSReader;
          } else {
            storeDir = directory();
          }
        } else {
          storeDir = cfsDir;
        }
      } else
        storeDir = NULL;

      // No compound file exists - use the multi-file format
      _fieldInfos = _CLNEW FieldInfos(cfsDir, (segment + ".fnm").c_str() );

      string fieldsSegment;

      if (si->getDocStoreOffset() != -1)
        fieldsSegment = si->getDocStoreSegment();
      else
        fieldsSegment = segment;

      if (doOpenStores) {
        fieldsReader = _CLNEW FieldsReader(storeDir, fieldsSegment.c_str(), _fieldInfos, readBufferSize,
                                        si->getDocStoreOffset(), si->docCount);

        // Verify two sources of "maxDoc" agree:
        if (si->getDocStoreOffset() == -1 && fieldsReader->size() != si->docCount) {
          string err = "doc counts differ for segment ";
          err += si->name;
          err += ": fieldsReader shows ";
          err += fieldsReader->size();
          err += " but segmentInfo shows ";
          err += si->docCount;
          _CLTHROWA(CL_ERR_CorruptIndex, err.c_str() );
        }
      }

      tis = _CLNEW TermInfosReader(cfsDir, segment.c_str(), _fieldInfos, readBufferSize);

      loadDeletedDocs();

      // make sure that all index files have been read or are kept open
      // so that if an index update removes them we'll still have them
      freqStream = cfsDir->openInput( (segment + ".frq").c_str(), readBufferSize);
      proxStream = cfsDir->openInput( (segment + ".prx").c_str(), readBufferSize);
      openNorms(cfsDir, readBufferSize);

      if (doOpenStores && _fieldInfos->hasVectors()) { // open term vector files only as needed
        string vectorsSegment;
        if (si->getDocStoreOffset() != -1)
          vectorsSegment = si->getDocStoreSegment();
        else
          vectorsSegment = segment;
        termVectorsReaderOrig = _CLNEW TermVectorsReader(storeDir, vectorsSegment.c_str(), _fieldInfos, readBufferSize, si->getDocStoreOffset(), si->docCount);
      }
      success = true;
    } _CLFINALLY (

      // With lock-less commits, it's entirely possible (and
      // fine) to hit a FileNotFound exception above.  In
      // this case, we want to explicitly close any subset
      // of things that were opened so that we don't have to
      // wait for a GC to do so.
      if (!success) {
        doClose();
      }
    )
  }

  SegmentReader* SegmentReader::get(SegmentInfo* si, bool doOpenStores) {
    return get(si->dir, si, NULL, false, false, BufferedIndexInput::BUFFER_SIZE, doOpenStores);
  }

  SegmentReader* SegmentReader::get(SegmentInfo* si, int32_t readBufferSize, bool doOpenStores){
    return get(si->dir, si, NULL, false, false, readBufferSize, doOpenStores);
  }
  SegmentReader* SegmentReader::get(SegmentInfos* sis, SegmentInfo* si,
                                  bool closeDir) {
    return get(si->dir, si, sis, closeDir, true, BufferedIndexInput::BUFFER_SIZE, true);
  }
  /**
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  SegmentReader* SegmentReader::get(Directory* dir, SegmentInfo* si,
                                  SegmentInfos* sis,
                                  bool closeDir, bool ownDir,
                                  int32_t readBufferSize,
                                  bool doOpenStores){
    SegmentReader* instance = _CLNEW SegmentReader(); //todo: make this configurable...
    instance->init(dir, sis, closeDir);
    instance->initialize(si, readBufferSize==-1 ? BufferedIndexInput::BUFFER_SIZE : readBufferSize, doOpenStores, false);
    return instance;
  }

  SegmentReader::SegmentReader():
    DirectoryIndexReader(),
    _norms(false,true)
  {
  }
  SegmentReader::~SegmentReader(){
  //Func - Destructor.
  //Pre  - doClose has been invoked!
  //Post - the instance has been destroyed

      doClose(); //this means that index reader doesn't need to be closed manually

      _CLDELETE(_fieldInfos);
      _CLDELETE(fieldsReader);
      _CLDELETE(tis);
      _CLDELETE(freqStream);
      _CLDELETE(proxStream);
      _CLDELETE(deletedDocs);
      _CLDELETE_ARRAY(ones);
      _CLDELETE(termVectorsReaderOrig)
      _CLDECDELETE(cfsReader);
      //termVectorsLocal->unregister(this);
  }

  void SegmentReader::commitChanges(){
    if (deletedDocsDirty) {               // re-write deleted
      si->advanceDelGen();

      // We can write directly to the actual name (vs to a
      // .tmp & renaming it) because the file is not live
      // until segments file is written:
      deletedDocs->write(directory(), si->getDelFileName().c_str());
    }
    if (undeleteAll && si->hasDeletions()) {
      si->clearDelGen();
    }
    if (normsDirty) {               // re-write norms
      si->setNumFields(_fieldInfos->size());
      NormsType::iterator it = _norms.begin();
      while (it != _norms.end()) {
        Norm* norm = it->second;
        if (norm->dirty) {
          norm->reWrite(si);
        }
        it++;
      }
    }
    deletedDocsDirty = false;
    normsDirty = false;
    undeleteAll = false;
  }

  void SegmentReader::doClose() {
  //Func - Closes all streams to the files of a single segment
  //Pre  - fieldsReader != NULL
  //       tis != NULL
  //Post - All streams to files have been closed

      _CLDELETE(deletedDocs);

      // close the single norms stream
      if (singleNormStream != NULL) {
        // we can close this stream, even if the norms
        // are shared, because every reader has it's own
        // singleNormStream
        singleNormStream->close();
        _CLDELETE(singleNormStream);
      }

      // re-opened SegmentReaders have their own instance of FieldsReader
      if (fieldsReader != NULL) {
        fieldsReader->close();
        _CLDELETE(fieldsReader);
      }

      if (tis != NULL) {
        tis->close();
        _CLDELETE(tis);
      }

      //Close the frequency stream
      if (freqStream != NULL){
        freqStream->close();
        _CLDELETE(freqStream);
      }
      //Close the prox stream
      if (proxStream != NULL){
          proxStream->close();
          _CLDELETE(proxStream);
      }

      if (termVectorsReaderOrig != NULL){
          termVectorsReaderOrig->close();
          _CLDELETE(termVectorsReaderOrig);
      }

      if (cfsReader != NULL){
        cfsReader->close();
        _CLDECDELETE(cfsReader);
      }

      if (storeCFSReader != NULL){
        storeCFSReader->close();
        _CLDELETE(storeCFSReader);
      }

      this->decRefNorms();
      _norms.clear();

      // maybe close directory
      DirectoryIndexReader::doClose();
  }

  bool SegmentReader::hasDeletions()  const{
    // Don't call ensureOpen() here (it could affect performance)
      return deletedDocs != NULL;
  }

  //static
  bool SegmentReader::usesCompoundFile(SegmentInfo* si) {
    return si->getUseCompoundFile();
  }

  //static
  bool SegmentReader::hasSeparateNorms(SegmentInfo* si) {
    return si->hasSeparateNorms();
  }

  bool SegmentReader::hasDeletions(const SegmentInfo* si) {
  //Func - Static method
  //       Checks if a segment managed by SegmentInfo si-> has deletions
  //Pre  - si-> holds a valid reference to an SegmentInfo instance
  //Post - if the segement contains deleteions true is returned otherwise flas

    // Don't call ensureOpen() here (it could affect performance)
    return si->hasDeletions();
  }

	//synchronized
  void SegmentReader::doDelete(const int32_t docNum){
  //Func - Marks document docNum as deleted
  //Pre  - docNum >=0 and DocNum < maxDoc()
  //       docNum contains the number of the document that must be
  //       marked deleted
  //Post - The document identified by docNum has been marked deleted

      SCOPED_LOCK_MUTEX(THIS_LOCK)

     CND_PRECONDITION(docNum >= 0, "docNum is a negative number");
     CND_PRECONDITION(docNum < maxDoc(), "docNum is bigger than the total number of documents");

	  //Check if deletedDocs exists
	  if (deletedDocs == NULL){
          deletedDocs = _CLNEW BitSet(maxDoc());

          //Condition check to see if deletedDocs points to a valid instance
          CND_CONDITION(deletedDocs != NULL,"No memory could be allocated for deletedDocs");
	  }
      //Flag that there are documents marked deleted
      deletedDocsDirty = true;
      undeleteAll = false;
      //Mark document identified by docNum as deleted
      deletedDocs->set(docNum);

  }

  void SegmentReader::doUndeleteAll(){
      _CLDELETE(deletedDocs);
      deletedDocsDirty = false;
      undeleteAll = true;
  }

  void SegmentReader::files(vector<string>& retarray) {
  //Func - Returns all file names managed by this SegmentReader
  //Pre  - segment != NULL
  //Post - All filenames managed by this SegmentRead have been returned
    vector<string> tmp = si->files();
    retarray.insert(retarray.end(),tmp.begin(),tmp.end());
  }

  TermEnum* SegmentReader::terms() {
  //Func - Returns an enumeration of all the Terms and TermInfos in the set.
  //Pre  - tis != NULL
  //Post - An enumeration of all the Terms and TermInfos in the set has been returned

      CND_PRECONDITION(tis != NULL, "tis is NULL");

      ensureOpen();
      return tis->terms();
  }

  TermEnum* SegmentReader::terms(const Term* t) {
  //Func - Returns an enumeration of terms starting at or after the named term t
  //Pre  - t != NULL
  //       tis != NULL
  //Post - An enumeration of terms starting at or after the named term t

      CND_PRECONDITION(t   != NULL, "t is NULL");
      CND_PRECONDITION(tis != NULL, "tis is NULL");

      ensureOpen();
      return tis->terms(t);
  }

  bool SegmentReader::document(int32_t n, Document& doc, const FieldSelector* fieldSelector) {
  //Func - writes the fields of document n into doc
  //Pre  - n >=0 and identifies the document n
  //Post - if the document has been deleted then an exception has been thrown
  //       otherwise a reference to the found document has been returned

      SCOPED_LOCK_MUTEX(THIS_LOCK)

      ensureOpen();

      CND_PRECONDITION(n >= 0, "n is a negative number");

	  //Check if the n-th document has been marked deleted
       if (isDeleted(n)){
          _CLTHROWA( CL_ERR_InvalidState,"attempt to access a deleted document" );
       }

	   //Retrieve the n-th document
       return fieldsReader->doc(n, doc, fieldSelector);
  }


  bool SegmentReader::isDeleted(const int32_t n){
  //Func - Checks if the n-th document has been marked deleted
  //Pre  - n >=0 and identifies the document n
  //Post - true has been returned if document n has been deleted otherwise fralse

      SCOPED_LOCK_MUTEX(THIS_LOCK)

      CND_PRECONDITION(n >= 0, "n is a negative number");

	  //Is document n deleted
      bool ret = (deletedDocs != NULL && deletedDocs->get(n));

      return ret;
  }

  TermDocs* SegmentReader::termDocs() {
  //Func - Returns an unpositioned TermDocs enumerator.
  //Pre  - true
  //Post - An unpositioned TermDocs enumerator has been returned

        ensureOpen();
       return _CLNEW SegmentTermDocs(this);
  }

  TermPositions* SegmentReader::termPositions() {
  //Func - Returns an unpositioned TermPositions enumerator.
  //Pre  - true
  //Post - An unpositioned TermPositions enumerator has been returned

        ensureOpen();
      return _CLNEW SegmentTermPositions(this);
  }

  int32_t SegmentReader::docFreq(const Term* t) {
  //Func - Returns the number of documents which contain the term t
  //Pre  - t holds a valid reference to a Term
  //Post - The number of documents which contain term t has been returned

        ensureOpen();

      //Get the TermInfo ti for Term  t in the set
      TermInfo* ti = tis->get(t);
      //Check if an TermInfo has been returned
      if (ti){
		  //Get the frequency of the term
          int32_t ret = ti->docFreq;
		  //TermInfo ti is not needed anymore so delete it
          _CLDELETE( ti );
		  //return the number of documents which containt term t
          return ret;
          }
	  else
		  //No TermInfo returned so return 0
          return 0;
  }

  int32_t SegmentReader::numDocs() {
  //Func - Returns the actual number of documents in the segment
  //Pre  - true
  //Post - The actual number of documents in the segments

        ensureOpen();

	  //Get the number of all the documents in the segment including the ones that have
	  //been marked deleted
      int32_t n = maxDoc();

	  //Check if there any deleted docs
      if (deletedDocs != NULL)
		  //Substract the number of deleted docs from the number returned by maxDoc
          n -= deletedDocs->count();

	  //return the actual number of documents in the segment
      return n;
  }

  int32_t SegmentReader::maxDoc() const {
  //Func - Returns the number of  all the documents in the segment including
  //       the ones that have been marked deleted
  //Pre  - true
  //Post - The total number of documents in the segment has been returned

    // Don't call ensureOpen() here (it could affect performance)

      return si->docCount;
  }


  void SegmentReader::setTermInfosIndexDivisor(int32_t indexDivisor){
    tis->setIndexDivisor(indexDivisor);
  }

  int32_t SegmentReader::getTermInfosIndexDivisor() {
    return tis->getIndexDivisor();
  }


void SegmentReader::getFieldNames(FieldOption fldOption, StringArrayWithDeletor& retarray){
  ensureOpen();

	size_t len = _fieldInfos->size();
	for (size_t i = 0; i < len; i++) {
		FieldInfo* fi = _fieldInfos->fieldInfo(i);
		bool v=false;
		if (fldOption & IndexReader::ALL) {
			v=true;
		}else {
			if (!fi->isIndexed && (fldOption & IndexReader::UNINDEXED) )
				v=true;
			else if (fi->isIndexed && (fldOption & IndexReader::INDEXED) )
				v=true;
      else if (fi->storePayloads && (fldOption & IndexReader::STORES_PAYLOADS) )
        v=true;
			else if (fi->isIndexed && fi->storeTermVector == false && ( fldOption & IndexReader::INDEXED_NO_TERMVECTOR) )
				v=true;
			else if ( (fldOption & IndexReader::TERMVECTOR) &&
				    fi->storeTermVector == true &&
					fi->storePositionWithTermVector == false &&
					fi->storeOffsetWithTermVector == false )
				v=true;
			else if (fi->isIndexed && fi->storeTermVector && (fldOption & IndexReader::INDEXED_WITH_TERMVECTOR) )
				v=true;
			else if (fi->storePositionWithTermVector && fi->storeOffsetWithTermVector == false &&
					(fldOption & IndexReader::TERMVECTOR_WITH_POSITION))
				v=true;
			else if (fi->storeOffsetWithTermVector && fi->storePositionWithTermVector == false &&
					(fldOption & IndexReader::TERMVECTOR_WITH_OFFSET) )
				v=true;
			else if ((fi->storeOffsetWithTermVector && fi->storePositionWithTermVector) &&
					(fldOption & IndexReader::TERMVECTOR_WITH_POSITION_OFFSET) )
				v=true;
		}
		if ( v )
			retarray.push_back(STRDUP_TtoT(fi->name));
	}
}

bool SegmentReader::hasNorms(const TCHAR* field){
  ensureOpen();
	return _norms.find(field) != _norms.end();
}


  void SegmentReader::norms(const TCHAR* field, uint8_t* bytes) {
  //Func - Reads the Norms for field from disk starting at offset in the inputstream
  //Pre  - field != NULL
  //       bytes != NULL is an array of bytes which is to be used to read the norms into.
  //       it is advisable to have bytes initalized by zeroes!
  //Post - The if an inputstream to the norm file could be retrieved the bytes have been read
  //       You are never sure whether or not the norms have been read into bytes properly!!!!!!!!!!!!!!!!!

    CND_PRECONDITION(field != NULL, "field is NULL");
    CND_PRECONDITION(bytes != NULL, "field is NULL");

    SCOPED_LOCK_MUTEX(THIS_LOCK)

    ensureOpen();
    Norm* norm = _norms.get(field);
    if ( norm == NULL ){
      memcpy(bytes, fakeNorms(), maxDoc());
      return;
    }


    {SCOPED_LOCK_MUTEX(norm->THIS_LOCK)
      if (norm->bytes != NULL) { // can copy from cache
      memcpy(bytes, norm->bytes, maxDoc());
        return;
      }

      // Read from disk.  norm.in may be shared across  multiple norms and
      // should only be used in a synchronized context.
      IndexInput* normStream;
      if (norm->useSingleNormStream) {
        normStream = singleNormStream;
      } else {
        normStream = norm->in;
      }
      normStream->seek(norm->normSeek);
      normStream->readBytes(bytes, maxDoc());
    }
  }

  uint8_t* SegmentReader::createFakeNorms(int32_t size) {
    uint8_t* ones = _CL_NEWARRAY(uint8_t,size);
    if ( size > 0 )
      memset(ones, DefaultSimilarity::encodeNorm(1.0f), size);
    return ones;
  }

  uint8_t* SegmentReader::fakeNorms() {
    if (ones==NULL)
		ones=createFakeNorms(maxDoc());
    return ones;
  }
  // can return NULL if norms aren't stored
  uint8_t* SegmentReader::getNorms(const TCHAR* field) {
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    Norm* norm = _norms.get(field);
    if (norm == NULL)  return NULL;  // not indexed, or norms not stored

    {SCOPED_LOCK_MUTEX(norm->THIS_LOCK)
      if (norm->bytes == NULL) {                     // value not yet read
        uint8_t* bytes = _CL_NEWARRAY(uint8_t, maxDoc());
        norms(field, bytes);
        norm->bytes = bytes;                         // cache it
        // it's OK to close the underlying IndexInput as we have cached the
        // norms and will never read them again.
        norm->close();
      }
        return norm->bytes;
    }
  }

  void SegmentReader::decRefNorms(){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    NormsType::iterator it = _norms.begin();
    while (it != _norms.end()) {
      Norm* norm = it->second;
      norm->decRef();
      it++;
    }
  }

  DirectoryIndexReader* SegmentReader::doReopen(SegmentInfos* infos){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    DirectoryIndexReader* newReader;

    if (infos->size() == 1) {
      SegmentInfo* si = infos->info(0);
      if (segment.compare(si->name)==0 && si->getUseCompoundFile() == this->si->getUseCompoundFile()) {
        newReader = reopenSegment(si);
      } else {
        // segment not referenced anymore, reopen not possible
        // or segment format changed
        newReader = SegmentReader::get(infos, infos->info(0), false);
      }
    } else {
      ValueArray<IndexReader*> readers(1);
      readers.values[0] = this;
      return _CLNEW MultiSegmentReader(_directory, infos, closeDirectory, &readers, NULL, NULL);
    }

    return newReader;
  }


  uint8_t* SegmentReader::norms(const TCHAR* field) {
  //Func - Returns the bytes array that holds the norms of a named field
  //Pre  - field != NULL and contains the name of the field for which the norms
  //       must be retrieved
  //Post - If there was norm for the named field then a bytes array has been allocated
  //       and returned containing the norms for that field. If the named field is unknown NULL is returned.

    CND_PRECONDITION(field != NULL, "field is NULL");
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    ensureOpen();
    uint8_t* bytes = getNorms(field);
    if (bytes==NULL)
		bytes=fakeNorms();
    return bytes;
  }

  void SegmentReader::doSetNorm(int32_t doc, const TCHAR* field, uint8_t value){
    Norm* norm = _norms.get(field);
    if (norm == NULL)                             // not an indexed field
      return;
    norm->dirty = true;                            // mark it dirty
    normsDirty = true;

    uint8_t* bits = norms(field);
    bits[doc] = value;                    // set the value
  }


  string SegmentReader::SegmentName(const char* ext, const int32_t x){
  //Func - Returns an allocated buffer in which it creates a filename by
  //       concatenating segment with ext and x
  //Pre    ext != NULL and holds the extension
  //       x contains a number
  //Post - A buffer has been instantiated an when x = -1 buffer contains the concatenation of
  //       segment and ext otherwise buffer contains the contentation of segment, ext and x

	  CND_PRECONDITION(ext     != NULL, "ext is NULL");

    return Misc::segmentname(segment.c_str(),ext,x);
  }

  void SegmentReader::openNorms(Directory* cfsDir, int32_t readBufferSize) {
  //Func - Open all norms files for all fields
  //       Creates for each field a norm Instance with an open inputstream to
  //       a corresponding norm file ready to be read
  //Pre  - true
  //Post - For each field a norm instance has been created with an open inputstream to
  //       a corresponding norm file ready to be read
    int64_t nextNormSeek = SegmentMerger::NORMS_HEADER_length; //skip header (header unused for now)
    int32_t _maxDoc = maxDoc();
    for (size_t i = 0; i < _fieldInfos->size(); i++) {
      FieldInfo* fi = _fieldInfos->fieldInfo(i);
      if (_norms.find(fi->name) != _norms.end()) {
        // in case this SegmentReader is being re-opened, we might be able to
        // reuse some norm instances and skip loading them here
        continue;
      }
      if (fi->isIndexed && !fi->omitNorms) {
        Directory* d = directory();
        string fileName = si->getNormFileName(fi->number);
        if (!si->hasSeparateNorms(fi->number)) {
          d = cfsDir;
        }

        // singleNormFile means multiple norms share this file
        string ext = string(".") + IndexFileNames::NORMS_EXTENSION;
        bool singleNormFile = fileName.compare(fileName.length()-ext.length(),ext.length(),ext)==0;
        IndexInput* normInput = NULL;
        int64_t normSeek;

        if (singleNormFile) {
          normSeek = nextNormSeek;
          if (singleNormStream==NULL) {
            singleNormStream = d->openInput(fileName.c_str(), readBufferSize);
          }
          // All norms in the .nrm file can share a single IndexInput since
          // they are only used in a synchronized context.
          // If this were to change in the future, a clone could be done here.
          normInput = singleNormStream;
        } else {
          normSeek = 0;
          normInput = d->openInput(fileName.c_str());
        }

        _norms[fi->name] = _CLNEW Norm(normInput, singleNormFile, fi->number, normSeek, this, segment.c_str());
        nextNormSeek += _maxDoc; // increment also if some norms are separate
      }
    }
  }


	TermVectorsReader* SegmentReader::getTermVectorsReader() {
		TermVectorsReader* tvReader = termVectorsLocal.get();
		if (tvReader == NULL) {
		  tvReader = termVectorsReaderOrig->clone();
		  termVectorsLocal.set(tvReader);
		}
		return tvReader;
	}

  FieldsReader* SegmentReader::getFieldsReader() {
    return fieldsReader;
  }

  FieldInfos* SegmentReader::getFieldInfos() {
    return _fieldInfos;
  }

   TermFreqVector* SegmentReader::getTermFreqVector(int32_t docNumber, const TCHAR* field){
    ensureOpen();
 		if ( field != NULL ){
			// Check if this field is invalid or has no stored term vector
			FieldInfo* fi = _fieldInfos->fieldInfo(field);
			if (fi == NULL || !fi->storeTermVector || termVectorsReaderOrig == NULL )
				return NULL;
		}
		TermVectorsReader* termVectorsReader = getTermVectorsReader();
		if (termVectorsReader == NULL)
		  return NULL;
		return termVectorsReader->get(docNumber, field);
  }

  ArrayBase<TermFreqVector*>* SegmentReader::getTermFreqVectors(int32_t docNumber) {
    ensureOpen();
    if (termVectorsReaderOrig == NULL)
      return NULL;

    TermVectorsReader* termVectorsReader = getTermVectorsReader();
    if (termVectorsReader == NULL)
      return NULL;

	  return termVectorsReader->get(docNumber);
  }



  void SegmentReader::getTermFreqVector(int32_t docNumber, const TCHAR* field, TermVectorMapper* mapper) {
    ensureOpen();
    FieldInfo* fi = _fieldInfos->fieldInfo(field);
    if (fi == NULL || !fi->storeTermVector || termVectorsReaderOrig == NULL)
      return;

    TermVectorsReader* termVectorsReader = getTermVectorsReader();
    if (termVectorsReader == NULL)
    {
      return;
    }


    termVectorsReader->get(docNumber, field, mapper);
  }


  void SegmentReader::getTermFreqVector(int32_t docNumber, TermVectorMapper* mapper){
    ensureOpen();
    if (termVectorsReaderOrig == NULL)
      return;

    TermVectorsReader* termVectorsReader = getTermVectorsReader();
    if (termVectorsReader == NULL)
      return;

    termVectorsReader->get(docNumber, mapper);
  }


  void SegmentReader::loadDeletedDocs(){
    // NOTE: the bitvector is stored using the regular directory, not cfs
    if (hasDeletions(si)) {
      deletedDocs = _CLNEW BitVector(directory(), si->getDelFileName().c_str());

      // Verify # deletes does not exceed maxDoc for this segment:
      if (deletedDocs->count() > maxDoc()) {
        string err = "number of deletes (";
        err += deletedDocs->count();
        err += ") exceeds max doc (";
        err += maxDoc();
        err += ") for segment ";
        err += si->name;
        _CLTHROWA(CL_ERR_CorruptIndex, err.c_str());
      }
    }
  }

  SegmentReader* SegmentReader::reopenSegment(SegmentInfo* si){
    SCOPED_LOCK_MUTEX(THIS_LOCK)
    bool deletionsUpToDate = (this->si->hasDeletions() == si->hasDeletions())
                                  && (!si->hasDeletions() || this->si->getDelFileName().compare(si->getDelFileName())==0 );
    bool normsUpToDate = true;


    ValueArray<bool>fieldNormsChanged(_fieldInfos->size());
    if (normsUpToDate) {
      for (size_t i = 0; i < _fieldInfos->size(); i++) {
        if (this->si->getNormFileName(i).compare(si->getNormFileName(i)) != 0) {
          normsUpToDate = false;
          fieldNormsChanged.values[i] = true;
        }
      }
    }

    if (normsUpToDate && deletionsUpToDate) {
      this->si = si; //force the result to use the new segment info (the old one is going to go away!)
      return this;
    }


      // clone reader
    SegmentReader* clone = NULL;
    bool success = false;
    try {
      clone = _CLNEW SegmentReader();
      clone->init(_directory, NULL, false);
      clone->initialize(si, readBufferSize, false, true);
      clone->cfsReader = cfsReader;
      clone->storeCFSReader = storeCFSReader;
      clone->_fieldInfos = _fieldInfos;
      clone->tis = tis;
      clone->freqStream = freqStream;
      clone->proxStream = proxStream;
      clone->termVectorsReaderOrig = termVectorsReaderOrig;

      // we have to open a new FieldsReader, because it is not thread-safe
      // and can thus not be shared among multiple SegmentReaders
      // TODO: Change this in case FieldsReader becomes thread-safe in the future
      string fieldsSegment;

      Directory* storeDir = directory();

      if (si->getDocStoreOffset() != -1) {
        fieldsSegment = si->getDocStoreSegment();
        if (storeCFSReader != NULL) {
          storeDir = storeCFSReader;
        }
      } else {
        fieldsSegment = segment;
        if (cfsReader != NULL) {
          storeDir = cfsReader;
        }
      }

      if (fieldsReader != NULL) {
        clone->fieldsReader = _CLNEW FieldsReader(storeDir, fieldsSegment.c_str(), _fieldInfos, readBufferSize,
                                        si->getDocStoreOffset(), si->docCount);
      }


      if (!deletionsUpToDate) {
        // load deleted docs
        clone->deletedDocs = NULL;
        clone->loadDeletedDocs();
      } else {
        clone->deletedDocs = this->deletedDocs;
      }

      if (!normsUpToDate) {
        // load norms
        for (size_t i = 0; i < fieldNormsChanged.length; i++) {
          // copy unchanged norms to the cloned reader and incRef those norms
          if (!fieldNormsChanged[i]) {
            const TCHAR* curField = _fieldInfos->fieldInfo(i)->name;
            Norm* norm = this->_norms.get(curField);
            norm->incRef();
            norm->_this = clone; //give the norm to the clone
          clone->_norms.put(curField, norm);
          }
        }

        clone->openNorms(si->getUseCompoundFile() ? cfsReader : directory(), readBufferSize);
      } else {
        NormsType::iterator it = _norms.begin();
        while (it != _norms.end()) {
          const TCHAR* field = it->first;
          Norm* norm = _norms[field];
          norm->incRef();
          norm->_this = clone; //give the norm to the clone
          clone->_norms.put(field, norm);
          it++;
        }
      }

      if (clone->singleNormStream == NULL) {
        for (size_t i = 0; i < _fieldInfos->size(); i++) {
          FieldInfo* fi = _fieldInfos->fieldInfo(i);
          if (fi->isIndexed && !fi->omitNorms) {
            Directory* d = si->getUseCompoundFile() ? cfsReader : directory();
            string fileName = si->getNormFileName(fi->number);
            if (si->hasSeparateNorms(fi->number)) {
              continue;
            }

            string ext = string(".") + IndexFileNames::NORMS_EXTENSION;
            if (fileName.compare(fileName.length()-ext.length(),ext.length(),ext)==0) {
              clone->singleNormStream = d->openInput(fileName.c_str(), readBufferSize);
              break;
            }
          }
        }
      }

      success = true;
    } _CLFINALLY (
      if (!success) {
        // An exception occured during reopen, we have to decRef the norms
        // that we incRef'ed already and close singleNormsStream and FieldsReader
        clone->decRefNorms();
      }
    )

    //disown this memory
    this->freqStream = NULL;
    this->_fieldInfos = NULL;
    this->tis = NULL;
    this->deletedDocs = NULL;
    this->ones = NULL;
    this->termVectorsReaderOrig = NULL;
    this->cfsReader = NULL;
    this->freqStream = NULL;
    this->proxStream = NULL;
    this->termVectorsReaderOrig = NULL;
    this->cfsReader = NULL;
    this->storeCFSReader = NULL;
    this->singleNormStream = NULL;

    return clone;
  }



  /** Returns the field infos of this segment */
  FieldInfos* SegmentReader::fieldInfos() {
    return _fieldInfos;
  }

  /**
   * Return the name of the segment this reader is reading.
   */
  const char* SegmentReader::getSegmentName() {
    return segment.c_str();
  }

  /**
   * Return the SegmentInfo of the segment this reader is reading.
   */
  SegmentInfo* SegmentReader::getSegmentInfo() {
    return si;
  }

  void SegmentReader::setSegmentInfo(SegmentInfo* info) {
    si = info;
  }

  void SegmentReader::startCommit() {
    DirectoryIndexReader::startCommit();
    rollbackDeletedDocsDirty = deletedDocsDirty;
    rollbackNormsDirty = normsDirty;
    rollbackUndeleteAll = undeleteAll;
    NormsType::iterator it = _norms.begin();
    while (it != _norms.end()) {
      Norm* norm = it->second;
      norm->rollbackDirty = norm->dirty;
    }
  }

  void SegmentReader::rollbackCommit() {
    DirectoryIndexReader::rollbackCommit();
    deletedDocsDirty = rollbackDeletedDocsDirty;
    normsDirty = rollbackNormsDirty;
    undeleteAll = rollbackUndeleteAll;
    NormsType::iterator it = _norms.begin();
    while (it != _norms.end()) {
      Norm* norm = it->second;
      norm->dirty = norm->rollbackDirty;
    }
  }

  const char* SegmentReader::getClassName(){
    return "SegmentReader";
  }
  const char* SegmentReader::getObjectName() const{
    return getClassName();
  }

  bool SegmentReader::normsClosed() {
    if (singleNormStream != NULL) {
      return false;
    }
    NormsType::iterator it = _norms.begin();
    while ( it != _norms.end() ) {
      Norm* norm = it->second;
      if (norm->refCount > 0) {
        return false;
      }
    }
    return true;
  }
CL_NS_END
