/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_snowball_filter_
#define _lucene_analysis_snowball_filter_

#include "CLucene/analysis/AnalysisHeader.h"
#include "libstemmer.h"

CL_NS_DEF2(analysis,snowball)

/** A filter that stems words using a Snowball-generated stemmer.
 *
 * Available stemmers are listed in {@link net.sf.snowball.ext}.  The name of a
 * stemmer is the part of the class name before "Stemmer", e.g., the stemmer in
 * {@link EnglishStemmer} is named "English".
 *
 * Note: todo: This is not thread safe...
 */
class CLUCENE_CONTRIBS_EXPORT SnowballFilter: public TokenFilter {
	struct sb_stemmer * stemmer;
public:

  /** Construct the named stemming filter.
   *
   * @param in the input tokens to stem
   * @param name the name of a stemmer
   */
	SnowballFilter(TokenStream* in, const TCHAR* language, bool deleteTS);

	~SnowballFilter();

    /** Returns the next input Token, after being stemmed */
    Token* next(Token* token);
};

CL_NS_END2
#endif
