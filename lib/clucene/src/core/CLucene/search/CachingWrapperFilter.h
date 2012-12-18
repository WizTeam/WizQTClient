/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_CachingWrapperFilter_
#define _lucene_search_CachingWrapperFilter_

//#include "CLucene/util/BitSet.h"
//#include "CLucene/index/IndexReader.h"
#include "Filter.h"

CL_NS_DEF(search)
/**
 * Wraps another filter's result and caches it.  The purpose is to allow
 * filters to implement this and allow itself to be cached. Alternatively,
 * use the CachingWrapperFilter to cache the filter.
 */
class CLUCENE_EXPORT AbstractCachingFilter: public Filter
{
	struct Internal;
	Internal* _internal;
	void closeCallback(CL_NS(index)::IndexReader* reader, void* param);
protected:
	AbstractCachingFilter( const AbstractCachingFilter& copy );
	virtual CL_NS(util)::BitSet* doBits( CL_NS(index)::IndexReader* reader ) = 0;
	virtual bool doShouldDeleteBitSet( CL_NS(util)::BitSet* /*bits*/ ){ return false; }
	AbstractCachingFilter();
public:
	virtual ~AbstractCachingFilter();

	/** Returns a BitSet with true for documents which should be permitted in
	search results, and false for those that should not. */
	CL_NS(util)::BitSet* bits( CL_NS(index)::IndexReader* reader );

	virtual Filter *clone() const = 0;
	virtual TCHAR *toString() = 0;

	bool shouldDeleteBitSet( const CL_NS(util)::BitSet* /*bits*/ ) const{ return false; }
};

/**
 * Wraps another filter's result and caches it.  The purpose is to allow
 * filters to simply filter, and then wrap with this class to add
 * caching, keeping the two concerns decoupled yet composable.
 */
class CLUCENE_EXPORT CachingWrapperFilter: public AbstractCachingFilter
{
private:
	Filter* filter;
	bool deleteFilter;
protected:
	CachingWrapperFilter( const CachingWrapperFilter& copy );
	CL_NS(util)::BitSet* doBits( CL_NS(index)::IndexReader* reader );
	bool doShouldDeleteBitSet( CL_NS(util)::BitSet* bits );
public:
	CachingWrapperFilter( Filter* filter, bool deleteFilter=true );
	~CachingWrapperFilter();

	Filter *clone() const;
	TCHAR *toString();
};

CL_NS_END
#endif
