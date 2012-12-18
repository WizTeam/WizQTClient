/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_FirstSpanQuery_
#define _lucene_search_spans_FirstSpanQuery_

CL_CLASS_DEF(index, IndexReader);
#include "SpanQuery.h"

CL_NS_DEF2( search, spans )

/** Matches spans near the beginning of a field. */
class CLUCENE_EXPORT SpanFirstQuery : public SpanQuery
{
private:
    class SpanFirstQuerySpans;

private:
    SpanQuery *     match;
    bool            bDeleteQuery;
    int32_t         end;

protected:
    SpanFirstQuery( const SpanFirstQuery& clone );

public:
    /** Construct a SpanFirstQuery matching spans in <code>match</code> whose end
     * position is less than or equal to <code>end</code>. */
    SpanFirstQuery( SpanQuery * match, int32_t end, bool bDeleteQuery );
    virtual ~SpanFirstQuery();

    CL_NS(search)::Query * clone() const;

    static const char * getClassName();
	const char * getObjectName() const;

    /** Return the SpanQuery whose matches are filtered. */
    SpanQuery * getMatch() const;

    /** Return the maximum end position permitted in a match. */
    int32_t getEnd() const;

    const TCHAR * getField() const;

    /** Returns a collection of all terms matched by this query.
     * @deprecated use extractTerms instead
     * @see #extractTerms(Set)
     */
//   public Collection getTerms() { return match.getTerms(); }
    void extractTerms( CL_NS(search)::TermSet * terms ) const;

    CL_NS(search)::Query * rewrite( CL_NS(index)::IndexReader * reader );

    using Query::toString;
    TCHAR* toString( const TCHAR* field ) const;
    bool equals( Query* other ) const;
    size_t hashCode() const;

    Spans * getSpans( CL_NS(index)::IndexReader * reader );
};

CL_NS_END2
#endif // _lucene_search_spans_FirstSpanQuery_
