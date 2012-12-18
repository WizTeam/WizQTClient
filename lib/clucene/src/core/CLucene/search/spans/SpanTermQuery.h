/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_SpanTermQuery_
#define _lucene_search_spans_SpanTermQuery_

CL_CLASS_DEF(index, Term);
CL_CLASS_DEF(index, IndexReader);
#include "SpanQuery.h"

CL_NS_DEF2( search, spans )

/** Matches spans containing a term. */
class CLUCENE_EXPORT SpanTermQuery : public SpanQuery
{
protected:
    CL_NS(index)::Term * term;

protected:
    SpanTermQuery( const SpanTermQuery& clone );

public:
    /** Construct a SpanTermQuery matching the named term's spans. */
    SpanTermQuery( CL_NS(index)::Term * term );
    virtual ~SpanTermQuery();

    static const char * getClassName();
	const char * getObjectName() const;

    /** Return the term whose spans are matched. */
	CL_NS(index)::Term * getTerm( bool pointer=true ) const;

    const TCHAR * getField() const;

    /** Returns a collection of all terms matched by this query.
     * @deprecated use extractTerms instead
     * @see #extractTerms(Set)
     */
//    public Collection getTerms()

    void extractTerms( CL_NS(search)::TermSet * terms ) const;
    Spans * getSpans( CL_NS(index)::IndexReader * reader );

    CL_NS(search)::Query * clone() const;

    /** Returns true iff <code>o</code> is equal to this. */
    bool equals( Query* other ) const;

    /** Returns a hash code value for this object.*/
    size_t hashCode() const;

    TCHAR* toString( const TCHAR* field ) const;
};

CL_NS_END2
#endif //_lucene_search_spans_SpanTermQuery_
