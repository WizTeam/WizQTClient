/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_languagebasedanalyzer_
#define _lucene_analysis_languagebasedanalyzer_

#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_DEF(analysis)

class CLUCENE_CONTRIBS_EXPORT LanguageBasedAnalyzer: public CL_NS(analysis)::Analyzer{
	TCHAR lang[100];
	bool stem;
public:
	LanguageBasedAnalyzer(const TCHAR* language=NULL, bool stem=true);
	~LanguageBasedAnalyzer();
	void setLanguage(const TCHAR* language);
	void setStem(bool stem);
	TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
  };

CL_NS_END
#endif
