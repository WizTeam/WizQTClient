/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

#include "CLucene/snowball/SnowballAnalyzer.h"

CL_NS_USE2(analysis, snowball);

void testSnowball(CuTest *tc) {
    SnowballAnalyzer an(_T("English"));
    CL_NS(util)::StringReader reader(_T("he abhorred accents"));

    TokenStream* ts = an.tokenStream(_T("test"), &reader);
    Token t;
    CLUCENE_ASSERT(ts->next(&t)!=NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("he")) == 0);
    CLUCENE_ASSERT(ts->next(&t)!=NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("abhor")) == 0);
    CLUCENE_ASSERT(ts->next(&t)!=NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("accent")) == 0);

    CLUCENE_ASSERT(ts->next(&t) == NULL);
    _CLDELETE(ts);
}

CuSuite *testsnowball(void) {
    CuSuite *suite = CuSuiteNew(_T("CLucene Snowball Test"));

    SUITE_ADD_TEST(suite, testSnowball);

    return suite;
}
