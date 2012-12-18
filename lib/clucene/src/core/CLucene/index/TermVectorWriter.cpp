/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_TermVector.h"
#include "_IndexFileNames.h"
#include "CLucene/util/Misc.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/store/IndexOutput.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_DEF(index)

 TermVectorsWriter::TermVectorsWriter(CL_NS(store)::Directory* directory,
    const char* segment,FieldInfos* fieldInfos)
 {
    // Open files for TermVector storage
    char fbuf[CL_MAX_NAME];
    strcpy(fbuf,segment);
    strcat(fbuf,".");
    char* fpbuf=fbuf+strlen(fbuf);

    strcpy(fpbuf,IndexFileNames::VECTORS_INDEX_EXTENSION);
    tvx = directory->createOutput(fbuf);
    tvx->writeInt(TermVectorsReader::FORMAT_VERSION);

    strcpy(fpbuf,IndexFileNames::VECTORS_DOCUMENTS_EXTENSION);
    tvd = directory->createOutput(fbuf);
    tvd->writeInt(TermVectorsReader::FORMAT_VERSION);

    strcpy(fpbuf,IndexFileNames::VECTORS_FIELDS_EXTENSION);
    tvf = directory->createOutput(fbuf);
    tvf->writeInt(TermVectorsReader::FORMAT_VERSION);

    this->fieldInfos = fieldInfos;
  }

  void TermVectorsWriter::close(CLuceneError* err){
    CLuceneError keep;
    bool bError = false;

    if ( tvx != NULL ){
      try{
        tvx->close();
      }catch(CLuceneError& ioerr){
        if ( ioerr.number() != CL_ERR_IO )
        {
            _CLDELETE(tvx);
            _CLDELETE(tvd);
            _CLDELETE(tvf);
            throw ioerr;
        }
        if (!bError)
        {
            bError = true;
            keep.set(ioerr.number(), ioerr.what());
        }
      }
      _CLDELETE(tvx);
    }

    if ( tvd != NULL ){
      try{
        tvd->close();
      }catch(CLuceneError& ioerr){
        if ( ioerr.number() != CL_ERR_IO )
        {
            _CLDELETE(tvd);
            _CLDELETE(tvf);
            throw ioerr;
        }
        if (!bError)
        {
            bError = true;
            keep.set(ioerr.number(), ioerr.what());
        }
      }
      _CLDELETE(tvd);
    }

    if ( tvf != NULL ){
      try{
        tvf->close();
      }catch(CLuceneError& ioerr){
        if ( ioerr.number() != CL_ERR_IO )
        {
            _CLDELETE(tvf);
            throw ioerr;
        }
        if (!bError)
        {
            bError = true;
            keep.set(ioerr.number(), ioerr.what());
        }
      }
      _CLDELETE(tvf);
    }

    if (bError)
    {
        if ( err != NULL )
            err->set(keep.number(), keep.what());
        else 
            throw keep;
    }
  }

  TermVectorsWriter::~TermVectorsWriter(){
    CLuceneError err;
    close(&err);
  }


  void TermVectorsWriter::addAllDocVectors(ArrayBase<TermFreqVector*>* _vectors){

    tvx->writeLong(tvd->getFilePointer());

    if (_vectors != NULL) {
      ArrayBase<TermFreqVector*>& vectors = *_vectors;

      const int32_t numFields = vectors.length;
      tvd->writeVInt(numFields);

      ValueArray<int64_t> fieldPointers(numFields);

      for (int32_t i=0; i<numFields; i++) {
        fieldPointers[i] = tvf->getFilePointer();

        const int32_t fieldNumber = fieldInfos->fieldNumber(vectors[i]->getField());

        // 1st pass: write field numbers to tvd
        tvd->writeVInt(fieldNumber);

        const int32_t numTerms = vectors[i]->size();
        tvf->writeVInt(numTerms);

        TermPositionVector* tpVector = NULL;

        uint8_t bits = 0;
        bool storePositions = false;
        bool storeOffsets = false;

        if ( vectors[i]->__asTermPositionVector() != NULL ) {
          // May have positions & offsets
          tpVector = vectors[i]->__asTermPositionVector();
          storePositions = tpVector->size() > 0 && tpVector->getTermPositions(0) != NULL;
          storeOffsets = tpVector->size() > 0 && tpVector->getOffsets(0) != NULL;
          bits = ((storePositions ? TermVectorsReader::STORE_POSITIONS_WITH_TERMVECTOR : 0) +
                         (storeOffsets ? TermVectorsReader::STORE_OFFSET_WITH_TERMVECTOR : 0));
        } else {
          tpVector = NULL;
          bits = 0;
          storePositions = false;
          storeOffsets = false;
        }

        tvf->writeVInt(bits);

        const ArrayBase<const TCHAR*>& terms = *vectors[i]->getTerms();
        const ArrayBase<int32_t>& freqs = *vectors[i]->getTermFrequencies();

        const TCHAR* lastTermText = LUCENE_BLANK_STRING;
        size_t lastTermTextLen = 0;

        for (int32_t j=0; j<numTerms; j++) {
          const TCHAR* termText = terms[j];
          size_t termTextLen = _tcslen(termText);
          int32_t start = Misc::stringDifference(lastTermText, lastTermTextLen, termText, termTextLen);
          int32_t length = termTextLen - start;
          tvf->writeVInt(start);       // write shared prefix length
          tvf->writeVInt(length);        // write delta length
          tvf->writeChars(termText + start, length);  // write delta chars
          lastTermText = termText;

          const int32_t termFreq = freqs[j];

          tvf->writeVInt(termFreq);

          if (storePositions) {
            const ArrayBase<int32_t>* _positions = tpVector->getTermPositions(j);
            if (_positions == NULL)
              _CLTHROWA(CL_ERR_IllegalState, "Trying to write positions that are NULL!");
            const ArrayBase<int32_t>& positions = *_positions;
            assert (positions.length == termFreq);

            // use delta encoding for positions
            int32_t lastPosition = 0;
            for(int32_t k=0;k<positions.length;k++) {
              const int32_t position = positions[k];
              tvf->writeVInt(position-lastPosition);
              lastPosition = position;
            }
          }

          if (storeOffsets) {
            const ArrayBase<TermVectorOffsetInfo*>* _offsets = tpVector->getOffsets(j);
            if (_offsets == NULL)
              _CLTHROWA(CL_ERR_IllegalState, "Trying to write offsets that are NULL!");
            const ArrayBase<TermVectorOffsetInfo*>& offsets = *_offsets;
            assert (offsets.length == termFreq);

            // use delta encoding for offsets
            int32_t lastEndOffset = 0;
            for(int k=0;k<offsets.length;k++) {
              const int32_t startOffset = offsets[k]->getStartOffset();
              const int32_t endOffset = offsets[k]->getEndOffset();
              tvf->writeVInt(startOffset-lastEndOffset);
              tvf->writeVInt(endOffset-startOffset);
              lastEndOffset = endOffset;
            }
          }
        }
      }

      // 2nd pass: write field pointers to tvd
      int64_t lastFieldPointer = 0;
      for (int32_t i=0; i<numFields; i++) {
        const int64_t fieldPointer = fieldPointers[i];
        tvd->writeVLong(fieldPointer-lastFieldPointer);
        lastFieldPointer = fieldPointer;
      }
    } else
      tvd->writeVInt(0);
  }

CL_NS_END
