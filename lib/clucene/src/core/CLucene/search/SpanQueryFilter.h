/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_SpanQueryFilter_
#define _lucene_search_SpanQueryFilter_

#include "SpanFilter.h"
#include "spans/SpanQuery.h"

CL_NS_DEF(search)

/**
 * Constrains search results to only match those which also match a provided
 * query. Also provides position information about where each document matches
 * at the cost of extra space compared with the QueryWrapperFilter.
 * There is an added cost to this above what is stored in a {@link QueryWrapperFilter}.  Namely,
 * the position information for each matching document is stored.
 * <p/>
 * This filter does not cache. See the {@link org.apache.lucene.search.CachingSpanFilter} for a wrapper that
 * caches.
 *
 * @version $Id:$
 */
class CLUCENE_EXPORT SpanQueryFilter : public SpanFilter 
{
protected:
    CL_NS2(search,spans)::SpanQuery *   query;
    bool                                bDeleteQuery;

protected:
    SpanQueryFilter();
    SpanQueryFilter( const SpanQueryFilter& copy );

public:
    /** Constructs a filter which only matches documents matching
     * <code>query</code>.
     * @param query The {@link org.apache.lucene.search.spans.SpanQuery} to use as the basis for the Filter.
     */
    SpanQueryFilter( const CL_NS2(search,spans)::SpanQuery * query );

    SpanQueryFilter( CL_NS2(search,spans)::SpanQuery * query, bool bDeleteQuery );

    virtual ~SpanQueryFilter();

    virtual Filter* clone() const;

    virtual CL_NS(util)::BitSet* bits( CL_NS(index)::IndexReader * reader );

    virtual SpanFilterResult * bitSpans( CL_NS(index)::IndexReader * reader );

    CL_NS2(search,spans)::SpanQuery * getQuery();

    virtual TCHAR* toString();


//     public boolean equals( Object o ) {
//         return o instanceof SpanQueryFilter && this.query.equals(((SpanQueryFilter) o).query);
//     }
// 
//     public int hashCode() {
//         return query.hashCode() ^ 0x923F64B9;
//     }
};

inline CL_NS2(search,spans)::SpanQuery * SpanQueryFilter::getQuery()
{
    return query;
}

CL_NS_END
#endif // _lucene_search_SpanQueryFilter_
