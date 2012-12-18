/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TestSpanExplanationsOfNonMatches
#define _lucene_search_spans_TestSpanExplanationsOfNonMatches

#include "TestSpanExplanations.h"

/**
 * subclass of TestSimpleExplanations that verifies non matches.
 */
class TestSpanExplanationsOfNonMatches : public TestSpanExplanations 
{
public:
    TestSpanExplanationsOfNonMatches( CuTest * tc );
    virtual ~TestSpanExplanationsOfNonMatches();

    /**
     * Overrides superclass to ignore matches and focus on non-matches
     * @see CheckHits#checkNoMatchExplanations
     */
    virtual void qtest( Query * q, int32_t * expDocNrs, size_t expDocNrsCount );
};
#endif

