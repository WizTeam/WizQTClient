/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/StringBuffer.h"

#include "SpanTermQuery.h"
#include "_TermSpans.h"

CL_NS_DEF2( search, spans )

SpanTermQuery::SpanTermQuery( CL_NS(index)::Term * term )
{
    this->term = _CL_POINTER( term );
}

SpanTermQuery::SpanTermQuery( const SpanTermQuery& clone ) :
    SpanQuery( clone )
{
    this->term = _CL_POINTER( clone.term );
}

SpanTermQuery::~SpanTermQuery()
{
    _CLLDECDELETE( term );
}

CL_NS(search)::Query * SpanTermQuery::clone() const
{
    return _CLNEW SpanTermQuery( *this );
}

const char* SpanTermQuery::getClassName()
{
	return "SpanTermQuery";
}

const char* SpanTermQuery::getObjectName() const
{
	return getClassName();
}

size_t SpanTermQuery::hashCode() const
{
	return Similarity::floatToByte(getBoost()) ^ term->hashCode() ^ 0xD23FE494;
}

CL_NS(index)::Term * SpanTermQuery::getTerm( bool pointer ) const
{
	if ( pointer )
		return _CL_POINTER( term );
	else
		return term;
}

const TCHAR * SpanTermQuery::getField() const
{
    return term->field();
}

void SpanTermQuery::extractTerms( CL_NS(search)::TermSet * terms ) const
{
    if( term && terms->end() == terms->find( term ))
        terms->insert( _CL_POINTER( term ));
}

Spans * SpanTermQuery::getSpans( CL_NS(index)::IndexReader * reader )
{
    return _CLNEW TermSpans( reader->termPositions( term ), term );
}

TCHAR* SpanTermQuery::toString( const TCHAR* field ) const
{
    CL_NS(util)::StringBuffer buffer;

    if( field && 0 == _tcscmp( term->field(), field ))
        buffer.append( term->text() );
    else
    {
        TCHAR * tszTerm = term->toString();
        buffer.append( tszTerm );
        buffer.appendBoost( getBoost() );
        _CLDELETE_CARRAY( tszTerm );
    }
    return buffer.toString();
}

bool SpanTermQuery::equals( Query* other ) const
{
    if( !( other->instanceOf( SpanTermQuery::getClassName() )))
	    return false;

	SpanTermQuery * that = (SpanTermQuery *) other;
	return ( this->getBoost() == that->getBoost() )
		  && this->term->equals( that->term );
}

CL_NS_END2
