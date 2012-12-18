/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_SkipListWriter.h"
#include "CLucene/util/_Arrays.h"

CL_NS_USE(store)
CL_NS_USE(util)
CL_NS_DEF(index)

void MultiLevelSkipListWriter::bufferSkip(int32_t df){
  int32_t numLevels;
 
  // determine max level
  for (numLevels = 0; (df % skipInterval) == 0 && numLevels < numberOfSkipLevels; df /= skipInterval) {
    numLevels++;
  }
  
  int64_t childPointer = 0;
  
  for (int32_t level = 0; level < numLevels; level++) {
    writeSkipData(level, (*skipBuffer)[level]);
    
    int64_t newChildPointer = (*skipBuffer)[level]->getFilePointer();
    
    if (level != 0) {
      // store child pointers for all levels except the lowest
      (*skipBuffer)[level]->writeVLong(childPointer);
    }
    
    //remember the childPointer for the next level
    childPointer = newChildPointer;
  }
}

int64_t MultiLevelSkipListWriter::writeSkip(IndexOutput* output){
  int64_t skipPointer = output->getFilePointer();
  if (skipBuffer == NULL || skipBuffer->length == 0) return skipPointer;
  
  for (int32_t level = numberOfSkipLevels - 1; level > 0; level--) {
    int64_t length = (*skipBuffer)[level]->getFilePointer();
    if (length > 0) {
      output->writeVLong(length);
      (*skipBuffer)[level]->writeTo(output);
    }
  }
  (*skipBuffer)[0]->writeTo(output);
  
  return skipPointer;
}

MultiLevelSkipListWriter::MultiLevelSkipListWriter(int32_t skipInterval, int32_t maxSkipLevels, int32_t df) {
  this->skipBuffer = NULL;
  this->skipInterval = skipInterval;
  
  // calculate the maximum number of skip levels for this document frequency
  numberOfSkipLevels = df == 0 ? 0 : (int32_t) floor(log((float_t)df) / log((float_t)skipInterval));
  
  // make sure it does not exceed maxSkipLevels
  if (numberOfSkipLevels > maxSkipLevels) {
    numberOfSkipLevels = maxSkipLevels;
  }
}
MultiLevelSkipListWriter::~MultiLevelSkipListWriter(){
  _CLDELETE(skipBuffer);
}

void MultiLevelSkipListWriter::init() {
  skipBuffer = _CLNEW CL_NS(util)::ObjectArray<CL_NS(store)::RAMOutputStream>(numberOfSkipLevels);
  for (int32_t i = 0; i < numberOfSkipLevels; i++) {
    skipBuffer->values[i] = _CLNEW RAMOutputStream;
  }
}

void MultiLevelSkipListWriter::resetSkip() {
  // creates new buffers or empties the existing ones
  if (skipBuffer == NULL) {
    init();
  } else {
    for (size_t i = 0; i < skipBuffer->length; i++) {
      (*skipBuffer)[i]->reset();
    }
  }      
}




void DefaultSkipListWriter::setSkipData(int32_t doc, bool storePayloads, int32_t payloadLength) {
  this->curDoc = doc;
  this->curStorePayloads = storePayloads;
  this->curPayloadLength = payloadLength;
  this->curFreqPointer = freqOutput->getFilePointer();
  this->curProxPointer = proxOutput->getFilePointer();
}

void DefaultSkipListWriter::resetSkip() {
  MultiLevelSkipListWriter::resetSkip();
  memset(lastSkipDoc, 0, numberOfSkipLevels * sizeof(int32_t) );
  Arrays<int32_t>::fill(lastSkipPayloadLength, numberOfSkipLevels, -1);  // we don't have to write the first length in the skip list
  Arrays<int64_t>::fill(lastSkipFreqPointer,   numberOfSkipLevels, freqOutput->getFilePointer());
  Arrays<int64_t>::fill(lastSkipProxPointer,   numberOfSkipLevels, proxOutput->getFilePointer());
}

void DefaultSkipListWriter::writeSkipData(int32_t level, IndexOutput* skipBuffer){
  // To efficiently store payloads in the posting lists we do not store the length of
  // every payload. Instead we omit the length for a payload if the previous payload had
  // the same length.
  // However, in order to support skipping the payload length at every skip point must be known.
  // So we use the same length encoding that we use for the posting lists for the skip data as well:
  // Case 1: current field does not store payloads
  //           SkipDatum                 --> DocSkip, FreqSkip, ProxSkip
  //           DocSkip,FreqSkip,ProxSkip --> VInt
  //           DocSkip records the document number before every SkipInterval th  document in TermFreqs. 
  //           Document numbers are represented as differences from the previous value in the sequence.
  // Case 2: current field stores payloads
  //           SkipDatum                 --> DocSkip, PayloadLength?, FreqSkip,ProxSkip
  //           DocSkip,FreqSkip,ProxSkip --> VInt
  //           PayloadLength             --> VInt    
  //         In this case DocSkip/2 is the difference between
  //         the current and the previous value. If DocSkip
  //         is odd, then a PayloadLength encoded as VInt follows,
  //         if DocSkip is even, then it is assumed that the
  //         current payload length equals the length at the previous
  //         skip point
  if (curStorePayloads) {
    int32_t delta = curDoc - lastSkipDoc[level];
    if (curPayloadLength == lastSkipPayloadLength[level]) {
      // the current payload length equals the length at the previous skip point,
      // so we don't store the length again
      skipBuffer->writeVInt(delta * 2);
    } else {
      // the payload length is different from the previous one. We shift the DocSkip, 
      // set the lowest bit and store the current payload length as VInt.
      skipBuffer->writeVInt(delta * 2 + 1);
      skipBuffer->writeVInt(curPayloadLength);
      lastSkipPayloadLength[level] = curPayloadLength;
    }
  } else {
    // current field does not store payloads
    skipBuffer->writeVInt(curDoc - lastSkipDoc[level]);
  }
  skipBuffer->writeVInt((int32_t) (curFreqPointer - lastSkipFreqPointer[level]));
  skipBuffer->writeVInt((int32_t) (curProxPointer - lastSkipProxPointer[level]));

  lastSkipDoc[level] = curDoc;
  //System.out.println("write doc at level " + level + ": " + curDoc);
  
  lastSkipFreqPointer[level] = curFreqPointer;
  lastSkipProxPointer[level] = curProxPointer;
}

DefaultSkipListWriter::DefaultSkipListWriter(int32_t skipInterval, int32_t numberOfSkipLevels, int32_t docCount, IndexOutput* freqOutput, IndexOutput* proxOutput):
  MultiLevelSkipListWriter(skipInterval, numberOfSkipLevels, docCount)
{
  this->freqOutput = freqOutput;
  this->proxOutput = proxOutput;
  this->curDoc = this->curPayloadLength = 0;
  this->curFreqPointer =this->curProxPointer = 0;
  
  lastSkipDoc = _CL_NEWARRAY(int32_t,numberOfSkipLevels);
  lastSkipPayloadLength =  _CL_NEWARRAY(int32_t,numberOfSkipLevels);
  lastSkipFreqPointer =  _CL_NEWARRAY(int64_t,numberOfSkipLevels);
  lastSkipProxPointer =  _CL_NEWARRAY(int64_t,numberOfSkipLevels);
}
DefaultSkipListWriter::~DefaultSkipListWriter(){
  _CLDELETE_ARRAY(lastSkipDoc);
  _CLDELETE_ARRAY(lastSkipPayloadLength);
  _CLDELETE_ARRAY(lastSkipFreqPointer);
  _CLDELETE_ARRAY(lastSkipProxPointer);
}
CL_NS_END
