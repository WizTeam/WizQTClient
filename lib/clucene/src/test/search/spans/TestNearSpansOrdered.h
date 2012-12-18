/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TestNearSpansOrdered
#define _lucene_search_spans_TestNearSpansOrdered

#include "test.h"

CL_CLASS_DEF2(search,spans,SpanNearQuery);
CL_CLASS_DEF2(search,spans,Spans);

class TestNearSpansOrdered 
{
private:
    CL_NS(search)::IndexSearcher *  searcher;
    CL_NS(store)::RAMDirectory *    directory;
    static const TCHAR *            docFields[];
    static const TCHAR *            field;

public:
    CuTest *                        tc;

public:
    TestNearSpansOrdered( CuTest* tc );
    virtual ~TestNearSpansOrdered();

    void setUp();

    void testSpanNearQuery();
    void testNearSpansNext();
    void testNearSpansSkipToLikeNext();
    void testNearSpansNextThenSkipTo();
    void testNearSpansNextThenSkipPast();
    void testNearSpansSkipPast();
    void testNearSpansSkipTo0();
    void testNearSpansSkipTo1();
    void testSpanNearScorerSkipTo1();
    void testSpanNearScorerExplain();

private:
    CL_NS2(search,spans)::SpanNearQuery * makeQuery( const TCHAR * s1, const TCHAR * s2, const TCHAR * s3, int32_t slop, bool inOrder );
    CL_NS2(search,spans)::SpanNearQuery * makeQuery();
  
    bool checkSpans( CL_NS2(search,spans)::Spans * spans, int32_t doc, int32_t start, int32_t end );
};
#endif

