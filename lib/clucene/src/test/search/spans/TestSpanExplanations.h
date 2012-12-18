/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TestSpanExplanations
#define _lucene_search_spans_TestSpanExplanations

#include "../TestExplanations.h"

/**
 * TestExplanations subclass focusing on span queries
 */
class TestSpanExplanations : public TestExplanations 
{
public:
    TestSpanExplanations( CuTest * tc );
    virtual ~TestSpanExplanations();

    void testST1();
    void testST2();
    void testST4();
    void testST5();

    void testSF1();
    void testSF2();
    void testSF4();
    void testSF5();
    void testSF6();
  
    void testSO1();
    void testSO2();
    void testSO3();
    void testSO4();

    void testSNear1();
    void testSNear2();
    void testSNear3();
    void testSNear4();
    void testSNear5();
    void testSNear6();
    void testSNear7();
    void testSNear8();
    void testSNear9();
    void testSNear10();
    void testSNear11();

    void testSNot1();
    void testSNot2();
    void testSNot4();
    void testSNot5();
    void testSNot7();
    void testSNot10();
};
#endif

