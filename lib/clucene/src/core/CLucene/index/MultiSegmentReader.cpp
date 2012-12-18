/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "IndexReader.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/FieldSelector.h"
#include "Term.h"
#include "Terms.h"
#include "CLucene/util/PriorityQueue.h"
#include "_SegmentHeader.h"
#include "_SegmentMergeInfo.h"
#include "_SegmentMergeQueue.h"
#include "MultiReader.h"
#include "_MultiSegmentReader.h"

CL_NS_USE(document)
CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)

void MultiSegmentReader::initialize(CL_NS(util)::ArrayBase<IndexReader*>* _subReaders){
  this->subReaders = _subReaders;

  _maxDoc        = 0;
  _numDocs       = -1;
  ones           = NULL;
  _hasDeletions  = false;

  starts = _CL_NEWARRAY(int32_t, subReaders->length + 1);    // build starts array
  for (size_t i = 0; i < subReaders->length; i++) {
      starts[i] = _maxDoc;

      // compute maxDocs
      _maxDoc += (*subReaders)[i]->maxDoc();
      if ((*subReaders)[i]->hasDeletions())
        _hasDeletions = true;
  }
  starts[subReaders->length] = _maxDoc;
}

MultiSegmentReader::MultiSegmentReader(CL_NS(store)::Directory* directory, SegmentInfos* sis, bool closeDirectory):
  DirectoryIndexReader(directory,sis,closeDirectory),
  normsCache(NormsCacheType(true,true))
{
  // To reduce the chance of hitting FileNotFound
  // (and having to retry), we open segments in
  // reverse because IndexWriter merges & deletes
  // the newest segments first.

  ArrayBase<IndexReader*>* readers = _CLNEW ObjectArray<IndexReader>(sis->size());
  for (int32_t i = (int32_t)sis->size()-1; i >= 0; i--) {
    try {
      readers->values[i] = SegmentReader::get(sis->info(i));
    } catch(CLuceneError& err) {
      if ( err.number() != CL_ERR_IO ) throw err;

      // Close all readers we had opened:
      for(i++;i<sis->size();i++) {
        try {
          (*readers)[i]->close();
        } catch (CLuceneError& err2) {
          if ( err.number() != CL_ERR_IO ) throw err2;
          // keep going - we want to clean up as much as possible
        }
      }
      throw err;
    }
  }
  initialize(readers);
}

/** This contructor is only used for {@link #reopen()} */
MultiSegmentReader::MultiSegmentReader(
      CL_NS(store)::Directory* directory,
      SegmentInfos* infos,
      bool closeDirectory,
      CL_NS(util)::ArrayBase<IndexReader*>* oldReaders,
      int32_t* oldStarts,
      NormsCacheType* oldNormsCache):
  DirectoryIndexReader(directory, infos, closeDirectory),
  normsCache(NormsCacheType(true,true))
{
  // we put the old SegmentReaders in a map, that allows us
  // to lookup a reader using its segment name
  map<string,size_t> segmentReaders;
  if (oldReaders != NULL) {
    // create a Map SegmentName->SegmentReader
    for (size_t i = 0; i < oldReaders->length; i++) {
      segmentReaders[((SegmentReader*)(*oldReaders)[i])->getSegmentName()] = i;
    }
  }

  ArrayBase<IndexReader*>* newReaders = _CLNEW ObjectArray<IndexReader>(infos->size());

  for (int32_t i = infos->size() - 1; i>=0; i--) {
    // find SegmentReader for this segment
    map<string,size_t>::iterator oldReaderIndex = segmentReaders.find(infos->info(i)->name);
    if ( oldReaderIndex == segmentReaders.end()) {
      // this is a new segment, no old SegmentReader can be reused
      newReaders->values[i] = NULL;
    } else {
      // there is an old reader for this segment - we'll try to reopen it
      newReaders->values[i] = (*oldReaders)[oldReaderIndex->second];
    }

    bool success = false;
    try {
      SegmentReader* newReader;
      if ((*newReaders)[i] == NULL || infos->info(i)->getUseCompoundFile() != ((SegmentReader*)(*newReaders)[i])->getSegmentInfo()->getUseCompoundFile()) {
        // this is a new reader; in case we hit an exception we can close it safely
        newReader = SegmentReader::get(infos->info(i));
      } else {
        newReader = ((SegmentReader*)(*newReaders)[i])->reopenSegment(infos->info(i));
      }
      if (newReader == (*newReaders)[i]) {
        // this reader is being re-used, so we take ownership of it...
        oldReaders->values[i] = NULL;
      }

      newReaders->values[i] = newReader;
      success = true;
    } _CLFINALLY (
      if (!success) {
        for (i++; i < infos->size(); i++) {
          if (newReaders->values[i] != NULL) {
            try {
              (*newReaders)[i]->close();
              _CLDELETE(newReaders->values[i]);
            }catch(CLuceneError& ignore){
              if ( ignore.number() != CL_ERR_IO ) throw ignore;
              // keep going - we want to clean up as much as possible
            }
          }
        }
      }
    )
  }

  // initialize the readers to calculate maxDoc before we try to reuse the old normsCache
  initialize(newReaders);

  // try to copy unchanged norms from the old normsCache to the new one
  if (oldNormsCache != NULL) {
    NormsCacheType::iterator it = oldNormsCache->begin();
    while (it != oldNormsCache->end()) {
      TCHAR* field = it->first;
      if (!hasNorms(field)) {
        continue;
      }
      uint8_t* oldBytes = it->second;
      uint8_t* bytes = _CL_NEWARRAY(uint8_t,maxDoc());


      for (size_t i = 0; i < subReaders->length; i++) {
        map<string,size_t>::iterator oldReaderIndex = segmentReaders.find(((SegmentReader*)(*subReaders)[i])->getSegmentName());

        // this SegmentReader was not re-opened, we can copy all of its norms
        if (oldReaderIndex != segmentReaders.end() &&
            ((*oldReaders)[oldReaderIndex->second] == (*subReaders)[i]
            || ((SegmentReader*)(*oldReaders)[oldReaderIndex->second])->_norms.get(field) == ((SegmentReader*)(*subReaders)[i])->_norms.get(field))) {
          // we don't have to synchronize here: either this constructor is called from a SegmentReader,
          // in which case no old norms cache is present, or it is called from MultiReader.reopen(),
          // which is synchronized
          memcpy(bytes + starts[i], oldBytes + oldStarts[oldReaderIndex->second], starts[i+1] - starts[i]);
        } else {
          (*subReaders)[i]->norms(field, bytes+starts[i]);
        }
      }

      normsCache.put(field, bytes);      // update cache

      it++;
    }
  }
}



MultiSegmentReader::~MultiSegmentReader() {
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed all IndexReader instances
//       this instance managed have been destroyed to

  _CLDELETE_ARRAY(ones);
  _CLDELETE_ARRAY(starts);

  //Iterate through the subReaders and destroy each reader
  _CLDELETE(subReaders);
}

const char* MultiTermEnum::getObjectName() const{ return getClassName(); }
const char* MultiTermEnum::getClassName(){ return "MultiTermEnum"; }

 DirectoryIndexReader* MultiSegmentReader::doReopen(SegmentInfos* infos){
   SCOPED_LOCK_MUTEX(THIS_LOCK)
    if (infos->size() == 1) {
      // The index has only one segment now, so we can't refresh the MultiSegmentReader.
      // Return a new SegmentReader instead
      return SegmentReader::get(infos, infos->info(0), false);
    } else {
      return _CLNEW MultiSegmentReader(_directory, infos, closeDirectory, subReaders, starts, &normsCache);
    }
  }


ArrayBase<TermFreqVector*>* MultiSegmentReader::getTermFreqVectors(int32_t n){
  ensureOpen();
	int32_t i = readerIndex(n);        // find segment num
	return (*subReaders)[i]->getTermFreqVectors(n - starts[i]); // dispatch to segment
}

TermFreqVector* MultiSegmentReader::getTermFreqVector(int32_t n, const TCHAR* field){
  ensureOpen();
	int32_t i = readerIndex(n);        // find segment num
	return (*subReaders)[i]->getTermFreqVector(n - starts[i], field);
}
void MultiSegmentReader::getTermFreqVector(int32_t docNumber, const TCHAR* field, TermVectorMapper* mapper){
  ensureOpen();
  int32_t i = readerIndex(docNumber);        // find segment num
  (*subReaders)[i]->getTermFreqVector(docNumber - starts[i], field, mapper);
}

void MultiSegmentReader::getTermFreqVector(int32_t docNumber, TermVectorMapper* mapper){
  ensureOpen();
  int32_t i = readerIndex(docNumber);        // find segment num
  (*subReaders)[i]->getTermFreqVector(docNumber - starts[i], mapper);
}


bool MultiSegmentReader::isOptimized() {
  return false;
}

int32_t MultiSegmentReader::numDocs() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
    // Don't call ensureOpen() here (it could affect performance)
	if (_numDocs == -1) {			  // check cache
	  int32_t n = 0;				  // cache miss--recompute
	  for (size_t i = 0; i < subReaders->length; i++)
	    n += (*subReaders)[i]->numDocs();		  // sum from readers
	  _numDocs = n;
	}
	return _numDocs;
}

int32_t MultiSegmentReader::maxDoc() const {
    // Don't call ensureOpen() here (it could affect performance)
	return _maxDoc;
}

const ArrayBase<IndexReader*>* MultiSegmentReader::getSubReaders() const{
  return subReaders;
}

bool MultiSegmentReader::document(int32_t n, CL_NS(document)::Document& doc, const FieldSelector* fieldSelector){
	ensureOpen();
  int32_t i = readerIndex(n);			  // find segment num
	return (*subReaders)[i]->document(n - starts[i],doc, fieldSelector);	  // dispatch to segment reader
}

bool MultiSegmentReader::isDeleted(const int32_t n) {
    // Don't call ensureOpen() here (it could affect performance)
	int32_t i = readerIndex(n);			  // find segment num
	return (*subReaders)[i]->isDeleted(n - starts[i]);	  // dispatch to segment reader
}

bool MultiSegmentReader::hasDeletions() const{
    // Don't call ensureOpen() here (it could affect performance)
    return _hasDeletions;
}

uint8_t* MultiSegmentReader::norms(const TCHAR* field){
	SCOPED_LOCK_MUTEX(THIS_LOCK)
    ensureOpen();
	uint8_t* bytes;
	bytes = normsCache.get((TCHAR*)field);
	if (bytes != NULL){
	  return bytes;				  // cache hit
	}

	if ( !hasNorms(field) )
		return fakeNorms();

	bytes = _CL_NEWARRAY(uint8_t,maxDoc());
	for (size_t i = 0; i < subReaders->length; i++)
	  (*subReaders)[i]->norms(field, bytes + starts[i]);

	//Unfortunately the data in the normCache can get corrupted, since it's being loaded with string
	//keys that may be deleted while still in use by the map. To prevent this field is duplicated
	//and then stored in the normCache
	TCHAR* key = STRDUP_TtoT(field);
	//update cache
	normsCache.put(key, bytes);

	return bytes;
}

void MultiSegmentReader::norms(const TCHAR* field, uint8_t* result) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
    ensureOpen();
	uint8_t* bytes = normsCache.get((TCHAR*)field);
	if (bytes==NULL && !hasNorms(field))
		bytes=fakeNorms();

	if (bytes != NULL){                            // cache hit
	   int32_t len = maxDoc();
	   memcpy(result,bytes,len * sizeof(int32_t));
	}

	for (size_t i = 0; i < subReaders->length; i++)      // read from segments
	  (*subReaders)[i]->norms(field, result + starts[i]);
}


void MultiSegmentReader::doSetNorm(int32_t n, const TCHAR* field, uint8_t value){
	normsCache.removeitr( normsCache.find((TCHAR*)field) );                         // clear cache
	int32_t i = readerIndex(n);                           // find segment num
	(*subReaders)[i]->setNorm(n-starts[i], field, value); // dispatch
}

TermEnum* MultiSegmentReader::terms() {
  ensureOpen();
	return _CLNEW MultiTermEnum(subReaders, starts, NULL);
}

TermEnum* MultiSegmentReader::terms(const Term* term) {
    ensureOpen();
	return _CLNEW MultiTermEnum(subReaders, starts, term);
}

int32_t MultiSegmentReader::docFreq(const Term* t) {
    ensureOpen();
	int32_t total = 0;				  // sum freqs in Multi
	for (size_t i = 0; i < subReaders->length; i++)
	  total += (*subReaders)[i]->docFreq(t);
	return total;
}

TermDocs* MultiSegmentReader::termDocs() {
    ensureOpen();
	TermDocs* ret =  _CLNEW MultiTermDocs(subReaders, starts);
	return ret;
}

TermPositions* MultiSegmentReader::termPositions() {
    ensureOpen();
	TermPositions* ret = static_cast<TermPositions*>(_CLNEW MultiTermPositions(subReaders, starts));
	return ret;
}

void MultiSegmentReader::setTermInfosIndexDivisor(int32_t indexDivisor) {
  for (size_t i = 0; i < subReaders->length; i++)
    (*subReaders)[i]->setTermInfosIndexDivisor(indexDivisor);
}

int32_t MultiSegmentReader::getTermInfosIndexDivisor() {
  if (subReaders->length > 0)
    return (*subReaders)[0]->getTermInfosIndexDivisor();
  else
    _CLTHROWA(CL_ERR_IllegalState,"no readers");
}

void MultiSegmentReader::doDelete(const int32_t n) {
	_numDocs = -1;				  // invalidate cache
	int32_t i = readerIndex(n);			  // find segment num
	(*subReaders)[i]->deleteDocument(n - starts[i]);		  // dispatch to segment reader
	_hasDeletions = true;
}

int32_t MultiSegmentReader::readerIndex(int32_t n) const{    // find reader for doc n:
  return readerIndex(n, this->starts, this->subReaders->length);
}


int32_t MultiSegmentReader::readerIndex(const int32_t n, int32_t* starts, int32_t numSubReaders) {	  // find reader for doc n:
	int32_t lo = 0;					   // search starts array
	int32_t hi = numSubReaders - 1;	// for first element less
	                                // than n, return its index
	while (hi >= lo) {
	  int32_t mid = (lo + hi) >> 1;
	  int32_t midValue = starts[mid];
	  if (n < midValue)
	    hi = mid - 1;
	  else if (n > midValue)
	    lo = mid + 1;
	  else{                                      // found a match
	    while (mid+1 < numSubReaders && starts[mid+1] == midValue) {
	      mid++;                                  // scan to last match
	    }
	    return mid;
	  }
	}
	return hi;
}

bool MultiSegmentReader::hasNorms(const TCHAR* field) {
    ensureOpen();
	for (size_t i = 0; i < subReaders->length; i++) {
		if ((*subReaders)[i]->hasNorms(field))
			return true;
	}
	return false;
}
uint8_t* MultiSegmentReader::fakeNorms() {
	if (ones==NULL)
		ones=SegmentReader::createFakeNorms(maxDoc());
	return ones;
}

void MultiSegmentReader::doUndeleteAll(){
	for (size_t i = 0; i < subReaders->length; i++)
		(*subReaders)[i]->undeleteAll();
	_hasDeletions = false;
	_numDocs = -1;
}
void MultiSegmentReader::commitChanges() {
	for (size_t i = 0; i < subReaders->length; i++)
	  (*subReaders)[i]->commit();
}

void MultiSegmentReader::doClose() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
	for (size_t i = 0; i < subReaders->length; i++){
	  if ( (*subReaders)[i] != NULL ){
	    (*subReaders)[i]->close();
	    _CLDELETE(subReaders->values[i]);
	  }
	}
  // maybe close directory
  DirectoryIndexReader::doClose();
}

void MultiSegmentReader::getFieldNames(FieldOption fieldNames, StringArrayWithDeletor& retarray, CL_NS(util)::ArrayBase<IndexReader*>* subReaders) {
  // maintain a unique set of field names
  for (size_t i = 0; i < subReaders->length; i++) {
    IndexReader* reader = (*subReaders)[i];
    StringArrayWithDeletor subFields(false);
    reader->getFieldNames(fieldNames, subFields);
    retarray.insert(retarray.end(),subFields.begin(),subFields.end());
    subFields.clear();
  }
}


void MultiSegmentReader::getFieldNames(FieldOption fldOption, StringArrayWithDeletor& retarray){
    StringArrayWithDeletor temp;
    CLHashList<TCHAR*> hashList;
    for (size_t i = 0; i < subReaders->length; i++) {
      IndexReader* reader = (*subReaders)[i];
      reader->getFieldNames(fldOption, temp);

      //create a unique list of names.
      StringArrayWithDeletor::iterator itr = temp.begin();
      while ( itr != temp.end() ){
          if ( hashList.find(*itr) == hashList.end() )
            hashList.insert(STRDUP_TtoT(*itr));
          itr++;
      }
    }
    //move the items into the return
    CLHashList<TCHAR*>::iterator itr = hashList.begin();
    while ( itr != hashList.end() ){
      retarray.push_back(*itr);//no need to copy, already done!
      itr++;
    }
}
const char* MultiSegmentReader::getClassName(){
  return "MultiSegmentReader";
}
const char* MultiSegmentReader::getObjectName() const{
  return getClassName();
}










void MultiTermDocs::init(ArrayBase<IndexReader*>* r, const int32_t* s){
	subReaders       = r;
	starts        = s;
	base          = 0;
	pointer       = 0;
	current       = NULL;
	term          = NULL;
	readerTermDocs   = NULL;

	//Check if there are subReaders
	if(subReaders != NULL && subReaders->length > 0){
	  readerTermDocs = _CLNEW ValueArray<TermDocs*>(subReaders->length);
	}
}
MultiTermDocs::MultiTermDocs(){
//Func - Default constructor
//       Initialises an empty MultiTermDocs.
//       This constructor is needed to allow the constructor of MultiTermPositions
//       initialise the instance by itself
//Pre  - true
//Post - An empty

	init(NULL,NULL);
}

MultiTermDocs::MultiTermDocs(ArrayBase<IndexReader*>* r, const int32_t* s){
//Func - Constructor
//Pre  - if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
//       s != NULL
//Post - The instance has been created
  init(r,s);
}

MultiTermDocs::~MultiTermDocs(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed

  close();
}


TermPositions* MultiTermDocs::__asTermPositions(){
  return NULL;
}

int32_t MultiTermDocs::doc() const {
  CND_PRECONDITION(current!=NULL,"current==NULL, check that next() was called");
  return base + current->doc();
}
int32_t MultiTermDocs::freq() const {
	CND_PRECONDITION(current!=NULL,"current==NULL, check that next() was called");
	return current->freq();
}

void MultiTermDocs::seek(TermEnum* termEnum){
	seek(termEnum->term(false));
}

void MultiTermDocs::seek( Term* tterm) {
//Func - Resets the instance for a new search
//Pre  - tterm != NULL
//Post - The instance has been reset for a new search

	CND_PRECONDITION(tterm != NULL, "tterm is NULL");

	//Assigning tterm is done as below for a reason
	//The construction ensures that if seek is called from within
	//MultiTermDocs with as argument this->term (seek(this->term)) that the assignment
	//will succeed and all referencecounters represent the correct situation

	//Get a pointer from tterm and increase its reference counter
	Term *TempTerm = _CL_POINTER(tterm);
//xx
	//Finialize term to ensure we decrease the reference counter of the instance which term points to
	_CLDECDELETE(term);

	//Assign TempTerm to term
	term = TempTerm;

	base = 0;
	pointer = 0;
	current = NULL;
}

bool MultiTermDocs::next() {
	for(;;) {
		if (current != NULL && current->next()) {
		  return true;
		} else if (pointer < subReaders->length) {
		  base = starts[pointer];
		  current = termDocs(pointer++);
		} else {
		  return false;
		}
	}
}

int32_t MultiTermDocs::read(int32_t* docs, int32_t* freqs, int32_t length) {
	while (true) {
	  while (current == NULL) {
	    if (pointer < subReaders->length) {		  // try next segment
	      base = starts[pointer];
	      current = termDocs(pointer++);
	    } else {
	      return 0;
	    }
	  }
	  int32_t end = current->read(docs, freqs,length);
	  if (end == 0) {				  // none left in segment
	    current = NULL;
	  } else {					  // got some
	    int32_t b = base;			  // adjust doc numbers
	    for (int32_t i = 0; i < end; i++)
	      docs[i] += b;
	    return end;
	  }
	}
}

bool MultiTermDocs::skipTo(const int32_t target) {
//	do {
//	  if (!next())
//	    return false;
//	} while (target > doc());
//	return true;
	for(;;) {
		if ( current != NULL && current->skipTo(target - base)) {
			return true;
		} else if ( pointer < subReaders->length ) {
			base = starts[pointer];
			current = termDocs(pointer++);
		} else {
			return false;
		}
	}
}

void MultiTermDocs::close() {
//Func - Closes all MultiTermDocs managed by this instance
//Pre  - true
//Post - All the MultiTermDocs have been closed


	//Check if readerTermDocs is valid
	if (readerTermDocs){
    TermDocs* curTD = NULL;
    //iterate through the readerTermDocs array
    for (size_t i = 0; i < subReaders->length; i++) {
        //Retrieve the i-th TermDocs instance
        curTD = (*readerTermDocs)[i];

        //Check if it is a valid pointer
        if (curTD != NULL) {
            //Close it
            curTD->close();
            _CLDELETE(curTD);
        }
    }

    _CLDELETE(readerTermDocs);
	}

	//current previously pointed to a member of readerTermDocs; ensure that
	//it doesn't now point to invalid memory.
	current = NULL;
	base          = 0;
	pointer       = 0;

	_CLDECDELETE(term);
}

TermDocs* MultiTermDocs::termDocs(IndexReader* reader) {
	return reader->termDocs();
}

TermDocs* MultiTermDocs::termDocs(const int32_t i) {
	if (term == NULL)
	  return NULL;
	TermDocs* result = (*readerTermDocs)[i];
	if (result == NULL){
    _CLLDELETE(readerTermDocs->values[i]);
	  readerTermDocs->values[i] = termDocs((*subReaders)[i]);
	  result = (*readerTermDocs)[i];
	}
	result->seek(term);

	return result;
}


MultiTermEnum::MultiTermEnum(ArrayBase<IndexReader*>* subReaders, const int32_t *starts, const Term* t){
//Func - Constructor
//       Opens all enumerations of all readers
//Pre  - readers != NULL and contains an array of IndexReader instances each responsible for
//       reading a single segment
//       subReaders->length >= 0 and represents the number of readers in the readers array
//       starts is an array of
//Post - An instance of has been created

//Pre  - if readers is NULL then subReaders->length must be 0 else if readers != NULL then subReaders->length > 0
//       s != NULL
//Post - The instance has been created

	CND_PRECONDITION(starts != NULL,"starts is NULL");

	//Temporary variables
	IndexReader*   reader    = NULL;
	TermEnum* termEnum  = NULL;
	SegmentMergeInfo* smi      = NULL;
	_docFreq = 0;
	_term = NULL;
	queue                      = _CLNEW SegmentMergeQueue(subReaders->length);

	CND_CONDITION (queue != NULL, "Could not allocate memory for queue");

	//iterate through all the readers
	for ( size_t i=0;i<subReaders->length;i++ ) {
		//Get the i-th reader
		reader = (*subReaders)[i];

		//Check if the enumeration must start from term t
		if (t != NULL) {
			//termEnum is an enumeration of terms starting at or after the named term t
			termEnum = reader->terms(t);
		}else{
			//termEnum is an enumeration of all the Terms and TermInfos in the set.
			termEnum = reader->terms();
		}

		//Instantiate an new SegmentMerginfo
		smi = _CLNEW SegmentMergeInfo(starts[i], termEnum, reader);

		// Note that in the call termEnum->getTerm(false) below false is required because
		// otherwise a reference is leaked. By passing false getTerm is
		// ordered to return an unowned reference instead. (Credits for DSR)
		if (t == NULL ? smi->next() : termEnum->term(false) != NULL){
			// initialize queue
			queue->put(smi);
		} else{
			//Close the SegmentMergeInfo
			smi->close();
			//And have it deleted
			_CLDELETE(smi);
		}
	}

	//Check if the queue has elements
	if (t != NULL && queue->size() > 0) {
		next();
	}
}

MultiTermEnum::~MultiTermEnum(){
//Func - Destructor
//Pre  - true
//Post - All the resource have been freed and the instance has been deleted

	//Close the enumeration
	close();

	//Delete the queue
	_CLDELETE(queue);
}

bool MultiTermEnum::next(){
//Func - Move the current term to the next in the set of enumerations
//Pre  - true
//Post - Returns true if term has been moved to the next in the set of enumerations
//       Returns false if this was not possible

	SegmentMergeInfo* top = queue->top();
	if (top == NULL) {
	    _CLDECDELETE(_term);
	    _term = NULL;
	    return false;
	}

	//The getTerm method requires the client programmer to indicate whether he
	// owns the returned reference, so we can discard ours
	// right away.
	_CLDECDELETE(_term);

	//Assign term the term of top and make sure the reference counter is increased
	_term = _CL_POINTER(top->term);
	_docFreq = 0;

	//Find the next term
	while (top != NULL && _term->compareTo(top->term) == 0) {
		//don't delete, this is the top
		queue->pop();
		// increment freq
		_docFreq += top->termEnum->docFreq();
		if (top->next()){
			// restore queue
			queue->put(top);
		}else{
			// done with a segment
			top->close();
			_CLDELETE(top);
		}
		top = queue->top();
	}

	return true;
}


Term* MultiTermEnum::term(bool pointer) {
  	if ( pointer )
    	return _CL_POINTER(_term);
    else
    	return _term;
}

int32_t MultiTermEnum::docFreq() const {
//Func - Returns the document frequency of the current term in the set
//Pre  - termInfo != NULL
//       next() must have been called once
//Post  - The document frequency of the current enumerated term has been returned

  return _docFreq;
}


void MultiTermEnum::close() {
//Func - Closes the set of enumerations in the queue
//Pre  - queue holds a valid reference to a SegmentMergeQueue
//Post - The queue has been closed all SegmentMergeInfo instance have been deleted by
//       the closing of the queue
//       term has been finalized and reset to NULL

	// Needed when this enumeration hasn't actually been exhausted yet
	_CLDECDELETE(_term);

	//Close the queue This will destroy all SegmentMergeInfo instances!
	queue->close();

}





MultiTermPositions::MultiTermPositions(ArrayBase<IndexReader*>* r, const int32_t* s){
//Func - Constructor
//Pre  - if r is NULL then rLen must be 0 else if r != NULL then rLen > 0
//       s != NULL
//Post - The instance has been created
  init(r,s);
}


TermDocs* MultiTermPositions::__asTermDocs(){
  return (TermDocs*) this;
}
TermPositions* MultiTermPositions::__asTermPositions(){
  return (TermPositions*) this;
}


TermDocs* MultiTermPositions::termDocs(IndexReader* reader) {
// Here in the MultiTermPositions class, we want this->current to always
// be a SegmentTermPositions rather than merely a SegmentTermDocs.
// To that end, we override the termDocs(IndexReader&) method to produce
// a SegmentTermPositions via the underlying reader's termPositions method
// rather merely producing a SegmentTermDocs via the reader's termDocs
// method.

	TermPositions* tp = reader->termPositions();
	TermDocs* ret = tp->__asTermDocs();

	CND_CONDITION(ret != NULL,
	    "Dynamic downcast in MultiTermPositions::termDocs from"
	    " TermPositions to TermDocs failed."
	  );
	return ret;
	}

int32_t MultiTermPositions::nextPosition() {
	//Func -
	//Pre  - current != NULL
	//Post -
	CND_PRECONDITION(current != NULL,"current is NULL");

	TermPositions* curAsTP = current->__asTermPositions();

	CND_CONDITION(curAsTP != NULL,
	    "Dynamic downcast in MultiTermPositions::nextPosition from"
	    " SegmentTermDocs to TermPositions failed."
	)
	return curAsTP->nextPosition();
}

int32_t MultiTermPositions::getPayloadLength() const{
  TermPositions* curAsTP = current->__asTermPositions();
  return curAsTP->getPayloadLength();
}

uint8_t* MultiTermPositions::getPayload(uint8_t* data){
  TermPositions* curAsTP = current->__asTermPositions();
  return curAsTP->getPayload(data);
}

bool MultiTermPositions::isPayloadAvailable() const{
  TermPositions* curAsTP = current->__asTermPositions();
  return curAsTP->isPayloadAvailable();
}

CL_NS_END
