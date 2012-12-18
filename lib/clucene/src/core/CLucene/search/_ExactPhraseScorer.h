/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_ExactPhraseScorer_
#define _lucene_search_ExactPhraseScorer_

#include "_PhraseScorer.h"

CL_NS_DEF(search)

  class ExactPhraseScorer: public PhraseScorer {
    public:
    ExactPhraseScorer(Weight* weight, CL_NS(index)::TermPositions** tps, int32_t* offsets, 
       Similarity* similarity, uint8_t* norms );
       
	virtual ~ExactPhraseScorer();

	virtual TCHAR* toString();

    protected:
      //Returns the exact freqency of the phrase
      float_t phraseFreq();
    };
CL_NS_END
#endif
