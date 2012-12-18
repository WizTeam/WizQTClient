/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_SegmentHeader.h"
#include "CLucene/util/PriorityQueue.h"
#include "CLucene/util/Misc.h"
#include "IndexReader.h"
#include "IndexWriter.h"
#include "_SegmentMerger.h"
#include "_FieldsWriter.h"
#include "CLucene/document/Document.h"
#include <assert.h>
#include "CLucene/index/_IndexFileNames.h"
#include "_CompoundFile.h"
#include "_SkipListWriter.h"
#include "CLucene/document/FieldSelector.h"

CL_NS_USE(util)
CL_NS_USE(document)
CL_NS_USE(store)
CL_NS_DEF(index)

const uint8_t SegmentMerger::NORMS_HEADER[] = {'N','R','M', (uint8_t)-1};
const int SegmentMerger::NORMS_HEADER_length = 4;
int32_t SegmentMerger::MAX_RAW_MERGE_DOCS = 4192;

void SegmentMerger::init(){
  skipListWriter   = NULL;
  freqOutput       = NULL;
  proxOutput       = NULL;
  termInfosWriter  = NULL;
  queue            = NULL;
  fieldInfos       = NULL;
  checkAbort       = NULL;
  skipInterval     = 0;
}

SegmentMerger::SegmentMerger(IndexWriter* writer, const char* name, MergePolicy::OneMerge* merge){
//Func - Constructor
//Pre  - dir holds a valid reference to a Directory
//       name != NULL
//Post - Instance has been created

  CND_PRECONDITION(name != NULL, "name is NULL");

  this->init();
  this->directory		   = writer->getDirectory();
  this->segment        = name;
  if (merge != NULL)
    this->checkAbort = _CLNEW CheckAbort(merge, directory);
  this->termIndexInterval= writer->getTermIndexInterval();
  this->mergedDocs = 0;
  this->maxSkipLevels = 0;
}

SegmentMerger::~SegmentMerger(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed

	//Clear the readers set
	readers.clear();

	//Delete field Infos
	_CLDELETE(fieldInfos);
	//Close and destroy the IndexOutput to the Frequency File
	if (freqOutput != NULL){
		freqOutput->close();
		_CLDELETE(freqOutput);
	}
	//Close and destroy the IndexOutput to the Prox File
	if (proxOutput != NULL){
		proxOutput->close();
		_CLDELETE(proxOutput);
	}
	//Close and destroy the termInfosWriter
	if (termInfosWriter != NULL){
		termInfosWriter->close();
		_CLDELETE(termInfosWriter);
	}
	//Close and destroy the queue
	if (queue != NULL){
		queue->close();
		_CLDELETE(queue);
	}

  _CLDELETE(checkAbort);
  _CLDELETE(skipListWriter);

}

void SegmentMerger::add(IndexReader* reader) {
//Func - Adds a IndexReader to the set of readers
//Pre  - reader contains a valid reference to a IndexReader
//Post - The SegementReader reader has been added to the set of readers

    readers.push_back(reader);
}

IndexReader* SegmentMerger::segmentReader(const int32_t i) {
//Func - Returns a reference to the i-th IndexReader
//Pre  - 0 <= i < readers.size()
//Post - A reference to the i-th IndexReader has been returned

	CND_PRECONDITION(i >= 0, "i is a negative number");
    CND_PRECONDITION((size_t)i < readers.size(), "i is bigger than the number of IndexReader instances");

	//Retrieve the i-th IndexReader
    IndexReader* ret = readers[i];
    CND_CONDITION(ret != NULL,"No IndexReader found");

    return ret;
}

int32_t SegmentMerger::merge(bool mergeDocStores) {
  this->mergeDocStores = mergeDocStores;

  // NOTE: it's important to add calls to
  // checkAbort.work(...) if you make any changes to this
  // method that will spend alot of time.  The frequency
  // of this check impacts how long
  // IndexWriter.close(false) takes to actually stop the
  // threads.

  mergedDocs = mergeFields();

	mergeTerms();
	mergeNorms();

	if (mergeDocStores && fieldInfos->hasVectors())
		mergeVectors();

	return mergedDocs;
}

void SegmentMerger::closeReaders(){
  for (uint32_t i = 0; i < readers.size(); i++) {  // close readers
      IndexReader* reader = readers[i];
      reader->close();
  }
}

void SegmentMerger::createCompoundFile(const char* filename, std::vector<std::string>* files){
  CompoundFileWriter* cfsWriter = _CLNEW CompoundFileWriter(directory, filename, checkAbort);

  bool ownFiles = false;
  if ( files == NULL ){
    files = new vector<string>;
    files->reserve(IndexFileNames::COMPOUND_EXTENSIONS().length + 1);
    ownFiles = true;
  }

	// Basic files
  for (int32_t i = 0; i < IndexFileNames::COMPOUND_EXTENSIONS().length; i++) {
    const char* ext = IndexFileNames::COMPOUND_EXTENSIONS()[i];
    if (mergeDocStores || (strcmp(ext,IndexFileNames::FIELDS_EXTENSION) != 0 &&
        strcmp(ext,IndexFileNames::FIELDS_INDEX_EXTENSION) != 0 ) ){
		  files->push_back ( string(segment) + "." + ext );
    }
	}

    // Field norm files
	for (size_t i = 0; i < fieldInfos->size(); i++) {
		FieldInfo* fi = fieldInfos->fieldInfo(i);
		if (fi->isIndexed && !fi->omitNorms) {
      files->push_back ( segment + "." + IndexFileNames::NORMS_EXTENSION );
      break;
		}
	}

  // Vector files
  if ( mergeDocStores && fieldInfos->hasVectors()) {
    for (int32_t i = 0; i < IndexFileNames::VECTOR_EXTENSIONS().length; i++) {
	      files->push_back ( segment + "." + IndexFileNames::VECTOR_EXTENSIONS()[i] );
      }
  }

	// Now merge all added files
	for ( size_t i=0;i<files->size();i++ ){
		cfsWriter->addFile( (*files)[i].c_str());
	}

	// Perform the merge
	cfsWriter->close();
	_CLDELETE(cfsWriter);
  if ( ownFiles ) delete files;
}

void SegmentMerger::addIndexed(IndexReader* reader, FieldInfos* fieldInfos, StringArrayWithDeletor& names,
	  bool storeTermVectors, bool storePositionWithTermVector,
    bool storeOffsetWithTermVector, bool storePayloads){

	StringArrayWithDeletor::const_iterator itr = names.begin();
	while ( itr != names.end() ){
		fieldInfos->add(*itr, true,
			storeTermVectors, storePositionWithTermVector,
			storeOffsetWithTermVector, !reader->hasNorms(*itr), storePayloads);

		++itr;
	}
}


// for merging we don't want to compress/uncompress the data, so to tell the FieldsReader that we're
// in  merge mode, we use this FieldSelector
class FieldSelectorMerge: public FieldSelector{
public:
  FieldSelectorResult accept(const TCHAR* /*fieldName*/) const{
    return FieldSelector::LOAD_FOR_MERGE;
  }
};


int32_t SegmentMerger::mergeFields() {
//Func - Merge the fields of all segments
//Pre  - true
//Post - The field infos and field values of all segments have been merged.

  if (!mergeDocStores) {
    // When we are not merging by doc stores, that means
    // all segments were written as part of a single
    // autoCommit=false IndexWriter session, so their field
    // name -> number mapping are the same.  So, we start
    // with the fieldInfos of the last segment in this
    // case, to keep that numbering.
    assert(readers[readers.size()-1]->instanceOf(SegmentReader::getClassName()));
    assert(false);//check last...and remove if correct...
    SegmentReader* sr = (SegmentReader*)readers[readers.size()-1];
    fieldInfos = sr->fieldInfos()->clone();
  } else {
    //Create a new FieldInfos
    fieldInfos = _CLNEW FieldInfos();		  // merge field names
  }
	//Condition check to see if fieldInfos points to a valid instance
	CND_CONDITION(fieldInfos != NULL,"Memory allocation for fieldInfos failed");

	IndexReader* reader = NULL;

  //Iterate through all readers
  for (uint32_t i = 0; i < readers.size(); i++){
	  //get the i-th reader
	  reader = readers[i];
	  //Condition check to see if reader points to a valid instance
	  CND_CONDITION(reader != NULL,"No IndexReader found");

    if (reader->instanceOf(SegmentReader::getClassName())) {
      SegmentReader* segmentReader = (SegmentReader*) reader;
      for (size_t j = 0; j < segmentReader->getFieldInfos()->size(); j++) {
        FieldInfo* fi = segmentReader->getFieldInfos()->fieldInfo(j);
        fieldInfos->add(fi->name, fi->isIndexed, fi->storeTermVector,
          fi->storePositionWithTermVector, fi->storeOffsetWithTermVector,
          !reader->hasNorms(fi->name), fi->storePayloads);
      }
    } else {
	    StringArrayWithDeletor tmp;

	    tmp.clear(); reader->getFieldNames(IndexReader::TERMVECTOR_WITH_POSITION_OFFSET, tmp);
	    addIndexed(reader, fieldInfos, tmp, true, true, true, false);

	    tmp.clear(); reader->getFieldNames(IndexReader::TERMVECTOR_WITH_POSITION, tmp);
	    addIndexed(reader, fieldInfos, tmp, true, true, false, false);

	    tmp.clear(); reader->getFieldNames(IndexReader::TERMVECTOR_WITH_OFFSET, tmp);
	    addIndexed(reader, fieldInfos, tmp, true, false, true, false);

	    tmp.clear(); reader->getFieldNames(IndexReader::TERMVECTOR, tmp);
	    addIndexed(reader, fieldInfos, tmp, true, false, false, false);

	    tmp.clear(); reader->getFieldNames(IndexReader::STORES_PAYLOADS, tmp);
	    addIndexed(reader, fieldInfos, tmp, false, false, false, true);

	    tmp.clear(); reader->getFieldNames(IndexReader::INDEXED, tmp);
	    addIndexed(reader, fieldInfos, tmp, false, false, false, false);

	    tmp.clear(); reader->getFieldNames(IndexReader::UNINDEXED, tmp);
	    if ( tmp.size() > 0 ){
		    TCHAR** arr = _CL_NEWARRAY(TCHAR*,tmp.size()+1);
            tmp.toArray_nullTerminated(arr);
		    fieldInfos->add((const TCHAR**)arr, false);
		    _CLDELETE_ARRAY(arr); //no need to delete the contents, since tmp is responsible for it
	    }
    }
  }

  //Write the new FieldInfos file to the directory
  fieldInfos->write(directory, Misc::segmentname(segment.c_str(),".fnm").c_str() );

	int32_t docCount = 0;

  if (mergeDocStores) {

    // If the i'th reader is a SegmentReader and has
    // identical fieldName -> number mapping, then this
    // array will be non-NULL at position i:
    ValueArray<SegmentReader*> matchingSegmentReaders(readers.size());

    // If this reader is a SegmentReader, and all of its
    // field name -> number mappings match the "merged"
    // FieldInfos, then we can do a bulk copy of the
    // stored fields:
    for (size_t i = 0; i < readers.size(); i++) {
      IndexReader* reader = readers[i];
      if (reader->instanceOf(SegmentReader::getClassName())) {
        SegmentReader* segmentReader = (SegmentReader*) reader;
        bool same = true;
        FieldInfos* segmentFieldInfos = segmentReader->getFieldInfos();
        for (size_t j = 0; same && j < segmentFieldInfos->size(); j++)
          same = _tcscmp(fieldInfos->fieldName(j), segmentFieldInfos->fieldName(j)) == 0;
        if (same) {
          matchingSegmentReaders.values[i] = segmentReader;
        }
      }
    }

    // Used for bulk-reading raw bytes for stored fields
    ValueArray<int32_t> rawDocLengths(MAX_RAW_MERGE_DOCS);

    // merge field values
    FieldsWriter fieldsWriter(directory, segment.c_str(), fieldInfos);

    try {
      for (size_t i = 0; i < readers.size(); i++) {
        IndexReader* reader = readers[i];
        SegmentReader* matchingSegmentReader = matchingSegmentReaders[i];
        FieldsReader* matchingFieldsReader;
        if (matchingSegmentReader != NULL)
          matchingFieldsReader = matchingSegmentReader->getFieldsReader();
        else
          matchingFieldsReader = NULL;
        const int32_t maxDoc = reader->maxDoc();
        Document doc;
        FieldSelectorMerge fieldSelectorMerge;
        for (int32_t j = 0; j < maxDoc;) {
          if (!reader->isDeleted(j)) { // skip deleted docs
            if (matchingSegmentReader != NULL) {
              // We can optimize this case (doing a bulk
              // byte copy) since the field numbers are
              // identical
              int32_t start = j;
              int32_t numDocs = 0;
              do {
                j++;
                numDocs++;
              } while(j < maxDoc && !matchingSegmentReader->isDeleted(j) && numDocs < MAX_RAW_MERGE_DOCS);

              IndexInput* stream = matchingFieldsReader->rawDocs(rawDocLengths.values, start, numDocs);
              fieldsWriter.addRawDocuments(stream, rawDocLengths.values, numDocs);
              docCount += numDocs;
              if (checkAbort != NULL)
                checkAbort->work(300*numDocs);
            } else {
              doc.clear();
              reader->document(j, doc, &fieldSelectorMerge);
              fieldsWriter.addDocument(&doc);
              j++;
              docCount++;
              if (checkAbort != NULL)
                checkAbort->work(300);
            }
          } else
            j++;
        }
      }
    } _CLFINALLY (
      fieldsWriter.close();
    )

    CND_PRECONDITION (docCount*8 == directory->fileLength( (segment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION).c_str() ),
    (string("after mergeFields: fdx size mismatch: ") + Misc::toString(docCount) + " docs vs " + Misc::toString(directory->fileLength( (segment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION).c_str() )) + " length in bytes of " + segment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION).c_str() );

  } else{
    // If we are skipping the doc stores, that means there
    // are no deletions in any of these segments, so we
    // just sum numDocs() of each segment to get total docCount
    for (size_t i = 0; i < readers.size(); i++)
      docCount += readers[i]->numDocs();
  }
  return docCount;
}


void SegmentMerger::mergeVectors(){
	TermVectorsWriter* termVectorsWriter =
		_CLNEW TermVectorsWriter(directory, segment.c_str(), fieldInfos);

	try {
		for (uint32_t r = 0; r < readers.size(); r++) {
			IndexReader* reader = readers[r];
			int32_t maxDoc = reader->maxDoc();
			for (int32_t docNum = 0; docNum < maxDoc; docNum++) {
				// skip deleted docs
				if (reader->isDeleted(docNum))
					continue;

				ArrayBase<TermFreqVector*>* tmp = reader->getTermFreqVectors(docNum);
//        if ( tmp != NULL ){
					termVectorsWriter->addAllDocVectors(tmp);
				  _CLLDELETE(tmp);
//        }
          if (checkAbort != NULL)
            checkAbort->work(300);
			}
		}
	}_CLFINALLY(
    if ( termVectorsWriter != NULL ){
      termVectorsWriter->close();
      _CLDELETE(termVectorsWriter);
    }
  );

  CND_PRECONDITION(4+mergedDocs*8 == directory->fileLength( (segment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION).c_str() ),
    (string("after mergeVectors: tvx size mismatch: ") + Misc::toString(mergedDocs) + " docs vs " + Misc::toString(directory->fileLength( (segment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION).c_str() )) + " length in bytes of " + segment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION).c_str() )

}


void SegmentMerger::mergeTerms() {
//Func - Merge the terms of all segments
//Pre  - fieldInfos != NULL
//Post - The terms of all segments have been merged

	CND_PRECONDITION(fieldInfos != NULL, "fieldInfos is NULL");

    try{
      //Open an IndexOutput to the new Frequency File
      freqOutput = directory->createOutput( Misc::segmentname(segment.c_str(),".frq").c_str() );

      //Open an IndexOutput to the new Prox File
      proxOutput = directory->createOutput( Misc::segmentname(segment.c_str(),".prx").c_str() );

      //Instantiate  a new termInfosWriter which will write in directory
      //for the segment name segment using the new merged fieldInfos
      termInfosWriter = _CLNEW TermInfosWriter(directory, segment.c_str(), fieldInfos, termIndexInterval);

      //Condition check to see if termInfosWriter points to a valid instance
      CND_CONDITION(termInfosWriter != NULL,"Memory allocation for termInfosWriter failed")	;

      skipInterval = termInfosWriter->skipInterval;
      maxSkipLevels = termInfosWriter->maxSkipLevels;
      skipListWriter = _CLNEW DefaultSkipListWriter(skipInterval, maxSkipLevels, mergedDocs, freqOutput, proxOutput);
      queue = _CLNEW SegmentMergeQueue(readers.size());

      //And merge the Term Infos
      mergeTermInfos();
    }_CLFINALLY(
      if ( freqOutput != NULL ){
        freqOutput->close();
        _CLDELETE(freqOutput);
      }
      if ( proxOutput != NULL ){
        proxOutput->close();
        _CLDELETE(proxOutput);
      }
      if ( termInfosWriter != NULL ){
        termInfosWriter->close();
        _CLDELETE(termInfosWriter);
      }
      if ( queue != NULL ){
        queue->close();
        _CLDELETE(queue);
      }
    );
}

void SegmentMerger::mergeTermInfos(){
//Func - Merges all TermInfos into a single segment
//Pre  - true
//Post - All TermInfos have been merged into a single segment

    //Condition check to see if queue points to a valid instance
    CND_CONDITION(queue != NULL, "Memory allocation for queue failed")	;

	//base is the id of the first document in a segment
    int32_t base = 0;

    IndexReader* reader = NULL;
	SegmentMergeInfo* smi = NULL;

	//iterate through all the readers
    for (uint32_t i = 0; i < readers.size(); i++) {
		  //Get the i-th reader
      reader = readers[i];

      //Condition check to see if reader points to a valid instance
      CND_CONDITION(reader != NULL, "No IndexReader found");

      //Get the term enumeration of the reader
      TermEnum* termEnum = reader->terms();
      //Instantiate a new SegmentMerginfo for the current reader and enumeration
      smi = _CLNEW SegmentMergeInfo(base, termEnum, reader);

      //Condition check to see if smi points to a valid instance
      CND_CONDITION(smi != NULL, "Memory allocation for smi failed")	;

      //Increase the base by the number of documents that have not been marked deleted
      //so base will contain a new value for the first document of the next iteration
      base += reader->numDocs();
		  //Get the next current term
		  if (smi->next()){
        //Store the SegmentMergeInfo smi with the initialized SegmentTermEnum TermEnum
        //into the queue
        queue->put(smi);
      }else{
        //Apparently the end of the TermEnum of the SegmentTerm has been reached so
        //close the SegmentMergeInfo smi
        smi->close();
        //And destroy the instance and set smi to NULL (It will be used later in this method)
        _CLDELETE(smi);
      }
    }

	  //Instantiate an array of SegmentMergeInfo instances called match
    SegmentMergeInfo** match = _CL_NEWARRAY(SegmentMergeInfo*,readers.size());

    //Condition check to see if match points to a valid instance
    CND_CONDITION(match != NULL, "Memory allocation for match failed")	;

    SegmentMergeInfo* top = NULL;

    //As long as there are SegmentMergeInfo instances stored in the queue
    while (queue->size() > 0) {
      int32_t matchSize = 0;

      // pop matching terms

      //Pop the first SegmentMergeInfo from the queue
      match[matchSize++] = queue->pop();
      //Get the Term of match[0]
      Term* term = match[0]->term;

      //Condition check to see if term points to a valid instance
      CND_CONDITION(term != NULL,"term is NULL")	;

      //Get the current top of the queue
      top = queue->top();

      //For each SegmentMergInfo still in the queue
		  //Check if term matches the term of the SegmentMergeInfo instances in the queue
      while (top != NULL && term->equals(top->term) ){
        //A match has been found so add the matching SegmentMergeInfo to the match array
        match[matchSize++] = queue->pop();
        //Get the next SegmentMergeInfo
        top = queue->top();
      }
      int32_t df = mergeTermInfo(match, matchSize);		  // add new TermInfo
      if (checkAbort != NULL)
        checkAbort->work(df/3.0);

      //Restore the SegmentTermInfo instances in the match array back into the queue
      while (matchSize > 0){
        smi = match[--matchSize];

        //Condition check to see if smi points to a valid instance
        CND_CONDITION(smi != NULL,"smi is NULL")	;

			  //Move to the next term in the enumeration of SegmentMergeInfo smi
			  if (smi->next()){
          //There still are some terms so restore smi in the queue
          queue->put(smi);

        }else{
		      //Done with a segment
		      //No terms anymore so close this SegmentMergeInfo instance
          smi->close();
          _CLDELETE( smi );
        }
      }
    }
    _CLDELETE_ARRAY(match);
}

int32_t SegmentMerger::mergeTermInfo( SegmentMergeInfo** smis, int32_t n){
//Func - Merge the TermInfo of a term found in one or more segments.
//Pre  - smis != NULL and it contains segments that are positioned at the same term.
//       n is equal to the number of SegmentMergeInfo instances in smis
//       freqOutput != NULL
//       proxOutput != NULL
//Post - The TermInfo of a term has been merged

	CND_PRECONDITION(smis != NULL, "smis is NULL");
	CND_PRECONDITION(freqOutput != NULL, "freqOutput is NULL");
	CND_PRECONDITION(proxOutput != NULL, "proxOutput is NULL");

  //Get the file pointer of the IndexOutput to the Frequency File
  int64_t freqPointer = freqOutput->getFilePointer();
  //Get the file pointer of the IndexOutput to the Prox File
  int64_t proxPointer = proxOutput->getFilePointer();

  //Process postings from multiple segments all positioned on the same term.
  int32_t df = appendPostings(smis, n);

  int64_t skipPointer = skipListWriter->writeSkip(freqOutput);

  //df contains the number of documents across all segments where this term was found
  if (df > 0) {
    //add an entry to the dictionary with pointers to prox and freq files
    termInfo.set(df, freqPointer, proxPointer, (int32_t)(skipPointer - freqPointer));
    //Precondition check for to be sure that the reference to
    //smis[0]->term will be valid
    CND_PRECONDITION(smis[0]->term != NULL, "smis[0]->term is NULL");
    //Write a new TermInfo
    termInfosWriter->add(smis[0]->term, &termInfo);
  }
  return df;
}


int32_t SegmentMerger::appendPostings(SegmentMergeInfo** smis, int32_t n){
//Func - Process postings from multiple segments all positioned on the
//       same term. Writes out merged entries into freqOutput and
//       the proxOutput streams.
//Pre  - smis != NULL and it contains segments that are positioned at the same term.
//       n is equal to the number of SegmentMergeInfo instances in smis
//       freqOutput != NULL
//       proxOutput != NULL
//Post - Returns number of documents across all segments where this term was found

  CND_PRECONDITION(smis != NULL, "smis is NULL");
  CND_PRECONDITION(freqOutput != NULL, "freqOutput is NULL");
  CND_PRECONDITION(proxOutput != NULL, "proxOutput is NULL");

  int32_t lastDoc = 0;
  int32_t df = 0;       //Document Counter

  skipListWriter->resetSkip();
  bool storePayloads = fieldInfos->fieldInfo(smis[0]->term->field())->storePayloads;
  int32_t lastPayloadLength = -1;   // ensures that we write the first length

  SegmentMergeInfo* smi = NULL;

  //Iterate through all SegmentMergeInfo instances in smis
  for ( int32_t i=0;i<n;i++ ){
    //Get the i-th SegmentMergeInfo
    smi = smis[i];

    //Condition check to see if smi points to a valid instance
    CND_PRECONDITION(smi!=NULL,"	 is NULL");

    //Get the term positions
    TermPositions* postings = smi->getPositions();
    assert(postings != NULL);
    //Get the base of this segment
    int32_t base = smi->base;
    //Get the docMap so we can see which documents have been deleted
    int32_t* docMap = smi->getDocMap();
    //Seek the termpost
    postings->seek(smi->termEnum);
    while (postings->next()) {
      int32_t doc = postings->doc();
      //Check if there are deletions
      if (docMap != NULL)
        doc = docMap[doc]; // map around deletions
      doc += base;                              // convert to merged space

      //Condition check to see doc is eaqual to or bigger than lastDoc
      if (doc < 0 || (df > 0 && doc <= lastDoc))
        _CLTHROWA(CL_ERR_CorruptIndex, (string("docs out of order (") + Misc::toString(doc) +
            " <= " + Misc::toString(lastDoc) + " )").c_str());

      //Increase the total frequency over all segments
      df++;

      if ((df % skipInterval) == 0) {
        skipListWriter->setSkipData(lastDoc, storePayloads, lastPayloadLength);
        skipListWriter->bufferSkip(df);
      }

      //Calculate a new docCode
      //use low bit to flag freq=1
      int32_t docCode = (doc - lastDoc) << 1;
      lastDoc = doc;

      //Get the frequency of the Term
      int32_t freq = postings->freq();
      if (freq == 1){
        //write doc & freq=1
        freqOutput->writeVInt(docCode | 1);
      }else{
        //write doc
        freqOutput->writeVInt(docCode);
        //write frequency in doc
        freqOutput->writeVInt(freq);
      }

      /** See {@link DocumentWriter#writePostings(Posting[], String)} for
      *  documentation about the encoding of positions and payloads
      */
      int32_t lastPosition = 0;
      // write position deltas
      for (int32_t j = 0; j < freq; j++) {
        //Get the next position
        int32_t position = postings->nextPosition();
        int32_t delta = position - lastPosition;
        if (storePayloads) {
          size_t payloadLength = postings->getPayloadLength();
          if (payloadLength == lastPayloadLength) {
            proxOutput->writeVInt(delta * 2);
          } else {
            proxOutput->writeVInt(delta * 2 + 1);
            proxOutput->writeVInt(payloadLength);
            lastPayloadLength = payloadLength;
          }
          if (payloadLength > 0) {
          	if ( payloadBuffer.length < payloadLength ){
              payloadBuffer.resize(payloadLength);
            }
            postings->getPayload(payloadBuffer.values);
            proxOutput->writeBytes(payloadBuffer.values, payloadLength);
          }
        } else {
          proxOutput->writeVInt(delta);
        }
        lastPosition = position;
      }
    }
  }

  //Return total number of documents across all segments where term was found
  return df;
}

void SegmentMerger::mergeNorms() {
//Func - Merges the norms for all fields
//Pre  - fieldInfos != NULL
//Post - The norms for all fields have been merged
  ValueArray<uint8_t> normBuffer;
	IndexOutput*  output  = NULL;
  try {

    CND_PRECONDITION(fieldInfos != NULL, "fieldInfos is NULL");

	  IndexReader* reader  = NULL;

	  //iterate through all the Field Infos instances
    for (size_t i = 0; i < fieldInfos->size(); i++) {
      //Get the i-th FieldInfo
      FieldInfo* fi = fieldInfos->fieldInfo(i);
      //Is this Field indexed?
      if (fi->isIndexed && !fi->omitNorms){
        //Instantiate  an IndexOutput to that norm file
        if (output == NULL) {
          output = directory->createOutput( (segment + "." + IndexFileNames::NORMS_EXTENSION).c_str() );
          output->writeBytes(NORMS_HEADER,NORMS_HEADER_length);
        }

        //Condition check to see if output points to a valid instance
        CND_CONDITION(output != NULL, "No Outputstream retrieved");

		    //Iterate through all IndexReaders
        for (uint32_t j = 0; j < readers.size(); j++) {
			    //Get the i-th IndexReader
			    reader = readers[j];

			    //Condition check to see if reader points to a valid instance
			    CND_CONDITION(reader != NULL, "No reader found");

			    //Get the total number of documents including the documents that have been marked deleted
			    size_t maxDoc = reader->maxDoc();

			    //Get an IndexInput to the norm file for this field in this segment
          if ( normBuffer.length < maxDoc ){
            normBuffer.resize(maxDoc);
            memset(normBuffer.values,0,sizeof(uint8_t) * maxDoc);
			    }
          reader->norms(fi->name, normBuffer.values);

          if (!reader->hasDeletions()) {
            //optimized case for segments without deleted docs
            output->writeBytes(normBuffer.values, maxDoc);
          } else {
            // this segment has deleted docs, so we have to
            // check for every doc if it is deleted or not

				    for(size_t k = 0; k < maxDoc; k++) {
					    //Check if document k is deleted
					    if (!reader->isDeleted(k)){
						    //write the new norm
						    output->writeByte(normBuffer[k]);
					    }
				    }
			    }
          if (checkAbort != NULL)
            checkAbort->work(maxDoc);
		    }
	    }
	  }
  }_CLFINALLY(
    if ( output != NULL ){
      output->close();
      _CLDELETE(output);
    }
  );
}


SegmentMerger::CheckAbort::CheckAbort(MergePolicy::OneMerge* merge, Directory* dir) {
  this->merge = merge;
  this->dir = dir;
  this->workCount = 0;
}

void SegmentMerger::CheckAbort::work(float_t units){
  workCount += units;
  if (workCount >= 10000.0) {
    merge->checkAborted(dir);
    workCount = 0;
  }
}
CL_NS_END
