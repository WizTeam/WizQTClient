/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TestSpans
#define _lucene_search_spans_TestSpans

#include "test.h"

#include "CLucene/search/spans/SpanTermQuery.h"
CL_NS_USE2(search,spans)

class TestSpans 
{
private:
    CL_NS(search)::IndexSearcher *  searcher;
    CL_NS(store)::RAMDirectory *    directory;

    static const TCHAR *            docFields[];

public:
    static const TCHAR *            field;
    CuTest *                        tc;


public:
    TestSpans( CuTest* tc );
    virtual ~TestSpans();

    void setUp();
    SpanTermQuery * makeSpanTermQuery( const TCHAR* text );

    void orderedSlopTest3( int32_t slop, int32_t * expectedDocs, size_t expectedDocsCount );
    void orderedSlopTest3Equal( int32_t slop, int32_t * expectedDocs, size_t expectedDocsCount );
    void orderedSlopTest1Equal( int32_t slop, int32_t * expectedDocs, size_t expectedDocsCount );

    void testSpanNearOrdered();
    void testSpanNearOrderedEqual();
    void testSpanNearOrderedEqual1();

    void testSpanNearOrderedOverlap();

    void testSpanOrEmpty();
    void testSpanOrSingle();
    void testSpanOrDouble();
    void testSpanOrDoubleSkip();
    void testSpanOrUnused();
    void testSpanOrTripleSameDoc();

private:
    void checkHits( Query * query, int32_t * results, size_t resultsCount );
    void orderedSlopTest3SQ( SpanQuery * q1, SpanQuery * q2, SpanQuery * q3, int32_t slop, int32_t * expectedDocs, size_t expectedDocCount );

    void orSpans( const TCHAR ** terms, size_t termsCount, Spans *& spans, Query *& query );
    void tstNextSpans( Spans * spans, int32_t doc, int32_t start, int32_t end );
};
#endif

