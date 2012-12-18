/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestSpansAdvanced2.h"
#include "CLucene/search/spans/SpanTermQuery.h"

CL_NS_USE2(search, spans);
CL_NS_USE(search);

TestSpansAdvanced2::TestSpansAdvanced2( CuTest* tc ) : TestSpansAdvanced( tc )
{
}

TestSpansAdvanced2::~TestSpansAdvanced2()
{
}

void TestSpansAdvanced2::addDocuments( IndexWriter * writer )
{
    TestSpansAdvanced::addDocuments( writer );
    addDocument( writer, _T( "A" ), _T( "Should we, could we, would we?" ));
    addDocument( writer, _T( "B" ), _T( "It should.  Should it?" ));
    addDocument( writer, _T( "C" ), _T( "It shouldn't." ));
    addDocument( writer, _T( "D" ), _T( "Should we, should we, should we." ));
}

/**
 * Verifies that the index has the correct number of documents.
 */
void TestSpansAdvanced2::testVerifyIndex()
{
    IndexReader * reader = IndexReader::open( directory );
    assertEquals( 8, reader->numDocs() );
    reader->close();
    _CLDELETE( reader );
}

/**
 * Tests a single span query that matches multiple documents.
 */
void TestSpansAdvanced2::testSingleSpanQuery()
{
    Term * t1 = _CLNEW Term( field_text, _T( "should" ));
    Query * spanQuery = _CLNEW SpanTermQuery( t1 );
    const TCHAR * expectedIds[] = { _T( "B" ), _T( "D" ), _T( "1" ), _T( "2" ), _T( "3" ), _T( "4" ), _T( "A" ) };
    float_t expectedScores[] = { 0.625f, 0.45927936f, 0.35355338f, 0.35355338f, 0.35355338f, 0.35355338f, 0.26516503f, };
    assertHits( spanQuery, _T( "single span query" ), expectedIds, expectedScores, 7 );
    _CLLDELETE( spanQuery );
    _CLLDECDELETE( t1 );
}

/**
 * Tests a single span query that matches multiple documents.
 */
void TestSpansAdvanced2::testMultipleDifferentSpanQueries()
{
    Term * t1 = _CLNEW Term( field_text, _T( "should" ));
    Term * t2 = _CLNEW Term( field_text, _T( "we" ));
    Query * spanQuery1 = _CLNEW SpanTermQuery( t1 );
    Query * spanQuery2 = _CLNEW SpanTermQuery( t2 );
    BooleanQuery * query = _CLNEW BooleanQuery();
    query->add( spanQuery1, true, BooleanClause::MUST );
    query->add( spanQuery2, true, BooleanClause::MUST );
    
    const TCHAR * expectedIds[] = { _T( "D" ), _T( "A" ) };
    // these values were pre LUCENE-413
    // final float[] expectedScores = new float[] { 0.93163157f, 0.20698164f };
    float_t expectedScores[] = { 1.0191123f, 0.93163157f };
    assertHits( query, _T( "multiple different span queries" ), expectedIds, expectedScores, 2 );

    _CLDELETE( query );
    _CLLDECDELETE( t1 );
    _CLLDECDELETE( t2 );
}

/**
 * Tests two span queries.
 */
void TestSpansAdvanced2::testBooleanQueryWithSpanQueries()
{
    doTestBooleanQueryWithSpanQueries( 0.73500174f );
}
