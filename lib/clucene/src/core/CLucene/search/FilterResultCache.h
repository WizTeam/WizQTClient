/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_FilterResultCache_
#define _lucene_search_FilterResultCache_

CL_CLASS_DEF(index,IndexReader)

CL_NS_DEF(search)

/**
 * Holds one cached result
 */
template<class T>
class ResultHolder : LUCENE_BASE
{
	bool deleteRes;
public:
	T* result;
	
	ResultHolder( T * result, bool deleteRes )
    {
		this->result = result;
		this->deleteRes = deleteRes;
	}
	~ResultHolder(){
		if ( deleteRes )
			_CLDELETE( result );
	}	
};

/**
 * Wraps another filter's result and caches it.  The purpose is to allow
 * filters to implement this and allow itself to be cached. Alternatively,
 * use the CachingWrapperFilter to cache the filter.
 */
template<class T>
class FilterResultCache
{
	typedef CL_NS(util)::CLHashMap<
        CL_NS(index)::IndexReader*, 
        BitSetHolder*, 
        CL_NS(util)::Compare::Void<CL_NS(index)::IndexReader>,
        CL_NS(util)::Equals::Void<CL_NS(index)::IndexReader>,
        CL_NS(util)::Deletor::Object<CL_NS(index)::IndexReader>, 
        CL_NS(util)::Deletor::Object<ResultHolder<T> > 
    > CacheType; 

	CacheType   cache;
    bool        bDeleteResults;
	DEFINE_MUTEX( cache_LOCK )


public:
	FilterResultCache( bool bDeleteResults );
    virtual ~FilterResultCache();

    T* getResult( CL_NS(index)::IndexReader* reader );

protected:
    void closeCallback( CL_NS(index)::IndexReader* reader, void* param );
};

CL_NS_END
#endif
