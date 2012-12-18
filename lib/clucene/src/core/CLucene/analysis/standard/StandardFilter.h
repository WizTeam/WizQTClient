/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_standard_StandardFilter
#define _lucene_analysis_standard_StandardFilter


#include "CLucene/analysis/AnalysisHeader.h"
//#include "../Analyzers.h"
//#include "StandardTokenizerConstants.h"

CL_CLASS_DEF(util,StringBuffer)

CL_NS_DEF2(analysis,standard)

	/** Normalizes tokens extracted with {@link lucene::analysis::standard::StandardTokenizer}. */
	class CLUCENE_EXPORT StandardFilter: public TokenFilter{
	public:
		// Construct filtering <i>in</i>. 
		StandardFilter(TokenStream* in, bool deleteTokenStream);

		virtual ~StandardFilter();

		
	  /** Returns the next token in the stream, or NULL at EOS.
	  * <p>Removes <tt>'s</tt> from the end of words.
	  * <p>Removes dots from acronyms.
	  */
		Token* next(Token* token);
	};
CL_NS_END2
#endif
