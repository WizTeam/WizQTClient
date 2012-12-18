/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/StringBuffer.h"

#include "SpanFirstQuery.h"
#include "Spans.h"

CL_NS_DEF2( search, spans )


/////////////////////////////////////////////////////////////////////////////
class SpanFirstQuery::SpanFirstQuerySpans : public Spans
{
private:
    Spans *          spans;
    int32_t          end_;
    SpanFirstQuery * parentQuery;

public:
    SpanFirstQuerySpans( SpanFirstQuery * parentQuery, CL_NS(index)::IndexReader * reader );
    virtual ~SpanFirstQuerySpans();

    bool next();
    bool skipTo( int32_t target );

    int32_t doc() const   { return spans->doc(); }
    int32_t start() const { return spans->start(); }
    int32_t end() const   { return spans->end(); }

    TCHAR* toString() const;
};

SpanFirstQuery::SpanFirstQuerySpans::SpanFirstQuerySpans( SpanFirstQuery * parentQuery, CL_NS(index)::IndexReader * reader )
{
    this->parentQuery = parentQuery;
    this->end_ = parentQuery->end;
    this->spans = parentQuery->match->getSpans( reader );
}

SpanFirstQuery::SpanFirstQuerySpans::~SpanFirstQuerySpans()
{
    _CLLDELETE( spans );
}

bool SpanFirstQuery::SpanFirstQuerySpans::next()
{
    while( spans->next() )          // scan to next match
    {
        if( spans->end() <= end_ )
            return true;
    }
    return false;
}

bool SpanFirstQuery::SpanFirstQuerySpans::skipTo( int32_t target )
{
    if( ! spans->skipTo( target ))
        return false;

    if( spans->end() <= end_ )      // there is a match
        return true;

    return next();                  // scan to next match
}

TCHAR* SpanFirstQuery::SpanFirstQuerySpans::toString() const
{
    CL_NS(util)::StringBuffer buffer;
    TCHAR *      tszQry = parentQuery->toString();

    buffer.append( _T( "spans(" ));
    buffer.append( tszQry );
    buffer.append( _T( ")" ));

    _CLDELETE_LARRAY( tszQry );
    return buffer.toString();
}


/////////////////////////////////////////////////////////////////////////////
SpanFirstQuery::SpanFirstQuery( SpanQuery * match, int32_t end, bool bDeleteQuery )
{
    this->match = match;
    this->end = end;
    this->bDeleteQuery = bDeleteQuery;
}

SpanFirstQuery::SpanFirstQuery( const SpanFirstQuery& clone ) :
    SpanQuery( clone )
{
    this->match = (SpanQuery *) clone.match->clone();
    this->end = clone.end;
    this->bDeleteQuery = true;
}

SpanFirstQuery::~SpanFirstQuery()
{
    if( bDeleteQuery )
    {
        _CLLDELETE( match );
    }
}

CL_NS(search)::Query * SpanFirstQuery::clone() const
{
    return _CLNEW SpanFirstQuery( *this );
}

const char * SpanFirstQuery::getClassName()
{
	return "SpanFirstQuery";
}

const char * SpanFirstQuery::getObjectName() const
{
	return getClassName();
}

SpanQuery * SpanFirstQuery::getMatch() const
{
    return match;
}

int32_t SpanFirstQuery::getEnd() const
{
    return end;
}

const TCHAR * SpanFirstQuery::getField() const
{
    return match->getField();
}

void SpanFirstQuery::extractTerms( CL_NS(search)::TermSet * terms ) const
{
    match->extractTerms( terms );
}

CL_NS(search)::Query * SpanFirstQuery::rewrite( CL_NS(index)::IndexReader * reader )
{
    SpanFirstQuery * clone = NULL;

    SpanQuery * rewritten = (SpanQuery *) match->rewrite( reader );
    if( rewritten != match )
    {
        clone = (SpanFirstQuery *) this->clone();
        _CLLDELETE( clone->match );
        clone->match = rewritten;
    }

    if( clone )
        return clone;                        // some clauses rewrote
    else
        return this;                         // no clauses rewrote
}

TCHAR* SpanFirstQuery::toString( const TCHAR* field ) const
{
    CL_NS(util)::StringBuffer buffer;
    TCHAR * tszMatch = match->toString( field );

    buffer.append( _T( "spanFirst(" ));
    buffer.append( tszMatch );
    buffer.append( _T( ", " ));
    buffer.appendInt( end );
    buffer.append( _T( ")" ));
    buffer.appendBoost( getBoost() );
    _CLDELETE_LARRAY( tszMatch );

    return buffer.toString();
}

bool SpanFirstQuery::equals( Query * other ) const
{
    if( this == other ) return true;
    if( other == NULL || !( other->instanceOf( SpanFirstQuery::getClassName() )))
	    return false;

	SpanFirstQuery * that = (SpanFirstQuery *) other;
    return end == that->end
        && getBoost() == that->getBoost()
        && match->equals( that->match );
}

size_t SpanFirstQuery::hashCode() const
{
    size_t h = match->hashCode();
    h ^= (h << 8) | (h >> 25);  // reversible
    h ^= Similarity::floatToByte( getBoost() ) ^ end;
    return h;
}

Spans * SpanFirstQuery::getSpans( CL_NS(index)::IndexReader * reader )
{
    return _CLNEW SpanFirstQuerySpans( this, reader );
}

CL_NS_END2
