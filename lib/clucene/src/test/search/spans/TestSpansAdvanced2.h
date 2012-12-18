/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TestSpansAdvanced2
#define _lucene_search_spans_TestSpansAdvanced2

#include "TestSpansAdvanced.h"

/*******************************************************************************
 * Some expanded tests to make sure my patch doesn't break other SpanTermQuery
 * functionality.
 *
 * @author Reece Wilton
 */
class TestSpansAdvanced2 : public TestSpansAdvanced 
{
public:
    TestSpansAdvanced2( CuTest* tc );
    virtual ~TestSpansAdvanced2();

    void testVerifyIndex();
    void testSingleSpanQuery();
    void testMultipleDifferentSpanQueries();
    void testBooleanQueryWithSpanQueries();

protected:
    virtual void addDocuments( IndexWriter * writer );
};
#endif

