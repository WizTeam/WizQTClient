/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexOutput.h"
#include "CLucene/store/_RAMDirectory.h"
#include "CLucene/util/Array.h"
#include "CLucene/util/_Arrays.h"
#include "CLucene/util/Misc.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/document/Field.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/document/Document.h"
#include "_FieldInfos.h"
#include "_TermInfo.h"
#include "_CompoundFile.h"
#include "IndexWriter.h"
#include "_IndexFileNames.h"
#include "_FieldsWriter.h"
#include "Term.h"
#include "_Term.h"
#include "_TermInfo.h"
#include "_TermVector.h"
#include "_TermInfosWriter.h"
#include "_SkipListWriter.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/search/Similarity.h"
#include "_TermInfosWriter.h"
#include "_FieldsWriter.h"
#include "_DocumentsWriter.h"
#include <assert.h>
#include <algorithm>
#include <iostream>

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(analysis)
CL_NS_USE(document)
CL_NS_USE(search)
CL_NS_DEF(index)


const int32_t DocumentsWriter::MAX_THREAD_STATE = 5;
const uint8_t DocumentsWriter::defaultNorm = Similarity::encodeNorm(1.0f);
const int32_t DocumentsWriter::nextLevelArray[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 9};
const int32_t DocumentsWriter::levelSizeArray[10] = {5, 14, 20, 30, 40, 40, 80, 80, 120, 200};
const int32_t DocumentsWriter::POSTING_NUM_BYTE = OBJECT_HEADER_BYTES + 9*INT_NUM_BYTE + 5*POINTER_NUM_BYTE;

const int32_t DocumentsWriter::BYTE_BLOCK_SHIFT = 15;
const int32_t DocumentsWriter::BYTE_BLOCK_SIZE = (int32_t)pow(2.0, BYTE_BLOCK_SHIFT);
const int32_t DocumentsWriter::BYTE_BLOCK_MASK = BYTE_BLOCK_SIZE - 1;
const int32_t DocumentsWriter::BYTE_BLOCK_NOT_MASK = ~BYTE_BLOCK_MASK;

const int32_t DocumentsWriter::CHAR_BLOCK_SHIFT = 14;
const int32_t DocumentsWriter::CHAR_BLOCK_SIZE = (int32_t)pow(2.0, CHAR_BLOCK_SHIFT);
const int32_t DocumentsWriter::CHAR_BLOCK_MASK = CHAR_BLOCK_SIZE - 1;

int32_t DocumentsWriter::OBJECT_HEADER_BYTES = 8;
int32_t DocumentsWriter::OBJECT_POINTER_BYTES = 4;    // TODO: should be 8 on 64-bit platform
int32_t DocumentsWriter::BYTES_PER_CHAR = 2;
int32_t DocumentsWriter::BYTES_PER_INT = 4;

const int32_t DocumentsWriter::POINTER_NUM_BYTE = 4;
const int32_t DocumentsWriter::INT_NUM_BYTE = 4;
const int32_t DocumentsWriter::CHAR_NUM_BYTE = 2; //TODO: adjust for c++...

const int32_t DocumentsWriter::MAX_TERM_LENGTH = DocumentsWriter::CHAR_BLOCK_SIZE-1;



AbortException::AbortException(CLuceneError& _err, DocumentsWriter* docWriter):
  err(_err)
{
  docWriter->setAborting();
}

DocumentsWriter::DocumentsWriter(CL_NS(store)::Directory* directory, IndexWriter* writer):
  bufferedDeleteTerms(_CLNEW TermNumMapType(true, true)),
  freeCharBlocks(FreeCharBlocksType(true)),
  freeByteBlocks(FreeByteBlocksType(true)),
  waitingThreadStates( CL_NS(util)::ValueArray<ThreadState*>(MAX_THREAD_STATE) )
{
  numBytesAlloc = 0;
  numBytesUsed = 0;
  this->directory = directory;
  this->writer = writer;
  this->hasNorms = this->bufferIsFull = false;
  fieldInfos = _CLNEW FieldInfos();

	maxBufferedDeleteTerms = IndexWriter::DEFAULT_MAX_BUFFERED_DELETE_TERMS;
	ramBufferSize = (int64_t) (IndexWriter::DEFAULT_RAM_BUFFER_SIZE_MB*1024*1024);
	maxBufferedDocs = IndexWriter::DEFAULT_MAX_BUFFERED_DOCS;

	numBufferedDeleteTerms = 0;
  copyByteBuffer = _CL_NEWARRAY(uint8_t, 4096);
  *copyByteBuffer = 0;

  this->closed = this->flushPending = false;
  _files = NULL;
  _abortedFiles = NULL;
  skipListWriter = NULL;
  infoStream = NULL;
  fieldsWriter = NULL;
  tvx = tvf = tvd = NULL;
  postingsFreeCountDW = postingsAllocCountDW = numWaiting = pauseThreads = abortCount = 0;
  docStoreOffset = nextDocID = numDocsInRAM = numDocsInStore = nextWriteDocID = 0;
}
DocumentsWriter::~DocumentsWriter(){
  _CLLDELETE(bufferedDeleteTerms);
  _CLLDELETE(skipListWriter);
  _CLDELETE_LARRAY(copyByteBuffer);
  _CLLDELETE(_files);
  _CLLDELETE(fieldInfos);

  for(size_t i=0;i<threadStates.length;i++) {
    _CLLDELETE(threadStates.values[i]);
  }

  // Make sure unused posting slots aren't attempted delete on
  if (this->postingsFreeListDW.values){
      if (this->postingsFreeCountDW < this->postingsFreeListDW.length) {
          memset(this->postingsFreeListDW.values + this->postingsFreeCountDW
              , 0
              , sizeof(Posting*));
      }
      postingsFreeListDW.deleteUntilNULL();
  }
}

void DocumentsWriter::setInfoStream(std::ostream* infoStream) {
  this->infoStream = infoStream;
}

void DocumentsWriter::setRAMBufferSizeMB(float_t mb) {
  if ( (int32_t)mb == IndexWriter::DISABLE_AUTO_FLUSH) {
    ramBufferSize = IndexWriter::DISABLE_AUTO_FLUSH;
  } else {
    ramBufferSize = (int64_t) (mb*1024*1024);
  }
}

float_t DocumentsWriter::getRAMBufferSizeMB() {
  if (ramBufferSize == IndexWriter::DISABLE_AUTO_FLUSH) {
    return (float_t)ramBufferSize;
  } else {
    return ramBufferSize/1024.0/1024.0;
  }
}

void DocumentsWriter::setMaxBufferedDocs(int32_t count) {
  maxBufferedDocs = count;
}

int32_t DocumentsWriter::getMaxBufferedDocs() {
  return maxBufferedDocs;
}

std::string DocumentsWriter::getSegment() {
  return segment;
}

int32_t DocumentsWriter::getNumDocsInRAM() {
  return numDocsInRAM;
}

const std::string& DocumentsWriter::getDocStoreSegment() {
  return docStoreSegment;
}

int32_t DocumentsWriter::getDocStoreOffset() {
  return docStoreOffset;
}

std::string DocumentsWriter::closeDocStore() {

  assert (allThreadsIdle());

  const std::vector<string>& flushedFiles = files();

  if (infoStream != NULL)
    (*infoStream) << string("\ncloseDocStore: ") << Misc::toString((int32_t)flushedFiles.size()) << string(" files to flush to segment ") <<
    docStoreSegment << string(" numDocs=") << Misc::toString(numDocsInStore) << string("\n");

  if (flushedFiles.size() > 0) {
    _CLDELETE(_files);

    if (tvx != NULL) {
      // At least one doc in this run had term vectors enabled
      assert ( !docStoreSegment.empty());
      tvx->close();
      _CLDELETE(tvx);
      tvf->close();
      _CLDELETE(tvf);
      tvd->close();
      _CLDELETE(tvd);

      assert ( 4+numDocsInStore*8 == directory->fileLength( (docStoreSegment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION).c_str()) ); // "after flush: tvx size mismatch: " + numDocsInStore + " docs vs " + directory->fileLength(docStoreSegment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION) + " length in bytes of " + docStoreSegment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION;
    }

    if (fieldsWriter != NULL) {
      assert (!docStoreSegment.empty());
      fieldsWriter->close();
      _CLDELETE(fieldsWriter);

      assert(numDocsInStore*8 == directory->fileLength( (docStoreSegment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION).c_str() ) );// "after flush: fdx size mismatch: " + numDocsInStore + " docs vs " + directory->fileLength(docStoreSegment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION) + " length in bytes of " + docStoreSegment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION;
    }

    std::string s = docStoreSegment;
    docStoreSegment.clear();
    docStoreOffset = 0;
    numDocsInStore = 0;
    return s;
  } else {
    return "";
  }
}

const std::vector<string>* DocumentsWriter::abortedFiles() {
  return _abortedFiles;
}

const std::vector<std::string>& DocumentsWriter::files() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (_files != NULL)
    return *_files;

  _files = _CLNEW std::vector<string>;

  // Stored fields:
  if (fieldsWriter != NULL) {
    assert ( !docStoreSegment.empty());
    _files->push_back(docStoreSegment + "." + IndexFileNames::FIELDS_EXTENSION);
    _files->push_back(docStoreSegment + "." + IndexFileNames::FIELDS_INDEX_EXTENSION);
  }

  // Vectors:
  if (tvx != NULL) {
    assert ( !docStoreSegment.empty());
    _files->push_back(docStoreSegment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION);
    _files->push_back(docStoreSegment + "." + IndexFileNames::VECTORS_FIELDS_EXTENSION);
    _files->push_back(docStoreSegment + "." + IndexFileNames::VECTORS_DOCUMENTS_EXTENSION);
  }

  return *_files;
}

void DocumentsWriter::setAborting() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  abortCount++;
}

void DocumentsWriter::abort(AbortException* ae) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  // Anywhere that throws an AbortException must first
  // mark aborting to make sure while the exception is
  // unwinding the un-synchronized stack, no thread grabs
  // the corrupt ThreadState that hit the aborting
  // exception:
  assert (ae == NULL || abortCount>0);

  try {

    if (infoStream != NULL)
      (*infoStream) << string("docWriter: now abort\n");

    // Forcefully remove waiting ThreadStates from line
    for(int32_t i=0;i<numWaiting;i++)
      waitingThreadStates[i]->isIdle = true;
    numWaiting = 0;

    // Wait for all other threads to finish with DocumentsWriter:
    pauseAllThreads();

    assert (0 == numWaiting);

    try {
      bufferedDeleteTerms->clear();
      bufferedDeleteDocIDs.clear();
      numBufferedDeleteTerms = 0;

      try {
		    const std::vector<string>& __abortedFiles = files();
		    _abortedFiles = _CLNEW std::vector<string>;
		    for ( std::vector<string>::const_iterator itr = __abortedFiles.begin();
			    itr != __abortedFiles.end(); itr ++ ){
			    _abortedFiles->push_back(*itr);
		    }
      } catch (...) {
		    _CLDELETE(_abortedFiles);
      }

	    docStoreSegment.clear();
      numDocsInStore = 0;
      docStoreOffset = 0;
      _CLDELETE(_files);

      // Clear vectors & fields from ThreadStates
      for(size_t i=0;i<threadStates.length;i++) {
        ThreadState* state = threadStates[i];
        state->tvfLocal->reset();
        state->fdtLocal->reset();
        if (state->localFieldsWriter != NULL) {
          try {
            state->localFieldsWriter->close();
          } catch (...) {
          }
          _CLDELETE(state->localFieldsWriter);
        }
      }

      // Reset vectors writer
      if (tvx != NULL) {
        try {
          tvx->close();
        } catch (...) {
        }
        _CLDELETE(tvx);
      }
      if (tvd != NULL) {
        try {
          tvd->close();
        } catch (...) {
        }
        _CLDELETE(tvd);
      }
      if (tvf != NULL) {
        try {
          tvf->close();
        } catch (...) {
        }
        _CLDELETE(tvf);
      }

      // Reset fields writer
      if (fieldsWriter != NULL) {
        try {
          fieldsWriter->close();
        } catch (...) {
        }
        _CLDELETE(fieldsWriter);
      }

      // Discard pending norms:
      const int32_t numField = fieldInfos->size();
      for (int32_t i=0;i<numField;i++) {
        FieldInfo* fi = fieldInfos->fieldInfo(i);
        if (fi->isIndexed && !fi->omitNorms) {
          BufferedNorms* n = norms[i];
          if (n != NULL)
            try {
              n->reset();
            } catch (...) {
            }
        }
      }

      // Reset all postings data
      resetPostingsData();

    } _CLFINALLY (
      resumeAllThreads();
  )

    // If we have a root cause exception, re-throw it now:
    if (ae != NULL) {
      CLuceneError& t = ae->err;
	    throw t;
    }
  } _CLFINALLY (
    if (ae != NULL)
      abortCount--;
    CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
  )
}

void DocumentsWriter::resetPostingsData() {
  // All ThreadStates should be idle when we are called
  assert ( allThreadsIdle() );
  threadBindings.clear();
  segment.erase();
  numDocsInRAM = 0;
  nextDocID = 0;
  nextWriteDocID = 0;
  _CLDELETE(_files);
  balanceRAM();
  bufferIsFull = false;
  flushPending = false;
  for(size_t i=0;i<threadStates.length;i++) {
    threadStates[i]->numThreads = 0;
    threadStates[i]->resetPostings();
  }
  numBytesUsed = 0;
}

// Returns true if an abort is in progress
bool DocumentsWriter::pauseAllThreads() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  pauseThreads++;
  while(!allThreadsIdle()) {
    CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)
  }
  return abortCount > 0;
}

void DocumentsWriter::resumeAllThreads() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  pauseThreads--;
  assert ( pauseThreads >= 0 );
  if (0 == pauseThreads){
      CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
  }
}

bool DocumentsWriter::allThreadsIdle() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  for(size_t i=0;i<threadStates.length;i++)
    if (!threadStates[i]->isIdle)
      return false;
  return true;
}

int32_t DocumentsWriter::flush(bool _closeDocStore) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  assert ( allThreadsIdle() );

  if (segment.empty()){
      // In case we are asked to flush an empty segment
      segment = writer->newSegmentName();
  }

  newFiles.clear();

  docStoreOffset = numDocsInStore;

  int32_t docCount;

  assert ( numDocsInRAM > 0 );

  if (infoStream != NULL)
    (*infoStream) << string("\nflush postings as segment ") << segment << string(" numDocs=") << Misc::toString(numDocsInRAM) << string("\n");

  bool success = false;

  try {

    if (_closeDocStore) {
      assert ( !docStoreSegment.empty());
	    assert ( docStoreSegment.compare(segment) == 0 );
	    const std::vector<string>& tmp = files();
	    for (std::vector<string>::const_iterator itr = tmp.begin();
		    itr != tmp.end(); itr++ )
		    newFiles.push_back(*itr);
      closeDocStore();
    }

    fieldInfos->write(directory, (segment + ".fnm").c_str() );

    docCount = numDocsInRAM;

    writeSegment(newFiles); //write new files directly...

    success = true;

  } _CLFINALLY(
    if (!success)
      abort(NULL);
  )

  return docCount;
}

void DocumentsWriter::createCompoundFile(const std::string& segment)
{
  CompoundFileWriter* cfsWriter = _CLNEW CompoundFileWriter(directory, (segment + "." + IndexFileNames::COMPOUND_FILE_EXTENSION).c_str());
  for (std::vector<string>::const_iterator itr = newFiles.begin();
		itr != newFiles.end(); itr ++ )
    cfsWriter->addFile( (*itr).c_str() );

  // Perform the merge
  cfsWriter->close();
  _CLDELETE(cfsWriter);
}

bool DocumentsWriter::setFlushPending() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (flushPending)
    return false;
  else {
    flushPending = true;
    return true;
  }
}

void DocumentsWriter::clearFlushPending() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  flushPending = false;
}

void DocumentsWriter::writeNorms(const std::string& segmentName, int32_t totalNumDoc) {
  IndexOutput* normsOut = directory->createOutput( (segmentName + "." + IndexFileNames::NORMS_EXTENSION).c_str() );

  try {
	  normsOut->writeBytes(SegmentMerger::NORMS_HEADER, SegmentMerger::NORMS_HEADER_length);

    const int32_t numField = fieldInfos->size();

    for (int32_t fieldIdx=0;fieldIdx<numField;fieldIdx++) {
      FieldInfo* fi = fieldInfos->fieldInfo(fieldIdx);
      if (fi->isIndexed && !fi->omitNorms) {
        BufferedNorms* n = norms[fieldIdx];
        int64_t v;
        if (n == NULL)
          v = 0;
        else {
          v = n->out.getFilePointer();
          n->out.writeTo(normsOut);
          n->reset();
        }
        if (v < totalNumDoc)
          fillBytes(normsOut, defaultNorm, (int32_t) (totalNumDoc-v));
      }
    }
  } _CLFINALLY (
    normsOut->close();
    _CLDELETE(normsOut);
  )
}

void DocumentsWriter::writeSegment(std::vector<std::string>& flushedFiles) {

  assert ( allThreadsIdle() );

  assert ( nextDocID == numDocsInRAM );

  const std::string segmentName = segment;

  TermInfosWriter* termsOut = _CLNEW TermInfosWriter(directory, segmentName.c_str(), fieldInfos,
                                                 writer->getTermIndexInterval());

  IndexOutput* freqOut = directory->createOutput( (segmentName + ".frq").c_str() );
  IndexOutput* proxOut = directory->createOutput( (segmentName + ".prx").c_str() );

  // Gather all FieldData's that have postings, across all
  // ThreadStates
  std::vector<ThreadState::FieldData*> allFields;
  assert ( allThreadsIdle() );
  for(size_t i=0;i<threadStates.length;i++) {
    ThreadState* state = threadStates[i];
    state->trimFields();
    const int32_t numFields = state->numAllFieldData;
    for(int32_t j=0;j<numFields;j++) {
      ThreadState::FieldData* fp = state->allFieldDataArray[j];
      if (fp->numPostings > 0)
		    allFields.push_back(fp);
    }
  }

  // Sort by field name
  std::sort(allFields.begin(),allFields.end(),ThreadState::FieldData::sort);
  const int32_t numAllFields = allFields.size();

  skipListWriter = _CLNEW DefaultSkipListWriter(termsOut->skipInterval,
                                             termsOut->maxSkipLevels,
                                             numDocsInRAM, freqOut, proxOut);

  int32_t start = 0;
  while(start < numAllFields) {

    const TCHAR* fieldName = allFields[start]->fieldInfo->name;

    int32_t end = start+1;
    while(end < numAllFields && _tcscmp(allFields[end]->fieldInfo->name, fieldName)==0 )
      end++;

    ValueArray<ThreadState::FieldData*> fields(end-start);
    for(int32_t i=start;i<end;i++)
      fields.values[i-start] = allFields[i];

    // If this field has postings then add them to the
    // segment
    appendPostings(&fields, termsOut, freqOut, proxOut);

    for(size_t i=0;i<fields.length;i++)
      fields[i]->resetPostingArrays();

    start = end;
  }

  freqOut->close();
  _CLDELETE(freqOut);
  proxOut->close();
  _CLDELETE(proxOut);
  termsOut->close();
  _CLDELETE(termsOut);
  _CLDELETE(skipListWriter);

  // Record all files we have flushed
  flushedFiles.push_back(segmentFileName(IndexFileNames::FIELD_INFOS_EXTENSION));
  flushedFiles.push_back(segmentFileName(IndexFileNames::FREQ_EXTENSION));
  flushedFiles.push_back(segmentFileName(IndexFileNames::PROX_EXTENSION));
  flushedFiles.push_back(segmentFileName(IndexFileNames::TERMS_EXTENSION));
  flushedFiles.push_back(segmentFileName(IndexFileNames::TERMS_INDEX_EXTENSION));

  if (hasNorms) {
    writeNorms(segmentName, numDocsInRAM);
    flushedFiles.push_back(segmentFileName(IndexFileNames::NORMS_EXTENSION));
  }

  if (infoStream != NULL) {
    const int64_t newSegmentSize = segmentSize(segmentName);

    (*infoStream) << string("  oldRAMSize=") << Misc::toString(numBytesUsed) <<
				string(" newFlushedSize=") << Misc::toString(newSegmentSize) <<
        string(" docs/MB=") << Misc::toString((float_t)(numDocsInRAM/(newSegmentSize/1024.0/1024.0))) <<
        string(" new/old=") << Misc::toString((float_t)(100.0*newSegmentSize/numBytesUsed)) << string("%\n");
  }

  resetPostingsData();

  nextDocID = 0;
  nextWriteDocID = 0;
  numDocsInRAM = 0;
  _CLDELETE(_files);

  // Maybe downsize this->postingsFreeListDW array
  if (this->postingsFreeListDW.length > 1.5*this->postingsFreeCountDW) {
    int32_t newSize = this->postingsFreeListDW.length;
    while(newSize > 1.25*this->postingsFreeCountDW) {
      newSize = (int32_t) (newSize*0.8);
    }
    this->postingsFreeListDW.resize(newSize);
  }

}

std::string DocumentsWriter::segmentFileName(const char* extension) {
  return segmentFileName( string(extension) );
}
std::string DocumentsWriter::segmentFileName(const std::string& extension) {
  return segment + "." + extension;
}

int32_t DocumentsWriter::compareText(const TCHAR* text1, const TCHAR* text2) {
int32_t pos1=0;
int32_t pos2=0;
  while(true) {
    const TCHAR c1 = text1[pos1++];
    const TCHAR c2 = text2[pos2++];
    if (c1 < c2)
      if (CLUCENE_END_OF_WORD == c2)
        return 1;
      else
        return -1;
    else if (c2 < c1)
      if (CLUCENE_END_OF_WORD == c1)
        return -1;
      else
        return 1;
    else if (CLUCENE_END_OF_WORD == c1)
      return 0;
  }
}


void DocumentsWriter::appendPostings(ArrayBase<ThreadState::FieldData*>* fields,
                    TermInfosWriter* termsOut,
                    IndexOutput* freqOut,
                    IndexOutput* proxOut) {

  const int32_t fieldNumber = (*fields)[0]->fieldInfo->number;
  int32_t numFields = fields->length;

  ObjectArray<FieldMergeState> mergeStatesData(numFields);
  ValueArray<FieldMergeState*> mergeStates(numFields);

  for(int32_t i=0;i<numFields;i++) {
    FieldMergeState* fms = mergeStatesData.values[i] = _CLNEW FieldMergeState();
    fms->field = (*fields)[i];
    fms->postings = fms->field->sortPostings();

    assert ( fms->field->fieldInfo == (*fields)[0]->fieldInfo );

    // Should always be true
    bool result = fms->nextTerm();
    assert (result);
  }
  memcpy(mergeStates.values,mergeStatesData.values,sizeof(FieldMergeState*) * numFields);

  const int32_t skipInterval = termsOut->skipInterval;
  currentFieldStorePayloads = (*fields)[0]->fieldInfo->storePayloads;

  ValueArray<FieldMergeState*> termStates(numFields);

  while(numFields > 0) {

    // Get the next term to merge
    termStates.values[0] = mergeStates[0];
    int32_t numToMerge = 1;

    for(int32_t i=1;i<numFields;i++) {
      const TCHAR* text = mergeStates[i]->text;
      const int32_t textOffset = mergeStates[i]->textOffset;
      const int32_t cmp = compareText(text+  textOffset, termStates[0]->text + termStates[0]->textOffset);

      if (cmp < 0) {
        termStates.values[0] = mergeStates[i];
        numToMerge = 1;
      } else if (cmp == 0)
        termStates.values[numToMerge++] = mergeStates[i];
    }

    int32_t df = 0;
    int32_t lastPayloadLength = -1;

    int32_t lastDoc = 0;

    const TCHAR* start = termStates[0]->text + termStates[0]->textOffset;
    const TCHAR* pos = start;
    while(*pos != CLUCENE_END_OF_WORD)
      pos++;

    int64_t freqPointer = freqOut->getFilePointer();
    int64_t proxPointer = proxOut->getFilePointer();

    skipListWriter->resetSkip();

    // Now termStates has numToMerge FieldMergeStates
    // which all share the same term.  Now we must
    // interleave the docID streams.
    while(numToMerge > 0) {

      if ((++df % skipInterval) == 0) {
        skipListWriter->setSkipData(lastDoc, currentFieldStorePayloads, lastPayloadLength);
        skipListWriter->bufferSkip(df);
      }

      FieldMergeState* minState = termStates[0];
      for(int32_t i=1;i<numToMerge;i++)
        if (termStates[i]->docID < minState->docID)
          minState = termStates[i];

      const int32_t doc = minState->docID;
      const int32_t termDocFreq = minState->termFreq;

      assert (doc < numDocsInRAM);
      assert ( doc > lastDoc || df == 1 );

      const int32_t newDocCode = (doc-lastDoc)<<1;
      lastDoc = doc;

      ByteSliceReader& prox = minState->prox;

      // Carefully copy over the prox + payload info,
      // changing the format to match Lucene's segment
      // format.
      for(int32_t j=0;j<termDocFreq;j++) {
        const int32_t code = prox.readVInt();
        if (currentFieldStorePayloads) {
          int32_t payloadLength;
          if ((code & 1) != 0) {
            // This position has a payload
            payloadLength = prox.readVInt();
          } else
            payloadLength = 0;
          if (payloadLength != lastPayloadLength) {
            proxOut->writeVInt(code|1);
            proxOut->writeVInt(payloadLength);
            lastPayloadLength = payloadLength;
          } else
            proxOut->writeVInt(code & (~1));
          if (payloadLength > 0)
            copyBytes(&prox, proxOut, payloadLength);
        } else {
          assert ( 0 == (code & 1) );
          proxOut->writeVInt(code>>1);
        }
      }

      if (1 == termDocFreq) {
        freqOut->writeVInt(newDocCode|1);
      } else {
        freqOut->writeVInt(newDocCode);
        freqOut->writeVInt(termDocFreq);
      }

      if (!minState->nextDoc()) {

        // Remove from termStates
        int32_t upto = 0;
        for(int32_t i=0;i<numToMerge;i++)
          if (termStates[i] != minState)
            termStates.values[upto++] = termStates[i];
        numToMerge--;
        assert (upto == numToMerge);

        // Advance this state to the next term

        if (!minState->nextTerm()) {
          // OK, no more terms, so remove from mergeStates
          // as well
          upto = 0;
          for(int32_t i=0;i<numFields;i++)
            if (mergeStates[i] != minState){
              mergeStates.values[upto++] = mergeStates[i];
            }
          numFields--;
          assert (upto == numFields);
        }
      }
    }

    assert (df > 0);

    // Done merging this term

    int64_t skipPointer = skipListWriter->writeSkip(freqOut);

    // Write term
    termInfo.set(df, freqPointer, proxPointer, (int32_t) (skipPointer - freqPointer));
    termsOut->add(fieldNumber, start, pos-start, &termInfo);
  }
}


void DocumentsWriter::close() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  closed = true;
  CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
}

DocumentsWriter::ThreadState* DocumentsWriter::getThreadState(Document* doc, Term* delTerm) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  // First, find a thread state.  If this thread already
  // has affinity to a specific ThreadState, use that one
  // again.
  ThreadState* state = NULL;
  if ( threadBindings.find(_LUCENE_CURRTHREADID) == threadBindings.end() ){
    // First time this thread has called us since last flush
    ThreadState* minThreadState = NULL;
    for(size_t i=0;i<threadStates.length;i++) {
      ThreadState* ts = threadStates[i];
      if (minThreadState == NULL || ts->numThreads < minThreadState->numThreads)
        minThreadState = ts;
    }
    if (minThreadState != NULL && (minThreadState->numThreads == 0 || threadStates.length == MAX_THREAD_STATE)) {
      state = minThreadState;
      state->numThreads++;
    } else {
      // Just create a new "private" thread state
      threadStates.resize(1+threadStates.length);
      //fill the new position
      state = threadStates.values[threadStates.length-1] = _CLNEW ThreadState(this);
    }
    threadBindings.put(_LUCENE_CURRTHREADID, state);
  }else{
    state = threadBindings[_LUCENE_CURRTHREADID];
  }

  // Next, wait until my thread state is idle (in case
  // it's shared with other threads) and for threads to
  // not be paused nor a flush pending:
  while(!closed && (!state->isIdle || pauseThreads != 0 || flushPending || abortCount > 0))
    CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)

  if (closed)
    _CLTHROWA(CL_ERR_AlreadyClosed, "this IndexWriter is closed");

  if (segment.empty())
    segment = writer->newSegmentName();

  state->isIdle = false;

  try {
    bool success = false;
    try {
      state->init(doc, nextDocID);
      if (delTerm != NULL) {
        addDeleteTerm(delTerm, state->docID);
        state->doFlushAfter = timeToFlushDeletes();
      }
      // Only increment nextDocID & numDocsInRAM on successful init
      nextDocID++;
      numDocsInRAM++;

      // We must at this point commit to flushing to ensure we
      // always get N docs when we flush by doc count, even if
      // > 1 thread is adding documents:
      if (!flushPending && maxBufferedDocs != IndexWriter::DISABLE_AUTO_FLUSH
          && numDocsInRAM >= maxBufferedDocs) {
        flushPending = true;
        state->doFlushAfter = true;
      }

      success = true;
    } _CLFINALLY (
      if (!success) {
        // Forcefully idle this ThreadState:
        state->isIdle = true;
        CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
        if (state->doFlushAfter) {
          state->doFlushAfter = false;
          flushPending = false;
        }
      }
    )
  } catch (AbortException& ae) {
    abort(&ae);
  }

  return state;
}

bool DocumentsWriter::addDocument(Document* doc, Analyzer* analyzer){
  return updateDocument(doc, analyzer, NULL);
}

bool DocumentsWriter::updateDocument(Term* t, Document* doc, Analyzer* analyzer){
  return updateDocument(doc, analyzer, t);
}

bool DocumentsWriter::updateDocument(Document* doc, Analyzer* analyzer, Term* delTerm) {

  // This call is synchronized but fast
  ThreadState* state = getThreadState(doc, delTerm);
  try {
    bool success = false;
    try {
      try {
        // This call is not synchronized and does all the work
        state->processDocument(analyzer);
      } _CLFINALLY (
        // This call is synchronized but fast
        finishDocument(state);
	    )
      success = true;
    } _CLFINALLY (
      if (!success) {
				SCOPED_LOCK_MUTEX(THIS_LOCK)

        // If this thread state had decided to flush, we
        // must clear it so another thread can flush
        if (state->doFlushAfter) {
          state->doFlushAfter = false;
          flushPending = false;
          CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
        }

        // Immediately mark this document as deleted
        // since likely it was partially added.  This
        // keeps indexing as "all or none" (atomic) when
        // adding a document:
        addDeleteDocID(state->docID);
	    }
    )
  } catch (AbortException& ae) {
    abort(&ae);
  }

  return state->doFlushAfter || timeToFlushDeletes();
}

int32_t DocumentsWriter::getNumBufferedDeleteTerms() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return numBufferedDeleteTerms;
}

const DocumentsWriter::TermNumMapType& DocumentsWriter::getBufferedDeleteTerms() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return *bufferedDeleteTerms;
}

const std::vector<int32_t>* DocumentsWriter::getBufferedDeleteDocIDs() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return &bufferedDeleteDocIDs;
}

// Reset buffered deletes.
void DocumentsWriter::clearBufferedDeletes() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  DocumentsWriter::TermNumMapType::iterator term = bufferedDeleteTerms->begin();
  while ( term != bufferedDeleteTerms->end() ){
    Term* t = term->first;
    _CLDELETE(term->second);
    bufferedDeleteTerms->erase(term);
    _CLDECDELETE(t);
    term = bufferedDeleteTerms->begin();
  }
  bufferedDeleteDocIDs.clear();
  numBufferedDeleteTerms = 0;
  if (numBytesUsed > 0)
    resetPostingsData();
}

bool DocumentsWriter::bufferDeleteTerms(const ArrayBase<Term*>* terms) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  while(pauseThreads != 0 || flushPending){
    CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)
  }
  for (size_t i = 0; i < terms->length; i++)
    addDeleteTerm((*terms)[i], numDocsInRAM);
  return timeToFlushDeletes();
}

bool DocumentsWriter::bufferDeleteTerm(Term* term) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  while(pauseThreads != 0 || flushPending){
    CONDITION_WAIT(THIS_LOCK, THIS_WAIT_CONDITION)
  }
  addDeleteTerm(term, numDocsInRAM);
  return timeToFlushDeletes();
}

bool DocumentsWriter::timeToFlushDeletes() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return (bufferIsFull
          || (maxBufferedDeleteTerms != IndexWriter::DISABLE_AUTO_FLUSH
              && numBufferedDeleteTerms >= maxBufferedDeleteTerms))
         && setFlushPending();
}

void DocumentsWriter::setMaxBufferedDeleteTerms(int32_t _maxBufferedDeleteTerms) {
  this->maxBufferedDeleteTerms = _maxBufferedDeleteTerms;
}

int32_t DocumentsWriter::getMaxBufferedDeleteTerms() {
  return maxBufferedDeleteTerms;
}

bool DocumentsWriter::hasDeletes() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  return bufferedDeleteTerms->size() > 0 || bufferedDeleteDocIDs.size() > 0;
}

// Buffer a term in bufferedDeleteTerms, which records the
// current number of documents buffered in ram so that the
// delete term will be applied to those documents as well
// as the disk segments.
void DocumentsWriter::addDeleteTerm(Term* term, int32_t docCount) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  Num* num = bufferedDeleteTerms->get(term);
  if (num == NULL) {
    bufferedDeleteTerms->put(_CL_POINTER(term), new Num(docCount));
    // This is coarse approximation of actual bytes used:
    numBytesUsed += ( _tcslen(term->field()) + term->textLength()) * BYTES_PER_CHAR
        + 4 + 5 * OBJECT_HEADER_BYTES + 5 * OBJECT_POINTER_BYTES;
    if (ramBufferSize != IndexWriter::DISABLE_AUTO_FLUSH
        && numBytesUsed > ramBufferSize) {
      bufferIsFull = true;
    }
  } else {
    num->setNum(docCount);
  }
  numBufferedDeleteTerms++;
}

void DocumentsWriter::addDeleteDocID(int32_t docId) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  bufferedDeleteDocIDs.push_back(docId);
  numBytesUsed += OBJECT_HEADER_BYTES + BYTES_PER_INT + OBJECT_POINTER_BYTES;
}

void DocumentsWriter::finishDocument(ThreadState* state) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  if (abortCount > 0) {
    // Forcefully idle this threadstate -- its state will
    // be reset by abort()
    state->isIdle = true;
    CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)
    return;
  }

  // Now write the indexed document to the real files.
  if (nextWriteDocID == state->docID) {
    // It's my turn, so write everything now:
    nextWriteDocID++;
    state->writeDocument();
    state->isIdle = true;
    CONDITION_NOTIFYALL(THIS_WAIT_CONDITION)

    // If any states were waiting on me, sweep through and
    // flush those that are enabled by my write.
    if (numWaiting > 0) {
      bool any = true;
      while(any) {
        any = false;
        for(int32_t i=0;i<numWaiting;) {
          ThreadState* s = waitingThreadStates[i];
          if (s->docID == nextWriteDocID) {
            s->writeDocument();
            s->isIdle = true;
            nextWriteDocID++;
            any = true;
            if (numWaiting > i+1)
              // Swap in the last waiting state to fill in
              // the hole we just created.  It's important
              // to do this as-we-go and not at the end of
              // the loop, because if we hit an aborting
              // exception in one of the s.writeDocument
              // calls (above), it leaves this array in an
              // inconsistent state:
              waitingThreadStates.values[i] = waitingThreadStates[numWaiting-1];
            numWaiting--;
          } else {
            assert (!s->isIdle);
            i++;
          }
        }
      }
    }
  } else {
    // Another thread got a docID before me, but, it
    // hasn't finished its processing.  So add myself to
    // the line but don't hold up this thread.
    waitingThreadStates.values[numWaiting++] = state;
  }
}

int64_t DocumentsWriter::getRAMUsed() {
  return numBytesUsed;
}

void DocumentsWriter::fillBytes(IndexOutput* out, uint8_t b, int32_t numBytes) {
  for(int32_t i=0;i<numBytes;i++)
    out->writeByte(b);
}

void DocumentsWriter::copyBytes(IndexInput* srcIn, IndexOutput* destIn, int64_t numBytes) {
  // TODO: we could do this more efficiently (save a copy)
  // because it's always from a ByteSliceReader ->
  // IndexOutput
  while(numBytes > 0) {
    int32_t chunk;
    if (numBytes > 4096)
      chunk = 4096;
    else
      chunk = (int32_t) numBytes;
    srcIn->readBytes(copyByteBuffer, chunk);
    destIn->writeBytes(copyByteBuffer, chunk);
    numBytes -= chunk;
  }
}


int64_t DocumentsWriter::segmentSize(const std::string& segmentName) {
  assert (infoStream != NULL);

  int64_t size = directory->fileLength( (segmentName + ".tii").c_str() ) +
    directory->fileLength( (segmentName + ".tis").c_str() ) +
    directory->fileLength( (segmentName + ".frq").c_str() ) +
    directory->fileLength( (segmentName + ".prx").c_str() );

  const std::string normFileName = segmentName + ".nrm";
  if (directory->fileExists(normFileName.c_str()))
    size += directory->fileLength(normFileName.c_str());

  return size;
}

void DocumentsWriter::getPostings(ValueArray<Posting*>& postings) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  numBytesUsed += postings.length * POSTING_NUM_BYTE;
  int32_t numToCopy;
  if (this->postingsFreeCountDW < postings.length)
    numToCopy = this->postingsFreeCountDW;
  else
    numToCopy = postings.length;

  const int32_t start = this->postingsFreeCountDW-numToCopy;
  if ( numToCopy > 0 ){
    memcpy(postings.values, this->postingsFreeListDW.values+start, sizeof(Posting*)*numToCopy);
  }
  this->postingsFreeCountDW -= numToCopy;

  // Directly allocate the remainder if any
  if (numToCopy < postings.length) {
    const int32_t extra = postings.length - numToCopy;
    const int32_t newPostingsAllocCount = this->postingsAllocCountDW + extra;
    if (newPostingsAllocCount > this->postingsFreeListDW.length)
      this->postingsFreeListDW.resize((int32_t) (1.25 * newPostingsAllocCount));

    balanceRAM();
    for(size_t i=numToCopy;i<postings.length;i++) {
      postings.values[i] = _CLNEW Posting();
      numBytesAlloc += POSTING_NUM_BYTE;
      this->postingsAllocCountDW++;
    }
  }
}

void DocumentsWriter::recyclePostings(ValueArray<Posting*>& postings, int32_t numPostings) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  // Move all Postings from this ThreadState back to our
  // free list.  We pre-allocated this array while we were
  // creating Postings to make sure it's large enough
  assert (this->postingsFreeCountDW + numPostings <= this->postingsFreeListDW.length);
  if ( numPostings > 0 )
    memcpy (this->postingsFreeListDW.values + this->postingsFreeCountDW, postings.values, numPostings * sizeof(Posting*));
  this->postingsFreeCountDW += numPostings;
}

uint8_t* DocumentsWriter::getByteBlock(bool trackAllocations) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  const int32_t size = freeByteBlocks.size();
  uint8_t* b;
  if (0 == size) {
    numBytesAlloc += BYTE_BLOCK_SIZE;
    balanceRAM();
    b = _CL_NEWARRAY(uint8_t, BYTE_BLOCK_SIZE);
    memset(b,0,sizeof(uint8_t) * BYTE_BLOCK_SIZE);
  } else {
    b = *freeByteBlocks.begin();
    freeByteBlocks.remove(freeByteBlocks.begin(),true);
  }
  if (trackAllocations)
    numBytesUsed += BYTE_BLOCK_SIZE;
  return b;
}

void DocumentsWriter::recycleBlocks(ArrayBase<uint8_t*>& blocks, int32_t start, int32_t end) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)
  for(int32_t i=start;i<end;i++){
    freeByteBlocks.push_back(blocks[i]);
    blocks[i] = NULL;
  }
}

TCHAR* DocumentsWriter::getCharBlock() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  const int32_t size = freeCharBlocks.size();
  TCHAR* c;
  if (0 == size) {
    numBytesAlloc += CHAR_BLOCK_SIZE * CHAR_NUM_BYTE;
    balanceRAM();
    c = _CL_NEWARRAY(TCHAR, CHAR_BLOCK_SIZE);
    memset(c,0,sizeof(TCHAR) * CHAR_BLOCK_SIZE);
  } else{
    c = *freeCharBlocks.begin();
    freeCharBlocks.remove(freeCharBlocks.begin(),true);
  }
  numBytesUsed += CHAR_BLOCK_SIZE * CHAR_NUM_BYTE;
  return c;
}

void DocumentsWriter::recycleBlocks(ArrayBase<TCHAR*>& blocks, int32_t start, int32_t numBlocks) {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  for(int32_t i=start;i<numBlocks;i++){
    freeCharBlocks.push_back(blocks[i]);
    blocks.values[i] = NULL;
  }
}

std::string DocumentsWriter::toMB(int64_t v) {
  char buf[40];
  cl_sprintf(buf,40, "%0.2f", v/1024.0/1024.0);
  return string(buf);
}

void DocumentsWriter::balanceRAM() {
	SCOPED_LOCK_MUTEX(THIS_LOCK)

  if (ramBufferSize == IndexWriter::DISABLE_AUTO_FLUSH || bufferIsFull)
    return;

  // We free our allocations if we've allocated 5% over
  // our allowed RAM buffer
  const int64_t freeTrigger = (int64_t) (1.05 * ramBufferSize);
  const int64_t freeLevel = (int64_t) (0.95 * ramBufferSize);

  // We flush when we've used our target usage
  const int64_t flushTrigger = (int64_t) ramBufferSize;

  if (numBytesAlloc > freeTrigger) {
    if (infoStream != NULL)
      (*infoStream) << string("  RAM: now balance allocations: usedMB=") << toMB(numBytesUsed) +
                         string(" vs trigger=") << toMB(flushTrigger) <<
                         string(" allocMB=") << toMB(numBytesAlloc) <<
                         string(" vs trigger=") << toMB(freeTrigger) <<
                         string(" postingsFree=") << toMB(this->postingsFreeCountDW*POSTING_NUM_BYTE) <<
                         string(" byteBlockFree=") << toMB(freeByteBlocks.size()*BYTE_BLOCK_SIZE) <<
                         string(" charBlockFree=") << toMB(freeCharBlocks.size()*CHAR_BLOCK_SIZE*CHAR_NUM_BYTE) << string("\n");

    // When we've crossed 100% of our target Postings
    // RAM usage, try to free up until we're back down
    // to 95%
    const int64_t startBytesAlloc = numBytesAlloc;

    const int32_t postingsFreeChunk = (int32_t) (BYTE_BLOCK_SIZE / POSTING_NUM_BYTE);

    int32_t iter = 0;

    // We free equally from each pool in 64 KB
    // chunks until we are below our threshold
    // (freeLevel)

    while(numBytesAlloc > freeLevel) {
      if (0 == freeByteBlocks.size() && 0 == freeCharBlocks.size() && 0 == this->postingsFreeCountDW) {
        // Nothing else to free -- must flush now.
        bufferIsFull = true;
        if (infoStream != NULL)
          (*infoStream) << string("    nothing to free; now set bufferIsFull\n");
        break;
      }

      if ((0 == iter % 3) && freeByteBlocks.size() > 0) {
        freeByteBlocks.remove(freeByteBlocks.size()-1);
        numBytesAlloc -= BYTE_BLOCK_SIZE;
      }

      if ((1 == iter % 3) && freeCharBlocks.size() > 0) {
        freeCharBlocks.remove(freeCharBlocks.size()-1);
        numBytesAlloc -= CHAR_BLOCK_SIZE * CHAR_NUM_BYTE;
      }

      if ((2 == iter % 3) && this->postingsFreeCountDW > 0) {
        int32_t numToFree;
        if (this->postingsFreeCountDW >= postingsFreeChunk)
          numToFree = postingsFreeChunk;
        else
          numToFree = this->postingsFreeCountDW;
        for ( size_t i = this->postingsFreeCountDW-numToFree;i< this->postingsFreeCountDW; i++ ){
          _CLDELETE(this->postingsFreeListDW.values[i]);
        }
        this->postingsFreeCountDW -= numToFree;
        this->postingsAllocCountDW -= numToFree;
        numBytesAlloc -= numToFree * POSTING_NUM_BYTE;
      }

      iter++;
    }

    if (infoStream != NULL){
      (*infoStream) << "    after free: freedMB=" + Misc::toString((float_t)((startBytesAlloc-numBytesAlloc)/1024.0/1024.0)) +
        " usedMB=" + Misc::toString((float_t)(numBytesUsed/1024.0/1024.0)) +
        " allocMB=" + Misc::toString((float_t)(numBytesAlloc/1024.0/1024.0)) << string("\n");
    }

  } else {
    // If we have not crossed the 100% mark, but have
    // crossed the 95% mark of RAM we are actually
    // using, go ahead and flush.  This prevents
    // over-allocating and then freeing, with every
    // flush.
    if (numBytesUsed > flushTrigger) {
	    if (infoStream != NULL){
        (*infoStream) << string("  RAM: now flush @ usedMB=") << Misc::toString((float_t)(numBytesUsed/1024.0/1024.0)) <<
            string(" allocMB=") << Misc::toString((float_t)(numBytesAlloc/1024.0/1024.0)) <<
            string(" triggerMB=") << Misc::toString((float_t)(flushTrigger/1024.0/1024.0)) << string("\n");
	    }

      bufferIsFull = true;
    }
  }
}


DocumentsWriter::BufferedNorms::BufferedNorms(){
  this->upto = 0;
}
void DocumentsWriter::BufferedNorms::add(float_t norm){
  uint8_t b = Similarity::encodeNorm(norm);
  out.writeByte(b);
  upto++;
}
void DocumentsWriter::BufferedNorms::reset(){
  out.reset();
  upto = 0;
}
void DocumentsWriter::BufferedNorms::fill(int32_t docID){
  // Must now fill in docs that didn't have this
  // field.  Note that this is how norms can consume
  // tremendous storage when the docs have widely
  // varying different fields, because we are not
  // storing the norms sparsely (see LUCENE-830)
  if (upto < docID) {
    fillBytes(&out, defaultNorm, docID-upto);
    upto = docID;
  }
}



DocumentsWriter::FieldMergeState::FieldMergeState(){
  field = NULL;
  postings = NULL;
  p = NULL;
  text = NULL;
  textOffset = 0;
  postingUpto = -1;
  docID = 0;
  termFreq = 0;
}
DocumentsWriter::FieldMergeState::~FieldMergeState(){
}
bool DocumentsWriter::FieldMergeState::nextTerm(){
  postingUpto++;
  if (postingUpto == field->numPostings)
    return false;

  p = (*postings)[postingUpto];
  docID = 0;

  text = field->threadState->charPool->buffers[p->textStart >> CHAR_BLOCK_SHIFT];
  textOffset = p->textStart & CHAR_BLOCK_MASK;

  if (p->freqUpto > p->freqStart)
    freq.init(field->threadState->postingsPool, p->freqStart, p->freqUpto);
  else
    freq.bufferOffset = freq.upto = freq.endIndex = 0;

  prox.init(field->threadState->postingsPool, p->proxStart, p->proxUpto);

  // Should always be true
  bool result = nextDoc();
  assert (result);

  return true;
}

bool DocumentsWriter::FieldMergeState::nextDoc() {
  if (freq.bufferOffset + freq.upto == freq.endIndex) {
    if (p->lastDocCode != -1) {
      // Return last doc
      docID = p->lastDocID;
      termFreq = p->docFreq;
      p->lastDocCode = -1;
      return true;
    } else
      // EOF
      return false;
  }

  const uint32_t code = (uint32_t)freq.readVInt();
  docID += code >> 1; //unsigned shift
  if ((code & 1) != 0)
    termFreq = 1;
  else
    termFreq = freq.readVInt();

  return true;
}


DocumentsWriter::ByteSliceReader::ByteSliceReader(){
  pool = NULL;
  bufferUpto = 0;
  buffer = 0;
  limit = 0;
  level = 0;
  upto = 0;
  bufferOffset = 0;
  endIndex = 0;
}
DocumentsWriter::ByteSliceReader::~ByteSliceReader(){
}
const char* DocumentsWriter::ByteSliceReader::getDirectoryType() const{
  return "";
}
const char* DocumentsWriter::ByteSliceReader::getObjectName() const{
  return getClassName();
}
const char* DocumentsWriter::ByteSliceReader::getClassName(){
  return "DocumentsWriter::ByteSliceReader";
}
IndexInput* DocumentsWriter::ByteSliceReader::clone() const{
  _CLTHROWA(CL_ERR_UnsupportedOperation, "Not implemented");
}
void DocumentsWriter::ByteSliceReader::init(ByteBlockPool* _pool, int32_t _startIndex, int32_t _endIndex) {

  assert (_endIndex-_startIndex > 0);

  level = 0;
  this->pool = _pool;
  this->endIndex = _endIndex;

  bufferUpto = _startIndex / BYTE_BLOCK_SIZE;
  bufferOffset = bufferUpto * BYTE_BLOCK_SIZE;
  buffer = pool->buffers[bufferUpto];
  upto = _startIndex & BYTE_BLOCK_MASK;

  const int32_t firstSize = levelSizeArray[0];

  if (_startIndex+firstSize >= endIndex) {
    // There is only this one slice to read
    limit = endIndex & BYTE_BLOCK_MASK;
  } else
    limit = upto+firstSize-4;
}

uint8_t DocumentsWriter::ByteSliceReader::readByte() {
  // Assert that we are not @ EOF
  assert (upto + bufferOffset < endIndex);
  if (upto == limit)
    nextSlice();
  return buffer[upto++];
}

int64_t DocumentsWriter::ByteSliceReader::writeTo(IndexOutput* out) {
  int64_t size = 0;
  while(true) {
    if (limit + bufferOffset == endIndex) {
      assert (endIndex - bufferOffset >= upto);
      out->writeBytes(buffer+upto, limit-upto);
      size += limit-upto;
      break;
    } else {
      out->writeBytes(buffer+upto, limit-upto);
      size += limit-upto;
      nextSlice();
    }
  }

  return size;
}

void DocumentsWriter::ByteSliceReader::nextSlice() {

  // Skip to our next slice
  const int32_t nextIndex = ((buffer[limit]&0xff)<<24) + ((buffer[1+limit]&0xff)<<16) + ((buffer[2+limit]&0xff)<<8) + (buffer[3+limit]&0xff);
  level = nextLevelArray[level];
  const int32_t newSize = levelSizeArray[level];

  bufferUpto = nextIndex / BYTE_BLOCK_SIZE;
  bufferOffset = bufferUpto * BYTE_BLOCK_SIZE;

  buffer = pool->buffers[bufferUpto];
  upto = nextIndex & BYTE_BLOCK_MASK;

  if (nextIndex + newSize >= endIndex) {
    // We are advancing to the const slice
    assert (endIndex - nextIndex > 0);
    limit = endIndex - bufferOffset;
  } else {
    // This is not the const slice (subtract 4 for the
    // forwarding address at the end of this new slice)
    limit = upto+newSize-4;
  }
}

void DocumentsWriter::ByteSliceReader::readBytes(uint8_t* b, int32_t len) {
  while(len > 0) {
    const int32_t numLeft = limit-upto;
    if (numLeft < len) {
      // Read entire slice
      memcpy(b, buffer+upto,numLeft * sizeof(uint8_t));
      b += numLeft;
      len -= numLeft;
      nextSlice();
    } else {
      // This slice is the last one
      memcpy(b, buffer+upto,len * sizeof(uint8_t));
      upto += len;
      break;
    }
  }
}

int64_t DocumentsWriter::ByteSliceReader::getFilePointer() const{_CLTHROWA(CL_ERR_Runtime,"not implemented");}
int64_t DocumentsWriter::ByteSliceReader::length() const{_CLTHROWA(CL_ERR_Runtime,"not implemented");}
void DocumentsWriter::ByteSliceReader::seek(const int64_t /*pos*/) {_CLTHROWA(CL_ERR_Runtime,"not implemented");}
void DocumentsWriter::ByteSliceReader::close() {_CLTHROWA(CL_ERR_Runtime,"not implemented");}

DocumentsWriter::ByteBlockPool::ByteBlockPool( bool _trackAllocations, DocumentsWriter* _parent):
  BlockPool<uint8_t>(_parent, BYTE_BLOCK_SIZE, _trackAllocations)
{
}
DocumentsWriter::ByteBlockPool::~ByteBlockPool(){
  reset();
  //delete the first block
  _CLDELETE_ARRAY(buffer);
}
uint8_t* DocumentsWriter::ByteBlockPool::getNewBlock(bool _trackAllocations){
  return parent->getByteBlock(_trackAllocations);
}
int32_t DocumentsWriter::ByteBlockPool::newSlice(const int32_t size) {
  if (tUpto > BYTE_BLOCK_SIZE-size)
    nextBuffer();
  const int32_t upto = tUpto;
  tUpto += size;
  buffer[tUpto-1] = 16;
  return upto;
}

int32_t DocumentsWriter::ByteBlockPool::allocSlice(uint8_t* slice, const int32_t upto) {
  const int32_t level = slice[upto] & 15;
  assert(level < 10);
  const int32_t newLevel = nextLevelArray[level];
  const int32_t newSize = levelSizeArray[newLevel];

  // Maybe allocate another block
  if (tUpto > BYTE_BLOCK_SIZE-newSize)
    nextBuffer();

  const int32_t newUpto = tUpto;
  const uint32_t offset = newUpto + tOffset;
  tUpto += newSize;

  // Copy forward the past 3 bytes (which we are about
  // to overwrite with the forwarding address):
  buffer[newUpto] = slice[upto-3];
  buffer[newUpto+1] = slice[upto-2];
  buffer[newUpto+2] = slice[upto-1];

  // Write forwarding address at end of last slice:
  slice[upto-3] = (uint8_t) (offset >> 24); //offset is unsigned...
  slice[upto-2] = (uint8_t) (offset >> 16);
  slice[upto-1] = (uint8_t) (offset >> 8);
  slice[upto] = (uint8_t) offset;

  // Write new level:
  buffer[tUpto-1] = (uint8_t) (16|newLevel);

  return newUpto+3;
}
void DocumentsWriter::ByteBlockPool::reset() {
  if (bufferUpto != -1) {
    // We allocated at least one buffer

    for(int i=0;i<bufferUpto;i++)
      // Fully zero fill buffers that we fully used
      memset(buffers.values[i], 0, BYTE_BLOCK_SIZE );

    // Partial zero fill the final buffer
    memset(buffers.values[bufferUpto], 0, tUpto);

    if (bufferUpto > 0)
      // Recycle all but the first buffer
      parent->recycleBlocks(buffers, 1, 1+bufferUpto);

    // Re-use the first buffer
    bufferUpto = 0;
    tUpto = 0;
    tOffset = 0;
    buffer = buffers[0];
  }
}
DocumentsWriter::CharBlockPool::CharBlockPool(DocumentsWriter* _parent):
    BlockPool<TCHAR>(_parent, CHAR_BLOCK_SIZE, false)
{
}
DocumentsWriter::CharBlockPool::~CharBlockPool(){
}
TCHAR* DocumentsWriter::CharBlockPool::getNewBlock(bool){
    return parent->getCharBlock();
}
void DocumentsWriter::CharBlockPool::reset() {
  parent->recycleBlocks(buffers, 0, 1+bufferUpto);
  bufferUpto = -1;
  tUpto = blockSize;
  tOffset = -blockSize;
}

CL_NS_END
