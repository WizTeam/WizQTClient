/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestSpanExplanationsOfNonMatches.h"
#include "../CheckHits.h"

TestSpanExplanationsOfNonMatches::TestSpanExplanationsOfNonMatches( CuTest * tc )
: TestSpanExplanations( tc )
{
}

TestSpanExplanationsOfNonMatches::~TestSpanExplanationsOfNonMatches()
{
}

void TestSpanExplanationsOfNonMatches::qtest( Query * q, int32_t * expDocNrs, size_t expDocNrsCount )
{
    CheckHits::checkNoMatchExplanations( tc, q, field, searcher, expDocNrs, expDocNrsCount );
}
