/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/util/StringBuffer.h"
#include "GermanStemmer.h"
#include "GermanStemFilter.h"

CL_NS_USE(analysis)
CL_NS_USE2(analysis,de)

    GermanStemFilter::GermanStemFilter(TokenStream* in, bool deleteTS) :
      TokenFilter(in, deleteTS)
    {
      stemmer = _CLNEW GermanStemmer();
      exclusionSet = NULL;
    }

    GermanStemFilter::GermanStemFilter(TokenStream* in, bool deleteTS, CLTCSetList* exclusionSet) :
      TokenFilter(in, deleteTS)
    {
      stemmer = _CLNEW GermanStemmer();
      this->exclusionSet = exclusionSet;
    }

    Token* GermanStemFilter::next(Token* t) {
      if (input->next(t) == NULL) {
        return NULL;
      } else if (exclusionSet != NULL && exclusionSet->find(t->termBuffer()) != exclusionSet->end()) { // Check the exclusiontable
        return t;
      } else {
        TCHAR* s = stemmer->stem(t->termBuffer(), t->termLength());
        // If not stemmed, dont waste the time creating a new token
        if (_tcscmp(s, t->termBuffer()) != 0) {
          t->setText(s);
        }
        return t;
      }
    }

    void GermanStemFilter::setStemmer(GermanStemmer* stemmer) {
      if (stemmer != NULL && this->stemmer != stemmer) {
        _CLLDELETE(this->stemmer);
        this->stemmer = stemmer;
      }
    }

    /**
     * Set an alternative exclusion list for this filter.
     */
    void GermanStemFilter::setExclusionSet(CLTCSetList* exclusionSet) {
      if (this->exclusionSet != exclusionSet) {
        _CLLDELETE(exclusionSet);
        this->exclusionSet = exclusionSet;
      }
    }
