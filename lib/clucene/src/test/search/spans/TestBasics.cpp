/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "../QueryUtils.h"
#include "../CheckHits.h"
#include "TestBasics.h"
#include "CLucene/search/spans/SpanQuery.h"
#include "CLucene/search/spans/SpanTermQuery.h"
#include "CLucene/search/spans/SpanNearQuery.h"
#include "CLucene/search/spans/SpanNotQuery.h"
#include "CLucene/search/spans/SpanOrQuery.h"
#include "CLucene/search/spans/SpanFirstQuery.h"

CL_NS_USE2(search,spans);

TestBasics::TestBasics( CuTest* tc )
{
    this->tc = tc;
    this->searcher = NULL;
    this->directory = NULL;
}

TestBasics::~TestBasics()
{
    if( searcher )
    {
        searcher->close();
        _CLDELETE( searcher );
    }

    if( directory )
    {
        directory->close();
        _CLDELETE( directory );
    }
}

void TestBasics::setUp()
{
    directory = _CLNEW RAMDirectory();
    Analyzer * analyzer = _CLNEW SimpleAnalyzer();
    IndexWriter * writer = _CLNEW IndexWriter( directory, analyzer, true );
    TCHAR buffer[ 200 ];

    for( int32_t i = 0; i < 1000; i++ )
    {
        Document doc;
        English::IntToEnglish( i, buffer, 200 );
        doc.add( * _CLNEW Field( _T( "field" ), buffer, Field::STORE_YES | Field::INDEX_TOKENIZED ));
        writer->addDocument( &doc );
    }
    
    writer->close();
    _CLDELETE( writer );
    _CLDELETE( analyzer );

    searcher = _CLNEW IndexSearcher( directory );
}
  
void TestBasics::testTerm()
{
    int32_t expectedDocs[] =
    { 
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
        370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 470, 471, 472, 473,
        474, 475, 476, 477, 478, 479, 570, 571, 572, 573, 574, 575, 576, 577,
        578, 579, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 770, 771,
        772, 773, 774, 775, 776, 777, 778, 779, 870, 871, 872, 873, 874, 875,
        876, 877, 878, 879, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979
    };

    Term * term = _CLNEW Term( _T( "field" ), _T( "seventy" ));
    Query * query = new TermQuery( term );
    _CLLDECDELETE( term );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
    _CLLDELETE( query );
}

void TestBasics::testTerm2()
{
    Term * term = _CLNEW Term( _T( "field" ), _T( "seventish" ));
    Query * query = new TermQuery( term );
    _CLLDECDELETE( term );

    checkHits( query, NULL, 0 );
    _CLLDELETE( query );
}

void TestBasics::testPhrase()
{
    int32_t expectedDocs[] = { 77, 177, 277, 377, 477, 577, 677, 777, 877, 977 };

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "seventy" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "seven" ));

    PhraseQuery * query = _CLNEW PhraseQuery();
    query->add( term1 );
    query->add( term2 );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
    _CLLDELETE( query );
}

void TestBasics::testPhrase2()
{
    Term * term1 = _CLNEW Term( _T( "field" ), _T( "seventish" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "sevenon" ));

    PhraseQuery * query = _CLNEW PhraseQuery();
    query->add( term1 );
    query->add( term2 );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

    checkHits( query, NULL, 0 );
    _CLLDELETE( query );
}

void TestBasics::testBoolean()
{
    int32_t expectedDocs[] = {77, 777, 177, 277, 377, 477, 577, 677, 770, 771, 772, 773, 774, 775, 776, 778, 779, 877, 977};

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "seventy" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "seven" ));

    BooleanQuery * query = _CLNEW BooleanQuery();
    query->add( _CLNEW TermQuery( term1 ), true, BooleanClause::MUST );
    query->add( _CLNEW TermQuery( term2 ), true, BooleanClause::MUST );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
    _CLLDELETE( query );
}

void TestBasics::testBoolean2()
{
    Term * term1 = _CLNEW Term( _T( "field" ), _T( "sevento" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "sevenly" ));

    BooleanQuery * query = _CLNEW BooleanQuery();
    query->add( _CLNEW TermQuery( term1 ), true, BooleanClause::MUST );
    query->add( _CLNEW TermQuery( term2 ), true, BooleanClause::MUST );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

    checkHits( query, NULL, 0 );
    _CLLDELETE( query );
}

void TestBasics::testSpanNearExact()
{
    int32_t expectedDocs[] = {77, 177, 277, 377, 477, 577, 677, 777, 877, 977};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "seventy" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "seven" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * query = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );
    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    Explanation explanation1;
    searcher->explain( query, 77, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 977, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    QueryUtils::check( tc, clauses[ 0 ] );
    QueryUtils::check( tc, clauses[ 1 ] );
    QueryUtils::checkUnequal( tc, clauses[ 0 ], clauses[ 1 ] );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDELETE( query );
}

void TestBasics::testSpanNearUnordered()
{
    int32_t expectedDocs[] = {609, 629, 639, 649, 659, 669, 679, 689, 699, 906, 926, 936, 946, 956, 966, 976, 986, 996};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "nine" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "six" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * query = _CLNEW SpanNearQuery( clauses, clauses+2, 4, false, true );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

     checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
    _CLLDELETE( query );
}

void TestBasics::testSpanNearOrdered()
{
    int32_t expectedDocs[] = {906, 926, 936, 946, 956, 966, 976, 986, 996};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "nine" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "six" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * query = _CLNEW SpanNearQuery( clauses, clauses+2, 4, true, true );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
    _CLLDELETE( query );
}

void TestBasics::testSpanNot()
{
    int32_t expectedDocs[] = {801, 821, 831, 851, 861, 871, 881, 891};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "eight" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "one" ));
    Term * term3 = _CLNEW Term( _T( "field" ), _T( "forty" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * near = _CLNEW SpanNearQuery( clauses, clauses+2, 4, true, true );

    SpanNotQuery * query = _CLNEW SpanNotQuery( near, _CLNEW SpanTermQuery( term3 ), true );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDECDELETE( term3 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    Explanation explanation1;
    searcher->explain( query, 801, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 891, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}

void TestBasics::testSpanWithMultipleNotSingle()
{
    int32_t expectedDocs[] = {801, 821, 831, 851, 861, 871, 881, 891};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "eight" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "one" ));
    Term * term3 = _CLNEW Term( _T( "field" ), _T( "forty" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * near = _CLNEW SpanNearQuery( clauses, clauses+2, 4, true, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( term3 );
    SpanOrQuery * orQuery = _CLNEW SpanOrQuery( clauses, clauses+1, true );

    SpanNotQuery * query = _CLNEW SpanNotQuery( near, orQuery, true );
    
    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDECDELETE( term3 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    Explanation explanation1;
    searcher->explain( query, 801, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 891, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}

void TestBasics::testSpanWithMultipleNotMany()
{
    int32_t expectedDocs[] = {801, 821, 831, 851, 871, 891};
    SpanQuery * clauses[ 3 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "eight" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "one" ));
    Term * term3 = _CLNEW Term( _T( "field" ), _T( "forty" ));
    Term * term4 = _CLNEW Term( _T( "field" ), _T( "sixty" ));
    Term * term5 = _CLNEW Term( _T( "field" ), _T( "eighty" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * near = _CLNEW SpanNearQuery( clauses, clauses+2, 4, true, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( term3 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term4 );
    clauses[ 2 ] = _CLNEW SpanTermQuery( term5 );
    SpanOrQuery * orQuery = _CLNEW SpanOrQuery( clauses, clauses+3, true );

    SpanNotQuery * query = _CLNEW SpanNotQuery( near, orQuery, true );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDECDELETE( term3 );
    _CLLDECDELETE( term4 );
    _CLLDECDELETE( term5 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    Explanation explanation1;
    searcher->explain( query, 801, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 891, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}
    
void TestBasics::testNpeInSpanNearWithSpanNot()
{
    int32_t expectedDocs[] = {801, 821, 831, 851, 861, 871, 881, 891};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "eight" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "one" ));
    Term * hun   = _CLNEW Term( _T( "field" ), _T( "hundred" ));
    Term * term3 = _CLNEW Term( _T( "field" ), _T( "forty" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * near = _CLNEW SpanNearQuery( clauses, clauses+2, 4, true, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( hun );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term3 );
    SpanNearQuery * exclude = _CLNEW SpanNearQuery( clauses, clauses+2, 1, true, true );
    
    SpanNotQuery * query = _CLNEW SpanNotQuery( near, exclude, true );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDECDELETE( hun );
    _CLLDECDELETE( term3 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    Explanation explanation1;
    searcher->explain( query, 801, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 891, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}
  
void TestBasics::testNpeInSpanNearInSpanFirstInSpanNot()
{
    int32_t expectedDocs[] = {40,41,42,43,44,45,46,47,48,49};
    SpanQuery * clauses[ 2 ];
    int32_t n = 5;

    Term * hun   = _CLNEW Term( _T( "field" ), _T( "hundred" ));
    Term * term40 = _CLNEW Term( _T( "field" ), _T( "forty" ));

    SpanTermQuery * termQry40 = _CLNEW SpanTermQuery( term40 );
    clauses[ 0 ] = _CLNEW SpanTermQuery( hun );
    clauses[ 1 ] = (SpanTermQuery *) termQry40->clone();

    SpanFirstQuery * include = _CLNEW SpanFirstQuery( termQry40, n, true ); 
    SpanNearQuery * near = _CLNEW SpanNearQuery( clauses, clauses+2, n-1, true, true );
    SpanFirstQuery * exclude = _CLNEW SpanFirstQuery( near, n-1, true );
    SpanNotQuery * q = _CLNEW SpanNotQuery( include, exclude, true );
    
    _CLLDECDELETE( hun );
    _CLLDECDELETE( term40 );

    checkHits( q, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    _CLLDELETE( q );
}
  
void TestBasics::testSpanFirst()
{
    int32_t expectedDocs[] = 
    {
        5, 500, 501, 502, 503, 504, 505, 506, 507, 508, 509, 510, 511, 512, 513,
        514, 515, 516, 517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527,
        528, 529, 530, 531, 532, 533, 534, 535, 536, 537, 538, 539, 540, 541,
        542, 543, 544, 545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555,
        556, 557, 558, 559, 560, 561, 562, 563, 564, 565, 566, 567, 568, 569,
        570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583,
        584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597,
        598, 599
    };
    
    Term * term1 = _CLNEW Term( _T( "field" ), _T( "five" ));
    SpanFirstQuery * query = _CLNEW SpanFirstQuery( _CLNEW SpanTermQuery( term1 ), 1, true );
    _CLLDECDELETE( term1 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    Explanation explanation1;
    searcher->explain( query, 5, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 599, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}

void TestBasics::testSpanOr()
{
    int32_t expectedDocs[] = {33, 47, 133, 147, 233, 247, 333, 347, 433, 447, 533, 547, 633, 647, 733, 747, 833, 847, 933, 947};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "thirty" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "three" ));
    Term * term3 = _CLNEW Term( _T( "field" ), _T( "forty" ));
    Term * term4 = _CLNEW Term( _T( "field" ), _T( "seven" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * near1 = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( term3 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term4 );
    SpanNearQuery * near2 = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    clauses[ 0 ] = near1;
    clauses[ 1 ] = near2;
    SpanOrQuery * query = _CLNEW SpanOrQuery( clauses, clauses+2, true );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDECDELETE( term3 );
    _CLLDECDELETE( term4 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
      
    Explanation explanation1;
    searcher->explain( query, 33, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    Explanation explanation2;
    searcher->explain( query, 947, &explanation2 );
    assertTrue( explanation2.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}

void TestBasics::testSpanExactNested()
{
    int32_t expectedDocs[] = {333};
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "three" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "hundred" ));
    Term * term3 = _CLNEW Term( _T( "field" ), _T( "thirty" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanNearQuery * near1 = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( term3 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term1 );
    SpanNearQuery * near2 = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    clauses[ 0 ] = near1;
    clauses[ 1 ] = near2;
    SpanNearQuery * query = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );
    _CLLDECDELETE( term3 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));
      
    Explanation explanation1;
    searcher->explain( query, 333, &explanation1 );
    assertTrue( explanation1.getDetail( 0 )->getValue() > 0.0f );       // ToDo: Fix IndexSearcher::explain() method

    _CLLDELETE( query );
}

void TestBasics::testSpanNearOr()
{
    int32_t expectedDocs[] = 
    {
        606, 607, 626, 627, 636, 637, 646, 647, 
        656, 657, 666, 667, 676, 677, 686, 687, 696, 697,
        706, 707, 726, 727, 736, 737, 746, 747, 
        756, 757, 766, 767, 776, 777, 786, 787, 796, 797
    };
    SpanQuery * clauses[ 2 ];

    Term * term1 = _CLNEW Term( _T( "field" ), _T( "six" ));
    Term * term2 = _CLNEW Term( _T( "field" ), _T( "seven" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( term1 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term2 );
    SpanOrQuery * to1 = _CLNEW SpanOrQuery( clauses, clauses+2, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( term2 );
    clauses[ 1 ] = _CLNEW SpanTermQuery( term1 );
    SpanOrQuery * to2 = _CLNEW SpanOrQuery( clauses, clauses+2, true );

    clauses[ 0 ] = to1;
    clauses[ 1 ] = to2;
    SpanNearQuery * query = _CLNEW SpanNearQuery( clauses, clauses+2, 10, true, true );

    _CLLDECDELETE( term1 );
    _CLLDECDELETE( term2 );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    _CLLDELETE( query );
}

void TestBasics::testSpanComplex1()
{
    int32_t expectedDocs[] = 
    {
        606, 607, 626, 627, 636, 637, 646, 647, 
        656, 657, 666, 667, 676, 677, 686, 687, 696, 697,
        706, 707, 726, 727, 736, 737, 746, 747, 
        756, 757, 766, 767, 776, 777, 786, 787, 796, 797
    };
    SpanQuery * clauses[ 2 ];

    Term * termSix = _CLNEW Term( _T( "field" ), _T( "six" ));
    Term * termHun = _CLNEW Term( _T( "field" ), _T( "hundred" ));
    Term * termSev = _CLNEW Term( _T( "field" ), _T( "seven" ));

    clauses[ 0 ] = _CLNEW SpanTermQuery( termSix );
    clauses[ 1 ] = _CLNEW SpanTermQuery( termHun );
    SpanNearQuery * tt1 = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( termSev );
    clauses[ 1 ] = _CLNEW SpanTermQuery( termHun );
    SpanNearQuery * tt2 = _CLNEW SpanNearQuery( clauses, clauses+2, 0, true, true );

    clauses[ 0 ] = tt1;
    clauses[ 1 ] = tt2;
    SpanOrQuery * to1 = _CLNEW SpanOrQuery( clauses, clauses+2, true );

    clauses[ 0 ] = _CLNEW SpanTermQuery( termSev );
    clauses[ 1 ] = _CLNEW SpanTermQuery( termSix );
    SpanOrQuery * to2 = _CLNEW SpanOrQuery( clauses, clauses+2, true );
    
    clauses[ 0 ] = to1;
    clauses[ 1 ] = to2;
    SpanNearQuery * query = _CLNEW SpanNearQuery( clauses, clauses+2, 100, true, true );
    
    _CLLDECDELETE( termSix );
    _CLLDECDELETE( termHun );
    _CLLDECDELETE( termSev );

    checkHits( query, expectedDocs, sizeof( expectedDocs ) / sizeof( expectedDocs[ 0 ] ));

    _CLLDELETE( query );
}

void TestBasics::checkHits( Query * query, int32_t * results, size_t resultsCount )
{
    CheckHits::checkHits( tc, query, _T( "field" ), searcher, results, resultsCount );
}
