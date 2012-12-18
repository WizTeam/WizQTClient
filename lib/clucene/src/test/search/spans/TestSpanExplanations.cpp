/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestSpanExplanations.h"
#include "CLucene/search/spans/SpanQuery.h"
#include "CLucene/search/spans/SpanOrQuery.h"
#include "CLucene/search/spans/SpanTermQuery.h"
#include "CLucene/search/spans/SpanNotQuery.h"
#include "CLucene/search/spans/SpanNearQuery.h"
#include "CLucene/search/spans/SpanFirstQuery.h"

TestSpanExplanations::TestSpanExplanations( CuTest * tc )
: TestExplanations( tc )
{
}

TestSpanExplanations::~TestSpanExplanations()
{
}

void TestSpanExplanations::testST1()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = st( _T( "w1" ));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testST2()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = st( _T( "w1" ));
    q->setBoost(1000);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testST4()
{
    int32_t expDocs[] = {2,3};
    SpanQuery * q = st( _T( "xx" ));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testST5()
{
    int32_t expDocs[] = {2,3};
    SpanQuery * q = st( _T( "xx" ));
    q->setBoost(1000);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSF1()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = sf(( _T( "w1" )),1);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSF2()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = sf(( _T( "w1" )),1);
    q->setBoost(1000);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSF4()
{
    int32_t expDocs[] = {2};
    SpanQuery * q = sf(( _T( "xx" )),2);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSF5()
{
    SpanQuery * q = sf(( _T( "yy" )),2);
    qtest( q, NULL, 0 );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSF6()
{
    int32_t expDocs[] = {2};
    SpanQuery * q = sf(( _T( "yy" )),4);
    q->setBoost(1000);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}
  
void TestSpanExplanations::testSO1()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = sor( _T( "w1" ), _T( "QQ" ));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSO2()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = sor( _T( "w1" ), _T( "w3" ), _T( "zz" ));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSO3()
{
    int32_t expDocs[] = {0,2,3};
    SpanQuery * q = sor( _T( "w5" ), _T( "QQ" ), _T( "yy" ));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSO4()
{
    int32_t expDocs[] = {0,2,3};
    SpanQuery * q = sor( _T( "w5" ), _T( "QQ" ), _T( "yy" ));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear1()
{
    SpanQuery * q = snear( _T( "w1" ), _T( "QQ" ),100,true);
    qtest( q, NULL, 0 );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear2()
{
    int32_t expDocs[] = {2,3};
    SpanQuery * q = snear( _T( "w1" ), _T( "xx" ),100,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear3()
{
    int32_t expDocs[] = {2};
    SpanQuery * q = snear( _T( "w1" ), _T( "xx" ),0,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear4()
{
    int32_t expDocs[] = {2,3};
    SpanQuery * q = snear( _T( "w1" ), _T( "xx" ),1,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear5()
{
    int32_t expDocs[] = {2};
    SpanQuery * q = snear( _T( "xx" ), _T( "w1" ),0,false);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear6()
{
    SpanQuery * q = snear( _T( "w1" ), _T( "w2" ), _T( "QQ" ),100,true);
    qtest( q, NULL, 0 );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear7()
{
    int32_t expDocs[] = {2,3};
    SpanQuery * q = snear( _T( "w1" ), _T( "xx" ), _T( "w2" ),100,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear8()
{
    int32_t expDocs[] = {2};
    SpanQuery * q = snear( _T( "w1" ), _T( "xx" ), _T( "w2" ),0,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear9()
{
    int32_t expDocs[] = {2,3};
    SpanQuery * q = snear( _T( "w1" ), _T( "xx" ), _T( "w2" ),1,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear10()
{
    int32_t expDocs[] = {2};
    SpanQuery * q = snear( _T( "xx" ), _T( "w1" ), _T( "w2" ),0,false);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNear11()
{
    int32_t expDocs[] = {0,1};
    SpanQuery * q = snear( _T( "w1" ), _T( "w2" ), _T( "w3" ),1,true);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNot1()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = snot(sf( _T( "w1" ),10),st( _T( "QQ" )));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNot2()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = snot(sf( _T( "w1" ),10),st( _T( "QQ" )));
    q->setBoost(1000);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNot4()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = snot(sf( _T( "w1" ),10),st( _T( "xx" )));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNot5()
{
    int32_t expDocs[] = {0,1,2,3};
    SpanQuery * q = snot(sf( _T( "w1" ),10),st( _T( "xx" )));
    q->setBoost(1000);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNot7()
{
    int32_t expDocs[] = {0,1,3};
    SpanQuery * f = snear( _T( "w1" ), _T( "w3" ),10,true);
    f->setBoost(1000);
    SpanQuery * q = snot(f, st( _T( "xx" )));
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}

void TestSpanExplanations::testSNot10()
{
    int32_t expDocs[] = {0,1,3};
    SpanQuery * t = st( _T( "xx" ));
    t->setBoost(10000);
    SpanQuery * q = snot(snear( _T( "w1" ), _T( "w3" ),10,true), t);
    qtest( q, expDocs, sizeof(expDocs)/sizeof(expDocs[0]) );
    _CLLDELETE( q );
}
