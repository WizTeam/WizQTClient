/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CachingTokenFilter.h"

CL_NS_DEF(analysis)

CachingTokenFilter::CachingTokenFilter( TokenStream * input, bool bDeleteTS ) 
: TokenFilter( input, bDeleteTS )
{
    bCacheInitialized = false;
}

CachingTokenFilter::~CachingTokenFilter()
{
    for( itCache = cache.begin(); itCache != cache.end(); itCache++ )
        delete (*itCache);

    cache.clear();
}

Token * CachingTokenFilter::next( Token* t )
{
    if( ! bCacheInitialized )
    {
        fillCache();
        bCacheInitialized = true;
        itCache = cache.begin();
    }
    
    // if the cache is exhausted, return null
    if( itCache == cache.end() )
        return NULL;

    t->set( (*itCache)->termBuffer(), (*itCache)->startOffset(), (*itCache)->endOffset(), (*itCache)->type() );
    t->setPositionIncrement( (*itCache)->getPositionIncrement() );
    t->setPayload( (*itCache)->getPayload() );
    itCache++;

    return t;
}
  
void CachingTokenFilter::reset()
{
    itCache = cache.begin();
}
  
void CachingTokenFilter::fillCache()
{
    Token token;
    Token * pCopy;

    while( input->next( &token ) )
    {
        pCopy = new Token( token.termBuffer(), token.startOffset(), token.endOffset(), token.type() );
        pCopy ->setPositionIncrement( token.getPositionIncrement() );
        pCopy ->setPayload( token.getPayload() );
        cache.push_back( pCopy );
    }
}

CL_NS_END