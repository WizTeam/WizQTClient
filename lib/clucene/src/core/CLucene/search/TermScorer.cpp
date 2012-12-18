/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_TermScorer.h"
#include "SearchHeader.h"
#include "Explanation.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "TermQuery.h"
#include "Similarity.h"
#include "Explanation.h"

CL_NS_USE(index)
CL_NS_DEF(search)

	TermScorer::TermScorer(Weight* w, CL_NS(index)::TermDocs* td, 
			Similarity* similarity,uint8_t* _norms):
	    Scorer(similarity),
	    termDocs(td),
	    norms(_norms),
	    weight(w),
	    weightValue(w->getValue()),
	    _doc(0),
	    pointer(0),
	    pointerMax(0)
	{
		memset(docs,0,32*sizeof(int32_t));
		memset(freqs,0,32*sizeof(int32_t));

		for (int32_t i = 0; i < LUCENE_SCORE_CACHE_SIZE; i++)
			scoreCache[i] = getSimilarity()->tf(i) * weightValue;
	}

	TermScorer::~TermScorer(){
		_CLLDELETE(termDocs);
	}
  bool TermScorer::next(){
    pointer++;
    if (pointer >= pointerMax) {
      pointerMax = termDocs->read(docs, freqs, 32);    // refill buffer
      if (pointerMax != 0) {
        pointer = 0;
      } else {
        termDocs->close();			  // close stream
        _doc = LUCENE_INT32_MAX_SHOULDBE;		  // set to sentinel value
        return false;
      }
    } 
    _doc = docs[pointer];
    return true;
  }

  bool TermScorer::skipTo(int32_t target) {
    // first scan in cache
    for (pointer++; pointer < pointerMax; pointer++) {
      if (docs[pointer] >= target) {
        _doc = docs[pointer];
        return true;
      }
    }

    // not found in cache, seek underlying stream
    bool result = termDocs->skipTo(target);
      if (result) {
         pointerMax = 1;
         pointer = 0;
         docs[pointer] = _doc = termDocs->doc();
         freqs[pointer] = termDocs->freq();
      } else {
         _doc = LUCENE_INT32_MAX_SHOULDBE;
      }
      return result;
  }

  Explanation* TermScorer::explain(int32_t doc) {
    TermQuery* query = (TermQuery*)weight->getQuery();
	Explanation* tfExplanation = _CLNEW Explanation();
    int32_t tf = 0;
    while (pointer < pointerMax) {
      if (docs[pointer] == doc)
        tf = freqs[pointer];
      pointer++;
    }
    if (tf == 0) {
      if (termDocs->skipTo(doc)) {
        if (termDocs->doc() == doc) {
          tf = termDocs->freq();
        }
      }
    }
    termDocs->close();
    tfExplanation->setValue(getSimilarity()->tf(tf));

    TCHAR buf[LUCENE_SEARCH_EXPLANATION_DESC_LEN+1];
	TCHAR* termToString = query->getTerm(false)->toString();
	_sntprintf(buf,LUCENE_SEARCH_EXPLANATION_DESC_LEN,_T("tf(termFreq(%s)=%d)"), termToString, tf);
    _CLDELETE_LCARRAY(termToString);
    tfExplanation->setDescription(buf);
	return tfExplanation;
  }

  TCHAR* TermScorer::toString() { 
     TCHAR* wb = weight->toString();
     int32_t rl = _tcslen(wb) + 9; //9=_tcslen("scorer("  ")") + 1
     TCHAR* ret = _CL_NEWARRAY(TCHAR,rl);
	 _sntprintf(ret,rl,_T("scorer(%s)"), wb);
     _CLDELETE_LCARRAY(wb);
     return ret;
  }

  float_t TermScorer::score(){
	 int32_t f = freqs[pointer];
    float_t raw =                                   // compute tf(f)*weight
      f < LUCENE_SCORE_CACHE_SIZE			  // check cache
      ? scoreCache[f]                             // cache hit
      : getSimilarity()->tf(f) * weightValue;        // cache miss

      return raw * Similarity::decodeNorm(norms[_doc]); // normalize for field
  }

  int32_t TermScorer::doc() const { return _doc; }
	
CL_NS_END
