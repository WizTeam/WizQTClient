/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/StringBuffer.h"

#include "SpanNearQuery.h"
#include "_EmptySpans.h"
#include "_NearSpansOrdered.h"
#include "_NearSpansUnordered.h"

CL_NS_DEF2( search, spans )

SpanNearQuery::SpanNearQuery( const SpanNearQuery& clone ) :
    SpanQuery( clone )
{
    this->clauses = _CL_NEWARRAY( SpanQuery *, clone.clausesCount );
    for( size_t i = 0; i < clone.clausesCount; i++ )
        this->clauses[ i ] = (SpanQuery *) clone.clauses[ i ]->clone();

    this->clausesCount = clone.clausesCount;
    this->bDeleteClauses = true;

    this->slop = clone.slop;
    this->inOrder = clone.inOrder;

    this->field = NULL;
    setField( clone.field );
}

SpanNearQuery::~SpanNearQuery()
{
    if( bDeleteClauses )
    {
        for( size_t i = 0; i < clausesCount; i++ )
            _CLLDELETE( clauses[ i ] );
    }

    clausesCount = 0;
    _CLDELETE_LARRAY( clauses );
    _CLDELETE_LARRAY( field );
}

CL_NS(search)::Query * SpanNearQuery::clone() const
{
    return _CLNEW SpanNearQuery( *this );
}

const char * SpanNearQuery::getClassName()
{
	return "SpanNearQuery";
}

const char * SpanNearQuery::getObjectName() const
{
	return getClassName();
}

SpanQuery ** SpanNearQuery::getClauses() const
{
    return clauses;
}

size_t SpanNearQuery::getClausesCount() const
{
    return clausesCount;
}

void SpanNearQuery::setField( const TCHAR * field )
{
    _CLDELETE_LARRAY( this->field );
    this->field = STRDUP_TtoT( field );
}

const TCHAR * SpanNearQuery::getField() const
{
    return field;
}

int32_t SpanNearQuery::getSlop() const
{
    return slop;
}

bool SpanNearQuery::isInOrder() const
{
    return inOrder;
}

void SpanNearQuery::extractTerms( CL_NS(search)::TermSet * terms ) const
{
    for( size_t i = 0; i < clausesCount; i++ )
        clauses[ i ]->extractTerms( terms );
}

CL_NS(search)::Query * SpanNearQuery::rewrite( CL_NS(index)::IndexReader * reader )
{
    SpanNearQuery * clone = NULL;

    for( size_t i = 0; i < clausesCount; i++ )
    {
        SpanQuery * c = clauses[ i ];
        SpanQuery * query = (SpanQuery *) c->rewrite( reader );
        if( query != c )
        {                     // clause rewrote: must clone
            if( clone == NULL )
                clone = (SpanNearQuery *) this->clone();

            _CLLDELETE( clone->clauses[ i ] );
            clone->clauses[ i ] = query;
        }
    }
    if( clone )
        return clone;                        // some clauses rewrote
    else
        return this;                         // no clauses rewrote
}

TCHAR* SpanNearQuery::toString( const TCHAR* field ) const
{
    CL_NS(util)::StringBuffer buffer;

    buffer.append( _T( "spanNear([" ));
    for( size_t i = 0; i < clausesCount; i++ )
    {
        if( i != 0 )
            buffer.append( _T( ", " ));

        TCHAR * tszClause = clauses[ i ]->toString( field );
        buffer.append( tszClause );
        _CLDELETE_ARRAY( tszClause );
    }

    buffer.append( _T( "], " ));
    buffer.appendInt( slop );
    buffer.append( _T( ", " ));
    buffer.appendBool( inOrder );
    buffer.append( _T( ")" ));
    buffer.appendBoost( getBoost() );

    return buffer.toString();
}

bool SpanNearQuery::equals( Query* other ) const
{
    if( this == other ) return true;
    if( other == NULL || !( other->instanceOf( SpanNearQuery::getClassName() )))
	    return false;

	SpanNearQuery * that = (SpanNearQuery *) other;
    if( inOrder != that->inOrder
        || slop != that->slop
        || getBoost() != that->getBoost()
        || 0 != _tcscmp( field, that->field ) )     // CLucene: java version does not compare field names
    {
        return false;
    }

    if( clausesCount != that->clausesCount )
        return false;
    for( size_t i = 0; i < clausesCount; i++ )
    {
        if( ! clauses[ i ]->equals( that->clauses[ i ] ))
            return false;
    }

    return true;
}

size_t SpanNearQuery::hashCode() const
{
    size_t result = 1;
    for( size_t i = 0; i < clausesCount; i++ )
        result = 31*result + clauses[ i ]->hashCode();

    // Mix bits before folding in things like boost, since it could cancel the
    // last element of clauses.  This particular mix also serves to
    // differentiate SpanNearQuery hash codes from others.
    result ^= (result << 14) | (result >> 19);  // reversible
    result += Similarity::floatToByte( getBoost() );
    result += slop;
    result ^= ( inOrder ? 0x99AFD3BD : 0 );

    return result;
}

Spans * SpanNearQuery::getSpans( CL_NS(index)::IndexReader * reader )
{
    if( clausesCount == 0 )
        return _CLNEW EmptySpans();               // CLucene: 0-clause case - different to java version, because java creates SpanOrQuery to call its function to create empty spans

    if( clausesCount == 1 )                       // optimize 1-clause case
      return clauses[ 0 ]->getSpans( reader );

    return inOrder
            ? (Spans *) _CLNEW NearSpansOrdered( this, reader )
            : (Spans *) _CLNEW NearSpansUnordered( this, reader );
}

CL_NS_END2
