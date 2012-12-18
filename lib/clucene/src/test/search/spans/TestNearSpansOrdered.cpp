/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestNearSpansOrdered.h"
#include "../CheckHits.h"
#include "CLucene/search/spans/SpanQuery.h"
#include "CLucene/search/spans/SpanTermQuery.h"
#include "CLucene/search/spans/SpanNearQuery.h"
#include "CLucene/search/spans/Spans.h"
#include "CLucene/search/Scorer.h"

CL_NS_USE2(search,spans);
CL_NS_USE(search);

const TCHAR * TestNearSpansOrdered::field = _T( "field" );
const TCHAR * TestNearSpansOrdered::docFields[] =
{
    _T( "w1 w2 w3 w4 w5" ),
    _T( "w1 w3 w2 w3 zz" ),
    _T( "w1 xx w2 yy w3" ),
    _T( "w1 w3 xx w2 yy w3 zz" )
};


TestNearSpansOrdered::TestNearSpansOrdered( CuTest* tc )
{
    this->tc = tc;
    this->searcher = NULL;
    this->directory = NULL;
}

TestNearSpansOrdered::~TestNearSpansOrdered()
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
}

void TestNearSpansOrdered::setUp()
{
    directory = _CLNEW RAMDirectory();
    Analyzer * analyzer = _CLNEW WhitespaceAnalyzer();
    IndexWriter * writer= _CLNEW IndexWriter( directory, analyzer, true );

    for( size_t i = 0; i < sizeof( docFields ) / sizeof( docFields[0] ); i++ )
    {
        Document doc;
        doc.add( * _CLNEW Field( field, docFields[ i ], Field::STORE_NO | Field::INDEX_TOKENIZED ));
        writer->addDocument( &doc );
    }
    
    writer->close();
    _CLDELETE( writer );
    _CLDELETE( analyzer );

    searcher = _CLNEW IndexSearcher( directory );
}

SpanNearQuery * TestNearSpansOrdered::makeQuery( const TCHAR * s1, const TCHAR * s2, const TCHAR * s3, int32_t slop, bool inOrder )
{
    SpanTermQuery * clauses[ 3 ];
    Term * t1 = _CLNEW Term( field, s1 );
    Term * t2 = _CLNEW Term( field, s2 );
    Term * t3 = _CLNEW Term( field, s3 );
    clauses[ 0 ] = _CLNEW SpanTermQuery( t1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( t2 );
    clauses[ 2 ] = _CLNEW SpanTermQuery( t3 );
    _CLLDECDELETE( t1 );
    _CLLDECDELETE( t2 );
    _CLLDECDELETE( t3 );
    return _CLNEW SpanNearQuery( clauses, clauses+3, slop, inOrder, true );
}

SpanNearQuery * TestNearSpansOrdered::makeQuery() 
{
    return makeQuery( _T( "w1" ), _T( "w2" ), _T( "w3" ), 1, true );
}
  
bool TestNearSpansOrdered::checkSpans( CL_NS2(search,spans)::Spans * spans, int32_t doc, int32_t start, int32_t end )
{
    return spans && spans->doc() == doc && spans->end() == end && spans->start() == start;
}

void TestNearSpansOrdered::testSpanNearQuery()
{
    int32_t expectedDocs[] = {0,1}; 

    SpanNearQuery * q = makeQuery();
    CheckHits::checkHits( tc, q, field, searcher, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
    _CLLDELETE( q );
}

void TestNearSpansOrdered::testNearSpansNext()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( spans->next());
    assertTrue( checkSpans( spans, 0, 0 ,3 ));
    assertTrue( spans->next() );
    assertTrue( checkSpans( spans, 1, 0, 4 ));
    assertTrue( ! spans->next() );
    _CLLDELETE( spans );
    _CLLDELETE( q );
}

/**
 * test does not imply that skipTo(doc+1) should work exactly the
 * same as next -- it's only applicable in this case since we know doc
 * does not contain more than one spans
 */
void TestNearSpansOrdered::testNearSpansSkipToLikeNext()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( spans->skipTo(0) );
    assertTrue( checkSpans( spans, 0, 0, 3 ));
    assertTrue( spans->skipTo(1) );
    assertTrue( checkSpans( spans, 1, 0, 4 ));
    assertTrue( ! spans->skipTo(2) );
    _CLLDELETE( spans );
    _CLLDELETE( q );
}
  
void TestNearSpansOrdered::testNearSpansNextThenSkipTo()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( spans->next() );
    assertTrue( checkSpans( spans, 0, 0, 3 ));
    assertTrue( spans->skipTo( 1 ));
    assertTrue( checkSpans( spans, 1, 0, 4 ));
    assertTrue( ! spans->next() );
    _CLLDELETE( spans );
    _CLLDELETE( q );
}
  
void TestNearSpansOrdered::testNearSpansNextThenSkipPast()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( spans->next() );
    assertTrue( checkSpans( spans, 0, 0, 3 ));
    assertTrue( ! spans->skipTo( 2 ));
    _CLLDELETE( spans );
    _CLLDELETE( q );
}
  
void TestNearSpansOrdered::testNearSpansSkipPast()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( ! spans->skipTo( 2 ));
    _CLLDELETE( spans );
    _CLLDELETE( q );
}

void TestNearSpansOrdered::testNearSpansSkipTo0()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( spans->skipTo( 0 ));
    assertTrue( checkSpans( spans, 0, 0, 3 ));
    _CLLDELETE( spans );
    _CLLDELETE( q );
}

void TestNearSpansOrdered::testNearSpansSkipTo1()
{
    SpanNearQuery * q = makeQuery();
    Spans * spans = q->getSpans( searcher->getReader() );
    assertTrue( spans->skipTo(1) );
    assertTrue( checkSpans( spans, 1, 0, 4 ));
    _CLLDELETE( spans );
    _CLLDELETE( q );
}

/**
 * not a direct test of NearSpans, but a demonstration of how/when
 * this causes problems
 */
void TestNearSpansOrdered::testSpanNearScorerSkipTo1()
{
    SpanNearQuery * q = makeQuery();
    Weight * w = q->_createWeight( searcher );
    Scorer * s = w->scorer( searcher->getReader() );
    assertTrue( s->skipTo( 1 ));
    assertTrue( 1 == s->doc());
    _CLLDELETE( s );
    _CLLDELETE( w );
    _CLLDELETE( q );
}

/**
 * not a direct test of NearSpans, but a demonstration of how/when
 * this causes problems
 */
void TestNearSpansOrdered::testSpanNearScorerExplain()
{
    SpanNearQuery * q = makeQuery();
    Weight * w = q->_createWeight( searcher );
    Scorer * s = w->scorer( searcher->getReader() );
    Explanation * e = s->explain( 1 );
    
    assertTrueMsg( _T( "Scorer explanation value for doc#1 isn't positive." ), 0.0f < e->getValue() );

    _CLLDELETE( e );
    _CLLDELETE( s );
    _CLLDELETE( w );
    _CLLDELETE( q );
}
