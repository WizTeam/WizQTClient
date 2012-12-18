/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CachingSpanFilter.h"
#include "CLucene/index/IndexReader.h"


CL_NS_DEF(search)

/**
 * Result wrapper for the cache
 */
class ResultHolder : LUCENE_BASE
{
	bool deleteResult;
public:
	SpanFilterResult * result;
	
	ResultHolder(SpanFilterResult * result, bool deleteResult )
    {
		this->result = result;
		this->deleteResult = deleteResult;
	}
	~ResultHolder()
    {
		if ( deleteResult )
			_CLDELETE( result );
	}	
};

struct CachingSpanFilter::Internal
{
	typedef CL_NS(util)::CLHashMap<
        CL_NS(index)::IndexReader *, 
        ResultHolder *, 
        CL_NS(util)::Compare::Void<CL_NS(index)::IndexReader>,
        CL_NS(util)::Equals::Void<CL_NS(index)::IndexReader>,
        CL_NS(util)::Deletor::Object<CL_NS(index)::IndexReader>, 
        CL_NS(util)::Deletor::Object<ResultHolder> 
    > ResultCacheType; 

	ResultCacheType     cache;
	DEFINE_MUTEX(cache_LOCK)

	Internal() : cache(false,true)
	{}
};

CachingSpanFilter::CachingSpanFilter( SpanFilter * filter, bool deleteFilter )
{
    _internal = _CLNEW Internal();
    this->filter = filter;
    this->deleteFilter = deleteFilter;
}

CachingSpanFilter::CachingSpanFilter( const CachingSpanFilter& copy )
{
    _internal = _CLNEW Internal();
	this->filter = (SpanFilter*)copy.filter->clone();
    this->deleteFilter = true;
}

CachingSpanFilter::~CachingSpanFilter()
{
    _CLDELETE( _internal );
    if( deleteFilter )
    {
        _CLDELETE( filter );
    }
    else
        filter = NULL;
}

Filter* CachingSpanFilter::clone() const 
{
	return _CLNEW CachingSpanFilter( *this );
}

CL_NS(util)::BitSet* CachingSpanFilter::bits( CL_NS(index)::IndexReader * reader )
{
    SpanFilterResult * result = getCachedResult( reader );
    return result != NULL ? result->getBits() : NULL;
}

SpanFilterResult * CachingSpanFilter::getCachedResult( CL_NS(index)::IndexReader * reader )
{
	SCOPED_LOCK_MUTEX( _internal->cache_LOCK )

    ResultHolder * resultHolder = _internal->cache.get( reader );
    if( ! resultHolder )
    {
        SpanFilterResult * result = filter->bitSpans( reader );
        resultHolder = _CLNEW ResultHolder( result, true );
        _internal->cache.put( reader, resultHolder );
    }

    return resultHolder->result;
}

SpanFilterResult * CachingSpanFilter::bitSpans( CL_NS(index)::IndexReader * reader )
{
    return getCachedResult( reader );
}

TCHAR* CachingSpanFilter::toString()
{
	TCHAR* ft = filter->toString();
	size_t len = _tcslen( ft ) + 20;
	TCHAR* ret = _CL_NEWARRAY( TCHAR, len );
	ret[0] = 0;
	_sntprintf( ret, len, _T( "CachingSpanFilter(%s)" ), ft );
	_CLDELETE_CARRAY( ft );
	return ret;
}

CL_NS_END