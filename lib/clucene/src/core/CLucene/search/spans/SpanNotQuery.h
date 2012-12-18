/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_SpanNotQuery_
#define _lucene_search_spans_SpanNotQuery_

CL_CLASS_DEF(index, IndexReader);
#include "SpanQuery.h"

CL_NS_DEF2( search, spans )

/** Removes matches which overlap with another SpanQuery. */
class CLUCENE_EXPORT SpanNotQuery : public SpanQuery
{
private:
    class SpanNotQuerySpans;

private:
    SpanQuery *     include;
    SpanQuery *     exclude;
    bool            bDeleteQueries;

protected:
    SpanNotQuery( const SpanNotQuery& clone );

public:
    /** Construct a SpanNotQuery matching spans from <code>include</code> which
     * have no overlap with spans from <code>exclude</code>.*/
    SpanNotQuery( SpanQuery * include, SpanQuery * exclude, bool bDeleteQueries );
    virtual ~SpanNotQuery();

    CL_NS(search)::Query * clone() const;

    static const char * getClassName();
	const char * getObjectName() const;

    /** Return the SpanQuery whose matches are filtered. */
    SpanQuery * getInclude() const;

    /** Return the SpanQuery whose matches must not overlap those returned. */
    SpanQuery * getExclude() const;

    const TCHAR * getField() const;

    /** Returns a collection of all terms matched by this query.
     * @deprecated use extractTerms instead
     * @see #extractTerms(Set)
     */
//   public Collection getTerms() { return include.getTerms(); }

    void extractTerms( CL_NS(search)::TermSet * terms ) const;

    CL_NS(search)::Query * rewrite( CL_NS(index)::IndexReader * reader );

    using Query::toString;
    TCHAR* toString( const TCHAR* field ) const;
    bool equals( Query* other ) const;
    size_t hashCode() const;

    Spans * getSpans( CL_NS(index)::IndexReader * reader );
};

CL_NS_END2
#endif // _lucene_search_spans_SpanNotQuery_
