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
#include "_TermInfo.h"
#include "_FieldInfos.h"
#include "_CompoundFile.h"
#include "IndexWriter.h"
#include "_IndexFileNames.h"
#include "_FieldsWriter.h"
#include "Term.h"
#include "_Term.h"
#include "_TermVector.h"
#include "_TermInfosWriter.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/search/Similarity.h"
#include "_TermInfosWriter.h"
#include "_FieldsWriter.h"
#include "_DocumentsWriter.h"
#include <assert.h>
#include <iostream>

CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(analysis)
CL_NS_USE(document)
CL_NS_USE(search)
CL_NS_DEF(index)


DocumentsWriter::ThreadState::ThreadState(DocumentsWriter* __parent):
  postingsFreeListTS(ValueArray<Posting*>(256)),
  vectorFieldPointers(ValueArray<int64_t>(10)),
  vectorFieldNumbers(ValueArray<int32_t>(10)),
  fieldDataArray(ValueArray<FieldData*>(8)),
  fieldDataHash(ValueArray<FieldData*>(16)),
  postingsVectors(ObjectArray<PostingVector>(1)),
  postingsPool( _CLNEW ByteBlockPool(true, __parent) ),
  vectorsPool( _CLNEW ByteBlockPool(false, __parent) ),
  charPool( _CLNEW CharBlockPool(__parent) ),
  allFieldDataArray(ValueArray<FieldData*>(10)),
  _parent(__parent)
{
  fieldDataHashMask = 15;
  postingsFreeCountTS = 0;
  stringReader = _CLNEW ReusableStringReader(_T(""),0,false);

  isIdle = true;
  numThreads = 1;

  tvfLocal = _CLNEW RAMOutputStream();    // Term vectors for one doc
  fdtLocal = _CLNEW RAMOutputStream();    // Stored fields for one doc

  this->docBoost = 0.0;
  this->fieldGen = this->posUpto = this->maxPostingsVectors = this->numStoredFields = 0;
  this->numAllFieldData = this->docID = 0;
  this->numFieldData = numVectorFields = this->proxUpto = this->freqUpto = this->offsetUpto = 0;
  this->localFieldsWriter = NULL;
  this->maxTermPrefix = NULL;
  this->p = NULL;
  this->prox = NULL;
  this->vector = NULL;
  this->offsets = NULL;
  this->pos = NULL;
  this->freq = NULL;
  this->doFlushAfter = false;
}

DocumentsWriter::ThreadState::~ThreadState(){
  _CLDELETE(postingsPool);
  _CLDELETE(vectorsPool);
  _CLDELETE(charPool);
  _CLDELETE(stringReader);
  _CLDELETE(tvfLocal);
  _CLDELETE(fdtLocal);

  for ( size_t i=0; i<allFieldDataArray.length;i++)
    _CLDELETE(allFieldDataArray.values[i]);
}

void DocumentsWriter::ThreadState::resetPostings() {
  fieldGen = 0;
  maxPostingsVectors = 0;
  doFlushAfter = false;
  if (localFieldsWriter != NULL) {
    localFieldsWriter->close();
    _CLDELETE(localFieldsWriter);
  }
  postingsPool->reset();
  charPool->reset();
  _parent->recyclePostings(this->postingsFreeListTS, this->postingsFreeCountTS);
  this->postingsFreeCountTS = 0;
  for(int32_t i=0;i<numAllFieldData;i++) {
    FieldData* fp = allFieldDataArray[i];
    fp->lastGen = -1;
    if (fp->numPostings > 0)
      fp->resetPostingArrays();
  }
}

void DocumentsWriter::ThreadState::writeDocument() {

  // If we hit an exception while appending to the
  // stored fields or term vectors files, we have to
  // abort all documents since we last flushed because
  // it means those files are possibly inconsistent.
  try {
    _parent->numDocsInStore++;

    // Append stored fields to the real FieldsWriter:
    _parent->fieldsWriter->flushDocument(numStoredFields, fdtLocal);
    fdtLocal->reset();

    // Append term vectors to the real outputs:
    if (_parent->tvx != NULL) {
      _parent->tvx->writeLong(_parent->tvd->getFilePointer());
      _parent->tvd->writeVInt(numVectorFields);
      if (numVectorFields > 0) {
        for(int32_t i=0;i<numVectorFields;i++)
          _parent->tvd->writeVInt(vectorFieldNumbers[i]);
        assert(0 == vectorFieldPointers[0]);
        _parent->tvd->writeVLong(_parent->tvf->getFilePointer());
        int64_t lastPos = vectorFieldPointers[0];
        for(int32_t i=1;i<numVectorFields;i++) {
          int64_t pos = vectorFieldPointers[i];
          _parent->tvd->writeVLong(pos-lastPos);
          lastPos = pos;
        }
        tvfLocal->writeTo(_parent->tvf);
        tvfLocal->reset();
      }
    }

    // Append norms for the fields we saw:
    for(int32_t i=0;i<numFieldData;i++) {
      FieldData* fp = fieldDataArray[i];
      if (fp->doNorms) {
        BufferedNorms* bn = _parent->norms[fp->fieldInfo->number];
        assert ( bn != NULL );
        assert ( bn->upto <= docID );
        bn->fill(docID);
        float_t norm = fp->boost * _parent->writer->getSimilarity()->lengthNorm(fp->fieldInfo->name, fp->length);
        bn->add(norm);
      }
    }
  } catch (CLuceneError& t) {
    // Forcefully idle this threadstate -- its state will
    // be reset by abort()
    isIdle = true;
    throw AbortException(t, _parent);
  }

  if (_parent->bufferIsFull && !_parent->flushPending) {
    _parent->flushPending = true;
    doFlushAfter = true;
  }
}

void DocumentsWriter::ThreadState::init(Document* doc, int32_t docID) {

  assert (!isIdle);
  assert (_parent->writer->testPoint("DocumentsWriter.ThreadState.init start"));

  this->docID = docID;
  docBoost = doc->getBoost();
  numStoredFields = 0;
  numFieldData = 0;
  numVectorFields = 0;
  maxTermPrefix = NULL;

  assert (0 == fdtLocal->length());
  assert (0 == fdtLocal->getFilePointer());
  assert (0 == tvfLocal->length());
  assert (0 == tvfLocal->getFilePointer());
  const int32_t thisFieldGen = fieldGen++;

  const Document::FieldsType& docFields = *doc->getFields();
  const int32_t numDocFields = docFields.size();
  bool docHasVectors = false;

  // Absorb any new fields first seen in this document.
  // Also absorb any changes to fields we had already
  // seen before (eg suddenly turning on norms or
  // vectors, etc.):

  for(int32_t i=0;i<numDocFields;i++) {
    Field* field = docFields[i];

    FieldInfo* fi = _parent->fieldInfos->add(field->name(), field->isIndexed(), field->isTermVectorStored(),
                                  field->isStorePositionWithTermVector(), field->isStoreOffsetWithTermVector(),
                                  field->getOmitNorms(), false);
    if (fi->isIndexed && !fi->omitNorms) {
      // Maybe grow our buffered norms
      if (_parent->norms.length <= fi->number) {
        int32_t newSize = (int32_t) ((1+fi->number)*1.25);
        _parent->norms.resize(newSize);
      }

      if (_parent->norms[fi->number] == NULL)
        _parent->norms.values[fi->number] = _CLNEW BufferedNorms();

      _parent->hasNorms = true;
    }

    // Make sure we have a FieldData allocated
    int32_t hashPos = Misc::thashCode(fi->name) & fieldDataHashMask; //TODO: put hash in fieldinfo
    FieldData* fp = fieldDataHash[hashPos];
    while(fp != NULL && _tcscmp(fp->fieldInfo->name, fi->name) != 0 )
      fp = fp->next;

    if (fp == NULL) {
      fp = _CLNEW FieldData(_parent,this,fi);
      fp->next = fieldDataHash[hashPos];
      fieldDataHash.values[hashPos] = fp;

      if (numAllFieldData == allFieldDataArray.length) {
        allFieldDataArray.resize( (int32_t) (allFieldDataArray.length*1.5) );

        ValueArray<FieldData*> newHashArray(fieldDataHash.length*2);

        // Rehash
        fieldDataHashMask = allFieldDataArray.length-1;
        for(size_t j=0;j<fieldDataHash.length;j++) {
          FieldData* fp0 = fieldDataHash[j];
          while(fp0 != NULL) {
            //todo: put hash code into fieldinfo to reduce number of hashes necessary
            hashPos = Misc::thashCode(fp0->fieldInfo->name) & fieldDataHashMask;
            FieldData* nextFP0 = fp0->next;
            fp0->next = newHashArray[hashPos];
            newHashArray.values[hashPos] = fp0;
            fp0 = nextFP0;
          }
        }
        fieldDataHash.resize( newHashArray.length );
        memcpy(fieldDataHash.values, newHashArray.values, newHashArray.length * sizeof(FieldData*));
      }
      allFieldDataArray.values[numAllFieldData++] = fp;
    } else {
      assert (fp->fieldInfo == fi);
    }

    if (thisFieldGen != fp->lastGen) {

      // First time we're seeing this field for this doc
      fp->lastGen = thisFieldGen;
      fp->fieldCount = 0;
      fp->doVectors = fp->doVectorPositions = fp->doVectorOffsets = false;
      fp->doNorms = fi->isIndexed && !fi->omitNorms;

      if (numFieldData == fieldDataArray.length) {
        fieldDataArray.resize(fieldDataArray.length*2);
      }
      fieldDataArray.values[numFieldData++] = fp;
    }

    if (field->isTermVectorStored()) {
      if (!fp->doVectors && numVectorFields++ == vectorFieldPointers.length) {
        const int32_t newSize = (int32_t)(numVectorFields*1.5);
		    vectorFieldPointers.resize(newSize);
		    vectorFieldNumbers.resize(newSize);
      }
      fp->doVectors = true;
      docHasVectors = true;

      fp->doVectorPositions |= field->isStorePositionWithTermVector();
      fp->doVectorOffsets |= field->isStoreOffsetWithTermVector();
    }

    if (fp->fieldCount == fp->docFields.length) {
      fp->docFields.resize(fp->docFields.length*2);
    }

    // Lazily allocate arrays for postings:
    if (field->isIndexed() && fp->postingsHash.values == NULL)
      fp->initPostingArrays();

    fp->docFields.values[fp->fieldCount++] = field;
  }

  // Maybe init the local & global fieldsWriter
  if (localFieldsWriter == NULL) {
    if (_parent->fieldsWriter == NULL) {
      assert (_parent->docStoreSegment.empty());
      assert (!_parent->segment.empty());
      _parent->docStoreSegment = _parent->segment;
      // If we hit an exception while init'ing the
      // fieldsWriter, we must abort this segment
      // because those files will be in an unknown
      // state:
      try {
        _parent->fieldsWriter = _CLNEW FieldsWriter(_parent->directory, _parent->docStoreSegment.c_str(), _parent->fieldInfos);
      } catch (CLuceneError& t) {
        throw AbortException(t,_parent);
      }
      _CLDELETE(_parent->_files);
    }
    localFieldsWriter = _CLNEW FieldsWriter(NULL, fdtLocal, _parent->fieldInfos);
  }

  // First time we see a doc that has field(s) with
  // stored vectors, we init our tvx writer
  if (docHasVectors) {
    if (_parent->tvx == NULL) {
      assert (!_parent->docStoreSegment.empty());
      // If we hit an exception while init'ing the term
      // vector output files, we must abort this segment
      // because those files will be in an unknown
      // state:
      try {
        _parent->tvx = _parent->directory->createOutput( (_parent->docStoreSegment + "." + IndexFileNames::VECTORS_INDEX_EXTENSION).c_str() );
        _parent->tvx->writeInt(TermVectorsReader::FORMAT_VERSION);
        _parent->tvd = _parent->directory->createOutput( (_parent->docStoreSegment +  "." + IndexFileNames::VECTORS_DOCUMENTS_EXTENSION).c_str() );
        _parent->tvd->writeInt(TermVectorsReader::FORMAT_VERSION);
        _parent->tvf = _parent->directory->createOutput( (_parent->docStoreSegment +  "." + IndexFileNames::VECTORS_FIELDS_EXTENSION).c_str() );
        _parent->tvf->writeInt(TermVectorsReader::FORMAT_VERSION);

        // We must "catch up" for all docs before us
        // that had no vectors:
        for(int32_t i=0;i<_parent->numDocsInStore;i++) {
          _parent->tvx->writeLong(_parent->tvd->getFilePointer());
          _parent->tvd->writeVInt(0);
        }

      } catch (CLuceneError& t) {
        throw AbortException(t, _parent);
      }
      _CLDELETE(_parent->_files);
    }

    numVectorFields = 0;
  }
}

void DocumentsWriter::ThreadState::doPostingSort(Posting** postings, int32_t numPosting) {
  quickSort(postings, 0, numPosting-1);
}

void DocumentsWriter::ThreadState::quickSort(Posting** postings, int32_t lo, int32_t hi) {
  if (lo >= hi)
    return;

  int32_t mid = ((uint32_t)(lo + hi)) >> 1; //unsigned shift...

  if (comparePostings(postings[lo], postings[mid]) > 0) {
    Posting* tmp = postings[lo];
    postings[lo] = postings[mid];
    postings[mid] = tmp;
  }

  if (comparePostings(postings[mid], postings[hi]) > 0) {
    Posting* tmp = postings[mid];
    postings[mid] = postings[hi];
    postings[hi] = tmp;

    if (comparePostings(postings[lo], postings[mid]) > 0) {
      Posting* tmp2 = postings[lo];
      postings[lo] = postings[mid];
      postings[mid] = tmp2;
    }
  }

  int32_t left = lo + 1;
  int32_t right = hi - 1;

  if (left >= right)
    return;

  Posting* partition = postings[mid];

  for (; ;) {
    while (comparePostings(postings[right], partition) > 0)
      --right;

    while (left < right && comparePostings(postings[left], partition) <= 0)
      ++left;

    if (left < right) {
      Posting* tmp = postings[left];
      postings[left] = postings[right];
      postings[right] = tmp;
      --right;
    } else {
      break;
    }
  }

  quickSort(postings, lo, left);
  quickSort(postings, left + 1, hi);
}

void DocumentsWriter::ThreadState::doVectorSort(ArrayBase<PostingVector*>& postings, int32_t numPosting) {
  quickSort(postings, 0, numPosting-1);
}

void DocumentsWriter::ThreadState::quickSort(ArrayBase<PostingVector*>& postings, int32_t lo, int32_t hi) {
  if (lo >= hi)
    return;

  int32_t mid = ((uint8_t)(lo + hi)) >> 1; //unsigned shift..

  if (comparePostings(postings[lo]->p, postings[mid]->p) > 0) {
    PostingVector* tmp = postings[lo];
    postings.values[lo] = postings[mid];
    postings.values[mid] = tmp;
  }

  if (comparePostings(postings[mid]->p, postings[hi]->p) > 0) {
    PostingVector* tmp = postings[mid];
    postings.values[mid] = postings[hi];
    postings.values[hi] = tmp;

    if (comparePostings(postings[lo]->p, postings[mid]->p) > 0) {
      PostingVector* tmp2 = postings[lo];
      postings.values[lo] = postings[mid];
      postings.values[mid] = tmp2;
    }
  }

  int32_t left = lo + 1;
  int32_t right = hi - 1;

  if (left >= right)
    return;

  PostingVector* partition = postings[mid];

  for (; ;) {
    while (comparePostings(postings[right]->p, partition->p) > 0)
      --right;

    while (left < right && comparePostings(postings[left]->p, partition->p) <= 0)
      ++left;

    if (left < right) {
      PostingVector* tmp = postings[left];
      postings.values[left] = postings[right];
      postings.values[right] = tmp;
      --right;
    } else {
      break;
    }
  }

  quickSort(postings, lo, left);
  quickSort(postings, left + 1, hi);
}

void DocumentsWriter::ThreadState::trimFields() {

  int32_t upto = 0;
  for(int32_t i=0;i<numAllFieldData;i++) {
    FieldData* fp = allFieldDataArray[i];
    if (fp->lastGen == -1) {
      // This field was not seen since the previous
      // flush, so, free up its resources now

      // Unhash
      const int32_t hashPos = Misc::thashCode(fp->fieldInfo->name) & fieldDataHashMask;
      FieldData* last = NULL;
      FieldData* fp0 = fieldDataHash[hashPos];
      while(fp0 != fp) {
        last = fp0;
        fp0 = fp0->next;
      }
      assert(fp0 != NULL);

      if (last == NULL)
        fieldDataHash.values[hashPos] = fp->next;
      else
        last->next = fp->next;

      if (_parent->infoStream != NULL)
        (*_parent->infoStream) << "  remove field=" << fp->fieldInfo->name << "\n";

      _CLDELETE(fp);
    } else {
      // Reset
      fp->lastGen = -1;
      allFieldDataArray.values[upto++] = fp;

      if (fp->numPostings > 0 && ((float_t) fp->numPostings) / fp->postingsHashSize < 0.2) {
        int32_t hashSize = fp->postingsHashSize;

        // Reduce hash so it's between 25-50% full
        while (fp->numPostings < (hashSize>>1) && hashSize >= 2)
          hashSize >>= 1;
        hashSize <<= 1;

        if (hashSize != fp->postingsHash.length)
          fp->rehashPostings(hashSize);
      }
    }
  }
  //delete everything after up to in allFieldDataArray
  for ( size_t i=upto;i<allFieldDataArray.length;i++ ){
    allFieldDataArray[i] = NULL;
  }

  // If we didn't see any norms for this field since
  // last flush, free it
  for(size_t i=0;i<_parent->norms.length;i++) {
    BufferedNorms* n = _parent->norms[i];
    if (n != NULL && n->upto == 0)
    {
      _CLLDELETE(n);
      _parent->norms.values[i] = NULL;
    }
  }

  numAllFieldData = upto;

  // Also pare back PostingsVectors if it's excessively
  // large
  if (maxPostingsVectors * 1.5 < postingsVectors.length) {
    int32_t newSize;
    if (0 == maxPostingsVectors)
      newSize = 1;
    else
      newSize = (int32_t) (1.5*maxPostingsVectors);
    postingsVectors.resize(newSize, true);
  }
}

void DocumentsWriter::ThreadState::processDocument(Analyzer* analyzer)
{

  const int32_t numFields = numFieldData;

  assert (0 == fdtLocal->length());

  if (_parent->tvx != NULL){
    // If we are writing vectors then we must visit
    // fields in sorted order so they are written in
    // sorted order.  TODO: we actually only need to
    // sort the subset of fields that have vectors
    // enabled; we could save [small amount of] CPU
    // here.
    Arrays<FieldData*>::sort(fieldDataArray.values,fieldDataArray.length, 0, numFields);
  }

  // We process the document one field at a time
  for(int32_t i=0;i<numFields;i++)
    fieldDataArray[i]->processField(analyzer);

  if (maxTermPrefix != NULL && _parent->infoStream != NULL)
    (*_parent->infoStream) << "WARNING: document contains at least one immense term (longer than the max length " << MAX_TERM_LENGTH << "), all of which were skipped.  Please correct the analyzer to not produce such terms.  The prefix of the first immense term is: '" << maxTermPrefix << "...'\n";

  if (_parent->ramBufferSize != IndexWriter::DISABLE_AUTO_FLUSH
      && _parent->numBytesUsed > 0.95 * _parent->ramBufferSize)
    _parent->balanceRAM();
}

// USE ONLY FOR DEBUGGING!
/*
  String getPostingText() {
  TCHAR* text = charPool->buffers[p->textStart >> CHAR_BLOCK_SHIFT];
  int32_t upto = p->textStart & CHAR_BLOCK_MASK;
  while((*text)[upto] != CLUCENE_END_OF_WORD)
  upto++;
  return new String(text, p->textStart, upto-(p->textStart & BYTE_BLOCK_MASK));
  }
*/

bool DocumentsWriter::ThreadState::postingEquals(const TCHAR* tokenText, const int32_t tokenTextLen) {

  const TCHAR* text = charPool->buffers[p->textStart >> CHAR_BLOCK_SHIFT];
  assert (text != NULL);
  int32_t pos = p->textStart & CHAR_BLOCK_MASK;

  int32_t tokenPos = 0;
  for(;tokenPos<tokenTextLen;pos++,tokenPos++)
    if (tokenText[tokenPos] != text[pos])
      return false;
  return CLUCENE_END_OF_WORD == text[pos];
}

int32_t DocumentsWriter::ThreadState::comparePostings(Posting* p1, Posting* p2) {
  const TCHAR* pos1 = charPool->buffers[p1->textStart >> CHAR_BLOCK_SHIFT] + (p1->textStart & CHAR_BLOCK_MASK);
  const TCHAR* pos2 = charPool->buffers[p2->textStart >> CHAR_BLOCK_SHIFT] + (p2->textStart & CHAR_BLOCK_MASK);
  while(true) {
    const TCHAR c1 = *pos1++;
    const TCHAR c2 = *pos2++;
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

void DocumentsWriter::ThreadState::writeFreqVInt(int32_t vi) {
  uint32_t i = vi;
  while ((i & ~0x7F) != 0) {
    writeFreqByte((uint8_t)((i & 0x7f) | 0x80));
    i >>= 7; //unsigned shift...
  }
  writeFreqByte((uint8_t) i);
}

void DocumentsWriter::ThreadState::writeProxVInt(int32_t vi) {
  uint32_t i = vi;
  while ((i & ~0x7F) != 0) {
    writeProxByte((uint8_t)((i & 0x7f) | 0x80));
    i >>= 7; //unsigned shift...
  }
  writeProxByte((uint8_t) i);
}

void DocumentsWriter::ThreadState::writeFreqByte(uint8_t b) {
  assert (freq != NULL);
  if (freq[freqUpto] != 0) {
    freqUpto = postingsPool->allocSlice(freq, freqUpto);
    freq = postingsPool->buffer;
    p->freqUpto = postingsPool->tOffset;
  }
  freq[freqUpto++] = b;
}

void DocumentsWriter::ThreadState::writeProxByte(uint8_t b) {
  assert (prox != NULL);
  if (prox[proxUpto] != 0) {
    proxUpto = postingsPool->allocSlice(prox, proxUpto);
    prox = postingsPool->buffer;
    p->proxUpto = postingsPool->tOffset;
    assert (prox != NULL);
  }
  prox[proxUpto++] = b;
  assert (proxUpto != DocumentsWriter::BYTE_BLOCK_SIZE);
}

void DocumentsWriter::ThreadState::writeProxBytes(uint8_t* b, int32_t offset, int32_t len) {
  const int32_t offsetEnd = offset + len;
  while(offset < offsetEnd) {
    if (prox[proxUpto] != 0) {
      // End marker
      proxUpto = postingsPool->allocSlice(prox, proxUpto);
      prox = postingsPool->buffer;
      p->proxUpto = postingsPool->tOffset;
    }

    prox[proxUpto++] = b[offset++];
    assert (proxUpto != DocumentsWriter::BYTE_BLOCK_SIZE);
  }
}

void DocumentsWriter::ThreadState::writeOffsetVInt(int32_t vi) {
  uint32_t i = vi;
  while ((i & ~0x7F) != 0) {
    writeOffsetByte((uint8_t)((i & 0x7f) | 0x80));
    i >>= 7; //unsigned shift...
  }
  writeOffsetByte((uint8_t) i);
}

void DocumentsWriter::ThreadState::writeOffsetByte(uint8_t b) {
  assert (offsets != NULL);
  if (offsets[offsetUpto] != 0) {
    offsetUpto = vectorsPool->allocSlice(offsets, offsetUpto);
    offsets = vectorsPool->buffer;
    vector->offsetUpto = vectorsPool->tOffset;
  }
  offsets[offsetUpto++] = b;
}

void DocumentsWriter::ThreadState::writePosVInt(int32_t vi) {
	uint32_t i = vi;
  while ((i & ~0x7F) != 0) {
    writePosByte((uint8_t)((i & 0x7f) | 0x80));
    i >>= 7; //unsigned shift...
  }
  writePosByte((uint8_t) i);
}

void DocumentsWriter::ThreadState::writePosByte(uint8_t b) {
  assert (pos != NULL);
  if (pos[posUpto] != 0) {
    posUpto = vectorsPool->allocSlice(pos, posUpto);
    pos = vectorsPool->buffer;
    vector->posUpto = vectorsPool->tOffset;
  }
  pos[posUpto++] = b;
}



DocumentsWriter::ThreadState::FieldData::FieldData(DocumentsWriter* __parent, ThreadState* __threadState, FieldInfo* fieldInfo):
  docFields(ValueArray<Field*>(1)),
  _parent(__parent),
  localToken (_CLNEW Token),
  vectorSliceReader(_CLNEW ByteSliceReader())
{
  this->fieldCount = this->postingsHashSize = this->postingsHashHalfSize = this->postingsVectorsUpto = 0;
  this->postingsHashMask = this->offsetEnd = 0;
  this->offsetStartCode = this->offsetStart = this->numPostings = this->position = this->length = this->offset = 0;
  this->boost = 0.0;
  this->next = NULL;
  this->lastGen = -1;
  this->fieldInfo = fieldInfo;
  this->threadState = __threadState;
  this->postingsCompacted = false;
}
DocumentsWriter::ThreadState::FieldData::~FieldData(){
  _CLDELETE(vectorSliceReader);
  _CLDELETE(localToken);
}
bool DocumentsWriter::ThreadState::FieldData::sort(FieldData* e1, FieldData* e2){
  return _tcscmp(e1->fieldInfo->name, e2->fieldInfo->name) < 0;
}
void DocumentsWriter::ThreadState::FieldData::resetPostingArrays() {
  if (!postingsCompacted)
    compactPostings();
  _parent->recyclePostings(this->postingsHash, numPostings);
  memset(postingsHash.values, 0, postingsHash.length * sizeof(Posting*));
  postingsCompacted = false;
  numPostings = 0;
}

const char* DocumentsWriter::ThreadState::FieldData::getObjectName() const{
  return getClassName();
}
const char* DocumentsWriter::ThreadState::FieldData::getClassName(){
  return "DocumentsWriter::ThreadState";
}
void DocumentsWriter::ThreadState::FieldData::initPostingArrays() {
  // Target hash fill factor of <= 50%
  // NOTE: must be a power of two for hash collision
  // strategy to work correctly
  postingsHashSize = 4;
  postingsHashHalfSize = 2;
  postingsHashMask = postingsHashSize-1;
  postingsHash.resize(postingsHashSize);
}

int32_t DocumentsWriter::ThreadState::FieldData::compareTo(NamedObject* o) {
  if ( o->getObjectName() != FieldData::getClassName() )
    return -1;
  return _tcscmp(fieldInfo->name, ((FieldData*) o)->fieldInfo->name);
}

void DocumentsWriter::ThreadState::FieldData::compactPostings() {
  int32_t upto = 0;
  for(int32_t i=0;i<postingsHashSize;i++)
    if (postingsHash[i] != NULL)
      postingsHash.values[upto++] = postingsHash[i];

  assert (upto == numPostings);
  postingsCompacted = true;
}

CL_NS(util)::ValueArray<DocumentsWriter::Posting*>* DocumentsWriter::ThreadState::FieldData::sortPostings() {
  compactPostings();
  threadState->doPostingSort(postingsHash.values, numPostings);
  return &postingsHash;
}


void DocumentsWriter::ThreadState::FieldData::processField(Analyzer* analyzer) {
  length = 0;
  position = 0;
  offset = 0;
  boost = threadState->docBoost;

  const int32_t maxFieldLength = _parent->writer->getMaxFieldLength();

  const int32_t limit = fieldCount;
  const ArrayBase<Field*>& docFieldsFinal = docFields;

  bool doWriteVectors = true;

  // Walk through all occurrences in this doc for this
  // field:
  try {
    for(int32_t j=0;j<limit;j++) {
      Field* field = docFieldsFinal[j];

      if (field->isIndexed())
        invertField(field, analyzer, maxFieldLength);

      if (field->isStored()) {
        threadState->numStoredFields++;
        bool success = false;
        try {
          threadState->localFieldsWriter->writeField(fieldInfo, field);
          success = true;
        } _CLFINALLY(
          // If we hit an exception inside
          // localFieldsWriter->writeField, the
          // contents of fdtLocal can be corrupt, so
          // we must discard all stored fields for
          // this document:
          if (!success)
            threadState->fdtLocal->reset();
        )
      }

      docFieldsFinal.values[j] = NULL;
    }
  } catch (AbortException& ae) {
    doWriteVectors = false;
    throw ae;
  } _CLFINALLY (
    if (postingsVectorsUpto > 0) {
      try {
        if (doWriteVectors) {
          // Add term vectors for this field
          bool success = false;
          try {
            writeVectors(fieldInfo);
            success = true;
          } _CLFINALLY (
            if (!success) {
              // If we hit an exception inside
              // writeVectors, the contents of tvfLocal
              // can be corrupt, so we must discard all
              // term vectors for this document:
              threadState->numVectorFields = 0;
              threadState->tvfLocal->reset();
            }
          )
        }
      } _CLFINALLY (
        if (postingsVectorsUpto > threadState->maxPostingsVectors)
          threadState->maxPostingsVectors = postingsVectorsUpto;
        postingsVectorsUpto = 0;
        threadState->vectorsPool->reset();
      )
    }
  )
}
void DocumentsWriter::ThreadState::FieldData::invertField(Field* field, Analyzer* analyzer, const int32_t maxFieldLength) {

  if (length>0)
    position += analyzer->getPositionIncrementGap(fieldInfo->name);

  if (!field->isTokenized()) {     // un-tokenized field
    const TCHAR* stringValue = field->stringValue();
    const size_t valueLength = _tcslen(stringValue);
    Token* token = localToken;
    token->clear();

    token->setText(stringValue,valueLength);
    token->setStartOffset(offset);
    token->setEndOffset(offset + valueLength);
    addPosition(token);
    offset += valueLength;
    length++;
  } else {                                  // tokenized field
    TokenStream* stream;
    TokenStream* streamValue = field->tokenStreamValue();

    if (streamValue != NULL)
      stream = streamValue;
    else {
      // the field does not have a TokenStream,
      // so we have to obtain one from the analyzer
      Reader* reader;        // find or make Reader
      Reader* readerValue = field->readerValue();

      if (readerValue != NULL)
        reader = readerValue;
      else {
        const TCHAR* stringValue = field->stringValue();
        size_t stringValueLength = _tcslen(stringValue);
        if (stringValue == NULL)
          _CLTHROWA(CL_ERR_IllegalArgument, "field must have either TokenStream, String or Reader value");
        threadState->stringReader->init(stringValue, stringValueLength);
        reader = threadState->stringReader;
      }

      // Tokenize field and add to postingTable
      stream = analyzer->reusableTokenStream(fieldInfo->name, reader);
    }

    // reset the TokenStream to the first token
    stream->reset();

    try {
      offsetEnd = offset-1;
      for(;;) {
        Token* token = stream->next(localToken);
        if (token == NULL) break;
        position += (token->getPositionIncrement() - 1);
        addPosition(token);
        ++length;

				// Apply field truncation policy.
				if (maxFieldLength != IndexWriter::FIELD_TRUNC_POLICY__WARN) {
					// The client programmer has explicitly authorized us to
					// truncate the token stream after maxFieldLength tokens.
					if ( length >= maxFieldLength) {
	          if (_parent->infoStream != NULL)
	            (*_parent->infoStream) << "maxFieldLength "  << maxFieldLength << " reached for field " << fieldInfo->name << ", ignoring following tokens\n";
						break;
					}
				} else if (length > IndexWriter::DEFAULT_MAX_FIELD_LENGTH) {
					const TCHAR* errMsgBase =
						_T("Indexing a huge number of tokens from a single")
						_T(" field (\"%s\", in this case) can cause CLucene")
						_T(" to use memory excessively.")
						_T("  By default, CLucene will accept only %s tokens")
						_T(" tokens from a single field before forcing the")
						_T(" client programmer to specify a threshold at")
						_T(" which to truncate the token stream.")
						_T("  You should set this threshold via")
						_T(" IndexReader::maxFieldLength (set to LUCENE_INT32_MAX")
						_T(" to disable truncation, or a value to specify maximum number of fields).");

					TCHAR defaultMaxAsChar[34];
					_i64tot(IndexWriter::DEFAULT_MAX_FIELD_LENGTH,
						defaultMaxAsChar, 10
					);
					int32_t errMsgLen = _tcslen(errMsgBase)
						+ _tcslen(fieldInfo->name)
						+ _tcslen(defaultMaxAsChar);
					TCHAR* errMsg = _CL_NEWARRAY(TCHAR,errMsgLen+1);

					_sntprintf(errMsg, errMsgLen,errMsgBase, fieldInfo->name, defaultMaxAsChar);

					_CLTHROWT_DEL(CL_ERR_Runtime,errMsg);
				}
      }
      offset = offsetEnd+1;
    } _CLFINALLY (
      stream->close(); //don't delete, this stream is re-used
    )
  }

  boost *= field->getBoost();
}

DocumentsWriter::PostingVector* DocumentsWriter::ThreadState::FieldData::addNewVector() {

  if (postingsVectorsUpto == threadState->postingsVectors.length) {
    int32_t newSize;
    if (threadState->postingsVectors.length < 2)
      newSize = 2;
    else
      newSize = (int32_t) (1.5*threadState->postingsVectors.length);
    threadState->postingsVectors.resize(newSize, true);
  }

  threadState->p->vector = threadState->postingsVectors[postingsVectorsUpto];
  if (threadState->p->vector == NULL)
    threadState->p->vector = threadState->postingsVectors.values[postingsVectorsUpto] = _CLNEW PostingVector();

  postingsVectorsUpto++;

  PostingVector* v = threadState->p->vector;
  v->p = threadState->p;

  const int32_t firstSize = levelSizeArray[0];

  if (doVectorPositions) {
    const int32_t upto = threadState->vectorsPool->newSlice(firstSize);
    v->posStart = v->posUpto = threadState->vectorsPool->tOffset + upto;
  }

  if (doVectorOffsets) {
    const int32_t upto = threadState->vectorsPool->newSlice(firstSize);
    v->offsetStart = v->offsetUpto = threadState->vectorsPool->tOffset + upto;
  }

  return v;
}

void DocumentsWriter::ThreadState::FieldData::addPosition(Token* token) {

  const Payload* payload = token->getPayload();

  // Get the text of this term.  Term can either
  // provide a String token or offset into a TCHAR*
  // array
  const TCHAR* tokenText = token->termBuffer();
  const int32_t tokenTextLen = token->termLength();

  int32_t code = 0;

  // Compute hashcode
  int32_t downto = tokenTextLen;
  while (downto > 0)
    code = (code*31) + tokenText[--downto];
/*
  std::cout << "  addPosition: buffer=" << Misc::toString(tokenText).substr(0,tokenTextLen) << " pos=" << position
            << " offsetStart=" << (offset+token->startOffset()) << " offsetEnd=" << (offset + token->endOffset())
            << " docID=" << threadState->docID << " doPos=" << (doVectorPositions?"true":"false") << " doOffset=" << (doVectorOffsets?"true":"false") << "\n";
*/
  int32_t hashPos = code & postingsHashMask;

  assert (!postingsCompacted);

  // Locate Posting in hash
  threadState->p = postingsHash[hashPos];

  if (threadState->p != NULL && !threadState->postingEquals(tokenText, tokenTextLen)) {
    // Conflict: keep searching different locations in
    // the hash table.
    const int32_t inc = ((code>>8)+code)|1;
    do {
      code += inc;
      hashPos = code & postingsHashMask;
      threadState->p = postingsHash[hashPos];
    } while (threadState->p != NULL && !threadState->postingEquals(tokenText, tokenTextLen));
  }

  int32_t proxCode;

  // If we hit an exception below, it's possible the
  // posting list or term vectors data will be
  // partially written and thus inconsistent if
  // flushed, so we have to abort all documents
  // since the last flush:

  try {

    if (threadState->p != NULL) {       // term seen since last flush

      if (threadState->docID != threadState->p->lastDocID) { // term not yet seen in this doc
/*
        std::cout << "    seen before (new docID=" << threadState->docID << ") freqUpto=" << threadState->p->freqUpto
                  << " proxUpto=" << threadState->p->proxUpto << "\n";
*/
        assert (threadState->p->docFreq > 0);

        // Now that we know doc freq for previous doc,
        // write it & lastDocCode
        threadState->freqUpto = threadState->p->freqUpto & BYTE_BLOCK_MASK;
        threadState->freq = threadState->postingsPool->buffers[threadState->p->freqUpto >> BYTE_BLOCK_SHIFT];
        if (1 == threadState->p->docFreq)
          threadState->writeFreqVInt(threadState->p->lastDocCode | 1);
        else {
          threadState->writeFreqVInt(threadState->p->lastDocCode);
          threadState->writeFreqVInt(threadState->p->docFreq);
        }
        threadState->p->freqUpto = threadState->freqUpto + (threadState->p->freqUpto & BYTE_BLOCK_NOT_MASK);

        if (doVectors) {
          threadState->vector = addNewVector();
          if (doVectorOffsets) {
            offsetStartCode = offsetStart = offset + token->startOffset();
            offsetEnd = offset + token->endOffset();
          }
        }

        proxCode = position;

        threadState->p->docFreq = 1;

        // Store code so we can write this after we're
        // done with this new doc
        threadState->p->lastDocCode = (threadState->docID - threadState->p->lastDocID) << 1;
        threadState->p->lastDocID = threadState->docID;

      } else {                                // term already seen in this doc
         //std::cout << "    seen before (same docID=" << threadState->docID << ") proxUpto=" << threadState->p->proxUpto << "\n";

        threadState->p->docFreq++;

        proxCode = position - threadState->p->lastPosition;

        if (doVectors) {
          threadState->vector = threadState->p->vector;
          if (threadState->vector == NULL)
            threadState->vector = addNewVector();
          if (doVectorOffsets) {
            offsetStart = offset + token->startOffset();
            offsetEnd = offset + token->endOffset();
            offsetStartCode = offsetStart - threadState->vector->lastOffset;
          }
        }
      }
    } else {            // term not seen before
      //std::cout << "    never seen docID=" << threadState->docID << "\n";

      // Refill?
      if (0 == threadState->postingsFreeCountTS) {
        _parent->getPostings(threadState->postingsFreeListTS);
        threadState->postingsFreeCountTS = threadState->postingsFreeListTS.length;
      }

      const int32_t textLen1 = 1+tokenTextLen;
      if (textLen1 + threadState->charPool->tUpto > CHAR_BLOCK_SIZE) {
        if (textLen1 > CHAR_BLOCK_SIZE) {
          // Just skip this term, to remain as robust as
          // possible during indexing.  A TokenFilter
          // can be inserted into the analyzer chain if
          // other behavior is wanted (pruning the term
          // to a prefix, throwing an exception, etc).
          if (threadState->maxTermPrefix == NULL){
            threadState->maxTermPrefix = _CL_NEWARRAY(TCHAR,31);
            _tcsncpy(threadState->maxTermPrefix,tokenText,30);
            threadState->maxTermPrefix[30] = 0;
          }

          // Still increment position:
          position++;
          return;
        }
        threadState->charPool->nextBuffer();
      }
      TCHAR* text = threadState->charPool->buffer;
      TCHAR* textUpto = text+ threadState->charPool->tUpto;

      // Pull next free Posting from free list
      threadState->p = threadState->postingsFreeListTS[--threadState->postingsFreeCountTS];
      assert(threadState->p != NULL);
      threadState->p->textStart = textUpto + threadState->charPool->tOffset - text;
      threadState->charPool->tUpto += textLen1;

      _tcsncpy(textUpto, tokenText, tokenTextLen);
      textUpto[tokenTextLen] = CLUCENE_END_OF_WORD;

      assert (postingsHash[hashPos] == NULL);

      postingsHash.values[hashPos] = threadState->p;
      numPostings++;

      if (numPostings == postingsHashHalfSize)
        rehashPostings(2*postingsHashSize);

      // Init first slice for freq & prox streams
      const int32_t firstSize = levelSizeArray[0];

      const int32_t upto1 = threadState->postingsPool->newSlice(firstSize);
      threadState->p->freqStart = threadState->p->freqUpto = threadState->postingsPool->tOffset + upto1;

      const int32_t upto2 = threadState->postingsPool->newSlice(firstSize);
      threadState->p->proxStart = threadState->p->proxUpto = threadState->postingsPool->tOffset + upto2;

      threadState->p->lastDocCode = threadState->docID << 1;
      threadState->p->lastDocID = threadState->docID;
      threadState->p->docFreq = 1;

      if (doVectors) {
        threadState->vector = addNewVector();
        if (doVectorOffsets) {
          offsetStart = offsetStartCode = offset + token->startOffset();
          offsetEnd = offset + token->endOffset();
        }
      }

      proxCode = position;
    }

    threadState->proxUpto = threadState->p->proxUpto & BYTE_BLOCK_MASK;
    threadState->prox = threadState->postingsPool->buffers[threadState->p->proxUpto >> BYTE_BLOCK_SHIFT];
    assert (threadState->prox != NULL);

    if (payload != NULL && payload->length() > 0) {
      threadState->writeProxVInt((proxCode<<1)|1);
      threadState->writeProxVInt(payload->length());
      threadState->writeProxBytes(payload->getData().values, payload->getOffset(), payload->length());
      fieldInfo->storePayloads = true;
    } else
      threadState->writeProxVInt(proxCode<<1);

    threadState->p->proxUpto = threadState->proxUpto + (threadState->p->proxUpto & BYTE_BLOCK_NOT_MASK);

    threadState->p->lastPosition = position++;

    if (doVectorPositions) {
      threadState->posUpto = threadState->vector->posUpto & BYTE_BLOCK_MASK;
      threadState->pos = threadState->vectorsPool->buffers[threadState->vector->posUpto >> BYTE_BLOCK_SHIFT];
      threadState->writePosVInt(proxCode);
      threadState->vector->posUpto = threadState->posUpto + (threadState->vector->posUpto & BYTE_BLOCK_NOT_MASK);
    }

    if (doVectorOffsets) {
      threadState->offsetUpto = threadState->vector->offsetUpto & BYTE_BLOCK_MASK;
      threadState->offsets = threadState->vectorsPool->buffers[threadState->vector->offsetUpto >> BYTE_BLOCK_SHIFT];
      threadState->writeOffsetVInt(offsetStartCode);
      threadState->writeOffsetVInt(offsetEnd-offsetStart);
      threadState->vector->lastOffset = offsetEnd;
      threadState->vector->offsetUpto = threadState->offsetUpto + (threadState->vector->offsetUpto & BYTE_BLOCK_NOT_MASK);
    }
  } catch (CLuceneError& t) {
    throw AbortException(t, _parent);
  }
}

void DocumentsWriter::ThreadState::FieldData::rehashPostings(const int32_t newSize) {

  const int32_t newMask = newSize-1;

  ValueArray<Posting*> newHash(newSize);
  int32_t hashPos, code;
  const TCHAR* pos = NULL;
  const TCHAR* start = NULL;
  Posting* p0;

  for(int32_t i=0;i<postingsHashSize;i++) {
    p0 = postingsHash[i];
    if (p0 != NULL) {
      start = threadState->charPool->buffers[p0->textStart >> CHAR_BLOCK_SHIFT] + (p0->textStart & CHAR_BLOCK_MASK);
      pos = start;
      while( *pos != CLUCENE_END_OF_WORD)
        pos++;
      code = 0;
      while (pos > start)
        code = (code*31) + *--pos;

      hashPos = code & newMask;
      assert (hashPos >= 0);
      if (newHash[hashPos] != NULL) {
        const int32_t inc = ((code>>8)+code)|1;
        do {
          code += inc;
          hashPos = code & newMask;
        } while (newHash[hashPos] != NULL);
      }
      newHash.values[hashPos] = p0;
    }
  }

  postingsHashMask =  newMask;
  postingsHash.deleteArray();
  postingsHash.length = newHash.length;
  postingsHash.values = newHash.takeArray();
  postingsHashSize = newSize;
  postingsHashHalfSize = newSize >> 1;
}

void DocumentsWriter::ThreadState::FieldData::writeVectors(FieldInfo* fieldInfo) {
  assert (fieldInfo->storeTermVector);

  threadState->vectorFieldNumbers.values[threadState->numVectorFields] = fieldInfo->number;
  threadState->vectorFieldPointers.values[threadState->numVectorFields] = threadState->tvfLocal->getFilePointer();
  threadState->numVectorFields++;

  const int32_t numPostingsVectors = postingsVectorsUpto;

  threadState->tvfLocal->writeVInt(numPostingsVectors);
  uint8_t bits = 0x0;
  if (doVectorPositions)
    bits |= TermVectorsReader::STORE_POSITIONS_WITH_TERMVECTOR;
  if (doVectorOffsets)
    bits |= TermVectorsReader::STORE_OFFSET_WITH_TERMVECTOR;
  threadState->tvfLocal->writeByte(bits);

  threadState->doVectorSort(threadState->postingsVectors, numPostingsVectors);

  Posting* lastPosting = NULL;

  ByteSliceReader* reader = vectorSliceReader;

  for(int32_t j=0;j<numPostingsVectors;j++) {
    PostingVector* vector = threadState->postingsVectors[j];
    Posting* posting = vector->p;
    const int32_t freq = posting->docFreq;

    int32_t prefix = 0;
    const TCHAR* text2 = threadState->charPool->buffers[posting->textStart >> CHAR_BLOCK_SHIFT];
    const TCHAR* start2 = text2 + (posting->textStart & CHAR_BLOCK_MASK);
    const TCHAR* pos2 = start2;

    // Compute common prefix between last term and
    // this term
    if (lastPosting == NULL)
      prefix = 0;
    else {
      const TCHAR* text1 = threadState->charPool->buffers[lastPosting->textStart >> CHAR_BLOCK_SHIFT];
      const TCHAR* start1 = text1 + (lastPosting->textStart & CHAR_BLOCK_MASK);
      const TCHAR* pos1 = start1;
      while(true) {
        if (*pos1 != *pos2 || *pos1 == CLUCENE_END_OF_WORD) {
          prefix = pos1-start1;
          break;
        }
        pos1++;
        pos2++;
      }
    }
    lastPosting = posting;

    // Compute length
    while(*pos2 != CLUCENE_END_OF_WORD)
      pos2++;

    const int32_t suffix = pos2 - start2 - prefix;
    threadState->tvfLocal->writeVInt(prefix);
    threadState->tvfLocal->writeVInt(suffix);
    threadState->tvfLocal->writeChars(start2 + prefix, suffix);
    threadState->tvfLocal->writeVInt(freq);

    if (doVectorPositions) {
      reader->init(threadState->vectorsPool, vector->posStart, vector->posUpto);
      reader->writeTo(threadState->tvfLocal);
    }

    if (doVectorOffsets) {
      reader->init(threadState->vectorsPool, vector->offsetStart, vector->offsetUpto);
      reader->writeTo(threadState->tvfLocal);
    }
  }
}

CL_NS_END
