/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestExplanations.h"
#include "CheckHits.h"
#include "CLucene/search/spans/SpanQuery.h"
#include "CLucene/search/spans/SpanOrQuery.h"
#include "CLucene/search/spans/SpanTermQuery.h"
#include "CLucene/search/spans/SpanNotQuery.h"
#include "CLucene/search/spans/SpanNearQuery.h"
#include "CLucene/search/spans/SpanFirstQuery.h"


/////////////////////////////////////////////////////////////////////////////
const TCHAR * TestExplanations::field = _T( "field" );
const TCHAR * TestExplanations::docFields[] =
{
    _T( "w1 w2 w3 w4 w5" ),
    _T( "w1 w3 w2 w3 zz" ),
    _T( "w1 xx w2 yy w3" ),
    _T( "w1 w3 xx w2 yy w3 zz" )
};


/////////////////////////////////////////////////////////////////////////////
TestExplanations::ItemizedFilter::ItemizedFilter( int32_t * docs, size_t docsCount )
{
    this->docs = _CL_NEWARRAY( int32_t, docsCount );
    memcpy( this->docs, docs, docsCount * sizeof( int32_t ));
    this->docsCount = docsCount;
}
TestExplanations::ItemizedFilter::~ItemizedFilter()
{
    _CLDELETE_LARRAY( docs );
    docsCount = 0;
}

CL_NS(util)::BitSet * TestExplanations::ItemizedFilter::bits( CL_NS(index)::IndexReader * r )
{
    CL_NS(util)::BitSet * b = _CLNEW CL_NS(util)::BitSet( r->maxDoc() );
    for( size_t i = 0; i < docsCount; i++ )
        b->set( docs[ i ] );
    return b;
}

/////////////////////////////////////////////////////////////////////////////
TestExplanations::TestExplanations( CuTest* tc )
{
    this->searcher = NULL;
    this->directory = NULL;
    this->qp = NULL;
    this->qpAnalyzer = NULL;
    this->tc = tc;
}

TestExplanations::~TestExplanations()
{
    if( searcher )
    {
        searcher->close();
        _CLDELETE( searcher );
    }

    if( directory )
    {
        directory->close();
        _CLDELETE( directory );
    }

    if( qp )
    {
        _CLDELETE( qp );
    }

    if( qpAnalyzer )
    {
        _CLDELETE( qpAnalyzer );
    }
}

void TestExplanations::setUp()
{
    directory = _CLNEW RAMDirectory();
    qpAnalyzer = _CLNEW WhitespaceAnalyzer();
    qp = _CLNEW QueryParser( field, qpAnalyzer );

    IndexWriter * writer= _CLNEW IndexWriter( directory, qpAnalyzer, true );
    for( size_t i = 0; i < sizeof( docFields ) / sizeof( docFields[0] ); i++ )
    {
        Document doc;
        doc.add( * _CLNEW Field( field, docFields[ i ], Field::STORE_NO | Field::INDEX_TOKENIZED ));
        writer->addDocument( &doc );
    }
    writer->close();
    _CLDELETE( writer );

    searcher = _CLNEW IndexSearcher( directory );
}

Query * TestExplanations::makeQuery( const TCHAR * queryText )
{
    return qp->parse( queryText );
}

void TestExplanations::qtest( const TCHAR * queryText, int32_t * expDocNrs, size_t expDocNrsCount )
{
    Query * q = makeQuery( queryText );
    qtest( q , expDocNrs, expDocNrsCount );
    _CLLDELETE( q );
}
  
void TestExplanations::qtest( Query * q, int32_t * expDocNrs, size_t expDocNrsCount )
{
    CheckHits::checkHitCollector( tc, q, field, searcher, expDocNrs, expDocNrsCount );
}

void TestExplanations::bqtest( Query * q, int32_t * expDocNrs, size_t expDocNrsCount )
{
    qtest( reqB( q ), expDocNrs, expDocNrsCount );
    qtest( optB( q ), expDocNrs, expDocNrsCount );
}

void TestExplanations::bqtest( const TCHAR * queryText, int32_t * expDocNrs, size_t expDocNrsCount )
{
    Query * q = makeQuery( queryText );
    bqtest( q, expDocNrs, expDocNrsCount );
    _CLLDELETE( q );
}
  
Term ** TestExplanations::ta( const TCHAR ** s, size_t count )
{
    Term ** t = _CL_NEWARRAY( Term *, count );
    for( size_t i = 0; i < count; i++ )
        t[ i ] = _CLNEW Term( field, s[ i ] );

    return t;
}

SpanTermQuery * TestExplanations::st( const TCHAR * s )
{
    Term * t = _CLNEW Term( field, s );
    SpanTermQuery * q = _CLNEW SpanTermQuery( t );
    _CLLDECDELETE( t );
    return q;
}
  
SpanNotQuery * TestExplanations::snot( SpanQuery * i, SpanQuery * e )
{
    return _CLNEW SpanNotQuery( i, e, true );
}

SpanOrQuery * TestExplanations::sor( const TCHAR * s,  const TCHAR * e ) 
{
    return sor( st( s ), st( e ));
}

SpanOrQuery * TestExplanations::sor( SpanQuery * s, SpanQuery * e )
{
    SpanQuery * clauses[] = { s, e };
    return _CLNEW SpanOrQuery( clauses, clauses+2, true );
}

SpanOrQuery * TestExplanations::sor( const TCHAR * s,  const TCHAR * m,  const TCHAR * e) 
{
    return sor(st(s), st(m), st(e));
}

SpanOrQuery * TestExplanations::sor( SpanQuery * s, SpanQuery * m, SpanQuery * e ) 
{
    SpanQuery * clauses[] = { s, m, e };
    return _CLNEW SpanOrQuery( clauses, clauses+3, true );
}

SpanNearQuery * TestExplanations::snear( const TCHAR * s,  const TCHAR * e, int32_t slop, bool inOrder )
{
    return snear( st(s), st(e), slop, inOrder );
}

SpanNearQuery * TestExplanations::snear( SpanQuery * s, SpanQuery * e, int32_t slop, bool inOrder )
{
    SpanQuery * clauses[] = { s, e };
    return _CLNEW SpanNearQuery( clauses, clauses+2, slop, inOrder, true );
}
  
SpanNearQuery * TestExplanations::snear( const TCHAR * s,  const TCHAR * m,  const TCHAR * e, int32_t slop, bool inOrder ) 
{
    return snear( st( s ), st( m ), st( e ), slop, inOrder );
}

SpanNearQuery * TestExplanations::snear( SpanQuery * s, SpanQuery * m, SpanQuery * e, int32_t slop, bool inOrder )
{
    SpanQuery * clauses[] = { s, m, e };
    return _CLNEW SpanNearQuery( clauses, clauses+3, slop, inOrder, true );
}
  
SpanFirstQuery * TestExplanations::sf( const TCHAR * s, int32_t b )
{
    return _CLNEW SpanFirstQuery( st( s ), b, true );
}

Query * TestExplanations::optB( const TCHAR * q )
{
    return optB( makeQuery( q ));
}

Query * TestExplanations::optB( Query * q ) 
{
    Term * t = _CLNEW Term( _T( "NEVER" ), _T( "MATCH" ));
    TermQuery * tq = _CLNEW TermQuery( t );
    _CLLDECDELETE( t );

    BooleanQuery * bq = _CLNEW BooleanQuery( true );
    bq->add( q, true, BooleanClause::SHOULD );
    bq->add( tq, true, BooleanClause::MUST_NOT );
    return bq;
}
  
Query * TestExplanations::reqB( const TCHAR * q )
{
    return reqB( makeQuery( q ));
}

Query * TestExplanations::reqB( Query * q ) 
{
    Term * t = _CLNEW Term( field, _T( "w1" ));
    TermQuery * tq = _CLNEW TermQuery( t );
    _CLLDECDELETE( t );

    BooleanQuery * bq = _CLNEW BooleanQuery( true );
    bq->add( q, BooleanClause::MUST );
    bq->add( tq, BooleanClause::SHOULD );
    return bq;
}

