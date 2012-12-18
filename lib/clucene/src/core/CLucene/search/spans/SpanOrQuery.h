/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_SpanOrQuery_
#define _lucene_search_spans_SpanOrQuery_

CL_CLASS_DEF(index, IndexReader);
#include "SpanQuery.h"

CL_NS_DEF2( search, spans )

/**
 * Matches the union of its clauses.
 */
class CLUCENE_EXPORT SpanOrQuery : public SpanQuery
{
private:
    class SpanQueue;
    class SpanOrQuerySpans;

private:
    SpanQuery **    clauses;
    size_t          clausesCount;
    bool            bDeleteClauses;

    TCHAR *         field;

protected:
    SpanOrQuery( const SpanOrQuery& clone );

public:
    /** Construct a SpanOrQuery merging the provided clauses. */
    template<class ClauseIterator>
    SpanOrQuery( ClauseIterator first, ClauseIterator last, bool bDeleteClauses )
    {
        // CLucene specific: at least one clause must be here
        if( first ==  last )
            _CLTHROWA( CL_ERR_IllegalArgument, "Missing query clauses." );

        this->bDeleteClauses = bDeleteClauses;
        this->clausesCount = last - first;
        this->clauses = _CL_NEWARRAY( SpanQuery *, clausesCount );
        this->field = NULL;

        // copy clauses array into an array and check fields
        for( size_t i = 0; first != last; first++, i++ )
        {
            SpanQuery * clause = *first;
            if( i == 0 )
            {
                setField( clause->getField() );
            }
            else if( 0 != _tcscmp( clause->getField(), field ))
            {
                _CLTHROWA( CL_ERR_IllegalArgument, "Clauses must have same field." );
            }
            this->clauses[ i ] = clause;
        }
    }

    virtual ~SpanOrQuery();

    CL_NS(search)::Query * clone() const;

    static const char * getClassName();
	const char * getObjectName() const;

    /** Return the clauses whose spans are matched.
     * CLucene: pointer to the internal array
     */
    SpanQuery ** getClauses() const;
    size_t getClausesCount() const;

    const TCHAR * getField() const;

    /** Returns a collection of all terms matched by this query.
     * @deprecated use extractTerms instead
     * @see #extractTerms(Set)
     */
//    public Collection getTerms()

    void extractTerms( CL_NS(search)::TermSet * terms ) const;

    CL_NS(search)::Query * rewrite( CL_NS(index)::IndexReader * reader );

    using Query::toString;
    TCHAR* toString( const TCHAR* field ) const;
    bool equals( Query* other ) const;
    size_t hashCode() const;

    /** This returns some kind of lazy spans. The set will be evaluated with the first call
     *  and this query and the given reader must exists at this time
     */
    Spans * getSpans( CL_NS(index)::IndexReader * reader );

protected:
    void setField( const TCHAR * field );
};

CL_NS_END2
#endif // _lucene_search_spans_SpanOrQuery_
