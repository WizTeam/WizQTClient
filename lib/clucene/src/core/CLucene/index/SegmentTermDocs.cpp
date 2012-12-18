/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_SegmentHeader.h"

#include "CLucene/store/IndexInput.h"
#include "Term.h"
#include <assert.h>

CL_NS_DEF(index)

  SegmentTermDocs::SegmentTermDocs(const SegmentReader* _parent) : parent(_parent),freqStream(_parent->freqStream->clone()),
		count(0),df(0),deletedDocs(_parent->deletedDocs),_doc(0),_freq(0),skipInterval(_parent->tis->getSkipInterval()),
		maxSkipLevels(_parent->tis->getMaxSkipLevels()),skipListReader(NULL),freqBasePointer(0),proxBasePointer(0),
		skipPointer(0),haveSkipped(false)
	{
      CND_CONDITION(_parent != NULL,"Parent is NULL");
   }

  SegmentTermDocs::~SegmentTermDocs() {
      close();
  }

  TermPositions* SegmentTermDocs::__asTermPositions(){
	  return NULL;
  }

  void SegmentTermDocs::seek(Term* term) {
    TermInfo* ti = parent->tis->get(term);
    seek(ti, term);
    _CLDELETE(ti);
  }

  void SegmentTermDocs::seek(TermEnum* termEnum){
    TermInfo* ti=NULL;
    Term* term = NULL;

      // use comparison of fieldinfos to verify that termEnum belongs to the same segment as this SegmentTermDocs
    if ( termEnum->getObjectName() == SegmentTermEnum::getClassName() &&
        ((SegmentTermEnum*)termEnum)->fieldInfos == parent->_fieldInfos ){
      SegmentTermEnum* segmentTermEnum = (SegmentTermEnum*) termEnum;
      term = segmentTermEnum->term(false);
      ti = segmentTermEnum->getTermInfo();
    }else{
      term = termEnum->term(false);
      ti = parent->tis->get(term);
    }
    
    seek(ti,term);
    _CLDELETE(ti);
  }
  void SegmentTermDocs::seek(const TermInfo* ti,Term* term) {
	  count = 0;
	  FieldInfo* fi = parent->_fieldInfos->fieldInfo(term->field());
	  currentFieldStoresPayloads = (fi != NULL) ? fi->storePayloads : false;
	  if (ti == NULL) {
		  df = 0;
	  } else {					// punt case
		  df = ti->docFreq;
		  _doc = 0;
		  freqBasePointer = ti->freqPointer;
		  proxBasePointer = ti->proxPointer;
		  skipPointer = freqBasePointer + ti->skipOffset;
		  freqStream->seek(freqBasePointer);
		  haveSkipped = false;
	  }
  }

  void SegmentTermDocs::close() {
	  _CLDELETE( freqStream );
	  _CLDELETE( skipListReader );
  }

  int32_t SegmentTermDocs::doc()const { 
	  return _doc; 
  }
  int32_t SegmentTermDocs::freq()const { 
	  return _freq; 
  }

  bool SegmentTermDocs::next() {
    while (true) {
      if (count == df)
        return false;

      uint32_t docCode = freqStream->readVInt();
      _doc += docCode >> 1; //unsigned shift
      if ((docCode & 1) != 0)			  // if low bit is set
        _freq = 1;				  // _freq is one
      else
        _freq = freqStream->readVInt();		  // else read _freq
      count++;

      if ( (deletedDocs == NULL) || (_doc >= 0 && deletedDocs->get(_doc) == false ) )
        break;
      skippingDoc();
    }
    return true;
  }

  int32_t SegmentTermDocs::read(int32_t* docs, int32_t* freqs, int32_t length) {
	  int32_t i = 0;
	  //todo: one optimization would be to get the pointer buffer for ram or mmap dirs 
	  //and iterate over them instead of using readByte() intensive functions.
	  while (i<length && count < df) {
		  // manually inlined call to next() for speed
		  uint32_t docCode = freqStream->readVInt();
		  _doc += docCode >> 1;
		  if ((docCode & 1) != 0)			  // if low bit is set
			  _freq = 1;				  // _freq is one
		  else
			  _freq = freqStream->readVInt();		  // else read _freq
		  count++;

		  if (deletedDocs == NULL || (_doc >= 0 && !deletedDocs->get(_doc))) {
			  docs[i] = _doc;
			  freqs[i] = _freq;
			  i++;
		  }
	  }
	  return i;
  }

  bool SegmentTermDocs::skipTo(const int32_t target){
    assert(count <= df );
    
    if (df >= skipInterval) {                      // optimized case
      if (skipListReader == NULL)
		  skipListReader = _CLNEW DefaultSkipListReader(freqStream->clone(), maxSkipLevels, skipInterval); // lazily clone

	  if (!haveSkipped) {                          // lazily initialize skip stream
		  skipListReader->init(skipPointer, freqBasePointer, proxBasePointer, df, currentFieldStoresPayloads);
		  haveSkipped = true;
	  }

      int32_t newCount = skipListReader->skipTo(target); 
      if (newCount > count) {
        freqStream->seek(skipListReader->getFreqPointer());
        skipProx(skipListReader->getProxPointer(), skipListReader->getPayloadLength());

        _doc = skipListReader->getDoc();
        count = newCount;
      }      
	}

    // done skipping, now just scan
    do {
      if (!next())
        return false;
    } while (target > _doc);
    return true;
  }


CL_NS_END
