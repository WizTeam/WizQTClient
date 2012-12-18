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

#include "SpanNotQuery.h"
#include "Spans.h"

CL_NS_DEF2( search, spans )


/////////////////////////////////////////////////////////////////////////////
class SpanNotQuery::SpanNotQuerySpans : public Spans
{
private:
    SpanNotQuery *  parentQuery;
    Spans *         includeSpans;
    bool            moreInclude;

    Spans *         excludeSpans;
    bool            moreExclude;

public:
    SpanNotQuerySpans( SpanNotQuery * parentQuery, CL_NS(index)::IndexReader * reader );
    virtual ~SpanNotQuerySpans();

    bool next();
    bool skipTo( int32_t target );

    int32_t doc() const     { return includeSpans->doc(); }
    int32_t start() const   { return includeSpans->start(); }
    int32_t end() const     { return includeSpans->end(); }

    TCHAR* toString() const;
};

SpanNotQuery::SpanNotQuerySpans::SpanNotQuerySpans( SpanNotQuery * parentQuery, CL_NS(index)::IndexReader * reader )
{
    this->parentQuery = parentQuery;

    includeSpans = parentQuery->include->getSpans( reader );
    moreInclude = true;

    excludeSpans = parentQuery->exclude->getSpans( reader );
    moreExclude = excludeSpans->next();
}

SpanNotQuery::SpanNotQuerySpans::~SpanNotQuerySpans()
{
    _CLLDELETE( includeSpans );
    _CLLDELETE( excludeSpans );
}

bool SpanNotQuery::SpanNotQuerySpans::next()
{
    if( moreInclude )                           // move to next include
        moreInclude = includeSpans->next();

    while( moreInclude && moreExclude )
    {
        if( includeSpans->doc() > excludeSpans->doc() ) // skip exclude
            moreExclude = excludeSpans->skipTo( includeSpans->doc() );

        while( moreExclude                      // while exclude is before
               && includeSpans->doc() == excludeSpans->doc()
               && excludeSpans->end() <= includeSpans->start())
        {
            moreExclude = excludeSpans->next();  // increment exclude
        }

        if( ! moreExclude                       // if no intersection
            || includeSpans->doc() != excludeSpans->doc()
            || includeSpans->end() <= excludeSpans->start())
        break;                                  // we found a match

        moreInclude = includeSpans->next();      // intersected: keep scanning
    }

    return moreInclude;
}


bool SpanNotQuery::SpanNotQuerySpans::skipTo( int32_t target )
{
    if( moreInclude )                           // skip include
        moreInclude = includeSpans->skipTo( target );

    if( ! moreInclude )
        return false;

    if( moreExclude                             // skip exclude
        && includeSpans->doc() > excludeSpans->doc())
    {
        moreExclude = excludeSpans->skipTo( includeSpans->doc() );
    }

    while( moreExclude                          // while exclude is before
           && includeSpans->doc() == excludeSpans->doc()
           && excludeSpans->end() <= includeSpans->start())
    {
        moreExclude = excludeSpans->next();     // increment exclude
    }

    if( ! moreExclude                           // if no intersection
        || includeSpans->doc() != excludeSpans->doc()
        || includeSpans->end() <= excludeSpans->start())
    {
        return true;                            // we found a match
    }

    return next();                              // scan to next match
}

TCHAR* SpanNotQuery::SpanNotQuerySpans::toString() const
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
SpanNotQuery::SpanNotQuery( SpanQuery * include, SpanQuery * exclude, bool bDeleteQueries )
{
    this->include = include;
    this->exclude = exclude;
    this->bDeleteQueries = bDeleteQueries;

    if( 0 != _tcscmp( include->getField(), exclude->getField()))
        _CLTHROWA( CL_ERR_IllegalArgument, "Clauses must have same field." );
}

SpanNotQuery::SpanNotQuery( const SpanNotQuery& clone ) :
    SpanQuery( clone )
{
    include = (SpanQuery *) clone.include->clone();
    exclude = (SpanQuery *) clone.exclude->clone();
    bDeleteQueries = true;
}

SpanNotQuery::~SpanNotQuery()
{
    if( bDeleteQueries )
    {
        _CLLDELETE( include );
        _CLLDELETE( exclude );
    }
}

CL_NS(search)::Query * SpanNotQuery::clone() const
{
    return _CLNEW SpanNotQuery( *this );
}

const char * SpanNotQuery::getClassName()
{
	return "SpanNotQuery";
}

const char * SpanNotQuery::getObjectName() const
{
	return getClassName();
}

SpanQuery * SpanNotQuery::getInclude() const
{
    return include;
}

SpanQuery * SpanNotQuery::getExclude() const
{
    return exclude;
}

const TCHAR * SpanNotQuery::getField() const
{
    return include->getField();
}

void SpanNotQuery::extractTerms( CL_NS(search)::TermSet * terms ) const
{
    include->extractTerms( terms );
}

TCHAR* SpanNotQuery::toString( const TCHAR* field ) const
{
    CL_NS(util)::StringBuffer buffer;

    TCHAR * tmp;

    buffer.append( _T( "spanNot(" ));
    tmp = include->toString( field );
    buffer.append( tmp );
    _CLDELETE_ARRAY( tmp );

    buffer.append( _T( ", " ));
    tmp = exclude->toString( field );
    buffer.append( tmp );
    _CLDELETE_ARRAY( tmp );

    buffer.append( _T( ")" ));
    buffer.appendFloat(  getBoost(), 1 );

    return buffer.toString();
}

CL_NS(search)::Query * SpanNotQuery::rewrite( CL_NS(index)::IndexReader * reader )
{
    SpanNotQuery * clone = NULL;

    SpanQuery * rewrittenInclude = (SpanQuery *) include->rewrite( reader );
    if( rewrittenInclude != include )
    {
        clone = (SpanNotQuery *) this->clone();
        _CLLDELETE( clone->include );
        clone->include = rewrittenInclude;
    }

    SpanQuery * rewrittenExclude = (SpanQuery *) exclude->rewrite( reader );
    if( rewrittenExclude != exclude )
    {
        if( ! clone )
            clone = (SpanNotQuery *) this->clone();
        _CLLDELETE( clone->exclude );
        clone->exclude = rewrittenExclude;
    }

    if( clone )
        return clone;                        // some clauses rewrote
    else
        return this;                         // no clauses rewrote
}

bool SpanNotQuery::equals( Query* other ) const
{
    if( this == other ) return true;
    if( other == NULL || !( other->instanceOf( SpanNotQuery::getClassName() )))
	    return false;

	SpanNotQuery * that = (SpanNotQuery *) other;
    return include->equals( that->include )
        && exclude->equals( that->exclude )
        && getBoost() == that->getBoost();
}

size_t SpanNotQuery::hashCode() const
{
    size_t h = include->hashCode();
    h = (h << 1) | (h >> 31);  // rotate left
    h ^= exclude->hashCode();
    h = (h << 1) | (h >> 31);  // rotate left
    h ^= Similarity::floatToByte( getBoost() );

    return h;
}

Spans * SpanNotQuery::getSpans( CL_NS(index)::IndexReader * reader )
{
   return _CLNEW SpanNotQuerySpans( this, reader );
}

CL_NS_END2
