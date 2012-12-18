/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_de_GermanStemFilter
#define _lucene_analysis_de_GermanStemFilter

CL_NS_DEF2(analysis,de)

/**
 * A filter that stems German words. It supports a table of words that should
 * not be stemmed at all. The stemmer used can be changed at runtime after the
 * filter object is created (as long as it is a GermanStemmer).
 */
class CLUCENE_CONTRIBS_EXPORT GermanStemFilter : public CL_NS(analysis)::TokenFilter
{
private:

    /**
     * The actual token in the input stream.
     */
    CL_NS(analysis)::Token* token;
    GermanStemmer* stemmer;
    CL_NS(analysis)::CLTCSetList* exclusionSet;

public:

    GermanStemFilter(TokenStream* in, bool deleteTS = false);

    /**
     * Builds a GermanStemFilter that uses an exclusiontable.
     */
    GermanStemFilter(TokenStream* in, bool deleteTS, CL_NS(analysis)::CLTCSetList* exclusionSet);

    /**
     * @return  Returns the next token in the stream, or null at EOS
     */
    virtual Token* next(Token* t);

    /**
     * Set a alternative/custom GermanStemmer for this filter.
     */
    void setStemmer(GermanStemmer* stemmer);

    /**
     * Set an alternative exclusion list for this filter.
     */
   void setExclusionSet(CL_NS(analysis)::CLTCSetList* exclusionSet);
};

CL_NS_END2
#endif
