/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_analysis_CachingTokenFilter
#define _lucene_analysis_CachingTokenFilter

#include <list>
#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_DEF(analysis)

class CLUCENE_EXPORT CachingTokenFilter : public TokenFilter 
{
private:
    bool                            bCacheInitialized;
    std::list<Token *>              cache;
    std::list<Token *>::iterator    itCache;
  
public:
    CachingTokenFilter( TokenStream * input, bool bDeleteTS );
    virtual ~CachingTokenFilter();

    Token * next( Token* t );
    void reset();
    void fillCache();
};

CL_NS_END

#endif // _lucene_analysis_CachingTokenFilter
