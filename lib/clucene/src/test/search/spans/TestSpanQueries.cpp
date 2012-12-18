/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestBasics.h"
#include "TestSpans.h"
#include "TestNearSpansOrdered.h"
#include "TestSpansAdvanced2.h"
#include "TestSpanExplanationsOfNonMatches.h"

/////////////////////////////////////////////////////////////////////////////
void testBasics( CuTest * tc )
{
    TestBasics basicsTest( tc );
    basicsTest.setUp();
    basicsTest.testTerm();
    basicsTest.testTerm2();
    basicsTest.testPhrase();
    basicsTest.testPhrase2();
    basicsTest.testBoolean();
    basicsTest.testBoolean2();
    basicsTest.testSpanNearExact();
    basicsTest.testSpanNearUnordered();
    basicsTest.testSpanNearOrdered();
    basicsTest.testSpanNot();
    basicsTest.testSpanWithMultipleNotSingle();
    basicsTest.testSpanWithMultipleNotMany();
    basicsTest.testNpeInSpanNearWithSpanNot();
    basicsTest.testNpeInSpanNearInSpanFirstInSpanNot();
    basicsTest.testSpanFirst();
    basicsTest.testSpanOr();
    basicsTest.testSpanExactNested(); 
    basicsTest.testSpanNearOr();
    basicsTest.testSpanComplex1();
}

/////////////////////////////////////////////////////////////////////////////
void testSpans( CuTest * tc )
{
    TestSpans spansTest( tc );
    spansTest.setUp();
    spansTest.testSpanNearOrdered();
    spansTest.testSpanNearOrderedEqual();
    spansTest.testSpanNearOrderedEqual1();
    spansTest.testSpanNearOrderedOverlap();
    
    // CLucene specific: SpanOr query with no clauses are not allowed
    // spansTest.testSpanOrEmpty();     
    
    spansTest.testSpanOrSingle();
    spansTest.testSpanOrDouble();
    spansTest.testSpanOrDoubleSkip();
    spansTest.testSpanOrUnused();
    spansTest.testSpanOrTripleSameDoc();
}

/////////////////////////////////////////////////////////////////////////////
void testNearSpansOrdered( CuTest * tc )
{
    TestNearSpansOrdered test( tc );
    test.setUp();
    test.testSpanNearQuery();
    test.testNearSpansNext();
    test.testNearSpansSkipToLikeNext();
    test.testNearSpansNextThenSkipTo();
    test.testNearSpansNextThenSkipPast();
    test.testNearSpansSkipPast();
    test.testNearSpansSkipTo0();
    test.testNearSpansSkipTo1();
    test.testSpanNearScorerSkipTo1();
    test.testSpanNearScorerExplain();
}

/////////////////////////////////////////////////////////////////////////////
void testSpansAdvanced( CuTest * tc )
{
    TestSpansAdvanced test( tc );
    test.setUp();
    test.testBooleanQueryWithSpanQueries();
}

/////////////////////////////////////////////////////////////////////////////
void testSpansAdvanced2( CuTest * tc )
{
    TestSpansAdvanced2 test( tc );
    test.setUp();
    test.testVerifyIndex();
    test.testSingleSpanQuery();
    test.testMultipleDifferentSpanQueries();
    test.testBooleanQueryWithSpanQueries();
}

/////////////////////////////////////////////////////////////////////////////
void testSpanExplanations( CuTest * tc )
{
    TestSpanExplanations test( tc );
    test.setUp();
    test.testST1();
    test.testST2();
    test.testST4();
    test.testST5();
    test.testSF1();
    test.testSF2();
    test.testSF4();
    test.testSF5();
    test.testSF6();
    test.testSO1();
    test.testSO2();
    test.testSO3();
    test.testSO4();
    test.testSNear1();
    test.testSNear2();
    test.testSNear3();
    test.testSNear4();
    test.testSNear5();
    test.testSNear6();
    test.testSNear7();
    test.testSNear8();
    test.testSNear9();
    test.testSNear10();
    test.testSNear11();
    test.testSNot1();
    test.testSNot2();
    test.testSNot4();
    test.testSNot5();
    test.testSNot7();
    test.testSNot10();
}

/////////////////////////////////////////////////////////////////////////////
void testSpanExplanationsOfNonMatches( CuTest * tc )
{
    TestSpanExplanationsOfNonMatches test( tc );
    test.setUp();
    test.testST1();
    test.testST2();
    test.testST4();
    test.testST5();
    test.testSF1();
    test.testSF2();
    test.testSF4();
    test.testSF5();
    test.testSF6();
    test.testSO1();
    test.testSO2();
    test.testSO3();
    test.testSO4();
    test.testSNear1();
    test.testSNear2();
    test.testSNear3();
    test.testSNear4();
    test.testSNear5();
    test.testSNear6();
    test.testSNear7();
    test.testSNear8();
    test.testSNear9();
    test.testSNear10();
    test.testSNear11();
    test.testSNot1();
    test.testSNot2();
    test.testSNot4();
    test.testSNot5();
    test.testSNot7();
    test.testSNot10();
}


/////////////////////////////////////////////////////////////////////////////
// Test suite for all tests of span queries
CuSuite *testSpanQueries(void)
{
	CuSuite *suite = CuSuiteNew( _T( "CLucene SpanQuery Tests" ));

    SUITE_ADD_TEST( suite, testBasics );
    SUITE_ADD_TEST( suite, testSpans );
    SUITE_ADD_TEST( suite, testNearSpansOrdered );
    SUITE_ADD_TEST( suite, testSpansAdvanced );
    SUITE_ADD_TEST( suite, testSpansAdvanced2 );
    SUITE_ADD_TEST( suite, testSpanExplanations );
    SUITE_ADD_TEST( suite, testSpanExplanationsOfNonMatches );

	return suite; 
}

