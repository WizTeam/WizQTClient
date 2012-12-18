/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_CachingSpanFilter_
#define _lucene_search_CachingSpanFilter_

#include "SpanFilter.h"

CL_NS_DEF(search)

/**
 * Wraps another SpanFilter's result and caches it.  The purpose is to allow
 * filters to simply filter, and then wrap with this class to add caching.
 */
class  CachingSpanFilter : public SpanFilter 
{
	struct Internal;
	Internal* _internal;

protected:
    SpanFilter *        filter;
    bool                deleteFilter;

protected:
    CachingSpanFilter( const CachingSpanFilter& copy );

public:
    /**
     * @param filter Filter to cache results of
     */
    CachingSpanFilter( SpanFilter * filter, bool deleteFilter=true );

    virtual ~CachingSpanFilter();

    virtual Filter* clone() const;

    virtual CL_NS(util)::BitSet* bits( CL_NS(index)::IndexReader * reader );

    virtual SpanFilterResult * bitSpans( CL_NS(index)::IndexReader * reader );

    virtual TCHAR* toString();

private:
    SpanFilterResult * getCachedResult( CL_NS(index)::IndexReader * reader );

//     public boolean equals(Object o) {
//       if (!(o instanceof CachingSpanFilter)) return false;
//       return this.filter.equals(((CachingSpanFilter)o).filter);
//     }
//   
//     public int hashCode() {
//       return filter.hashCode() ^ 0x1117BF25;
//     }
};

CL_NS_END
#endif // _lucene_search_CachingSpanFilter_
