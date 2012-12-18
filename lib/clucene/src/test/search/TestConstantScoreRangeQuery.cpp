/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
*
------------------------------------------------------------------------------*/

#include "test.h"

#include "QueryUtils.h"
#include "BaseTestRangeFilter.h"
#include "CLucene/search/ConstantScoreQuery.h"


/*------------------------------------------------------------------------------
* Test based on JLucene-2.3.2 (06/08/2010)
------------------------------------------------------------------------------*/
class TestConstantScoreRangeQuery : public BaseTestRangeFilter
{
private:
    Directory * m_pSmall;

public:
    TestConstantScoreRangeQuery( CuTest * _tc ) : BaseTestRangeFilter( _tc ), m_pSmall( NULL ) {}
	~TestConstantScoreRangeQuery()  
    {
        if( m_pSmall )
        {
            m_pSmall->close();
            _CLLDECDELETE( m_pSmall );
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    void setUp()
    {
        TCHAR       tbuffer[16];
        const TCHAR*      data[] = 
        {   
            _T( "A 1 2 3 4 5 6" ),
            _T( "Z       4 5 6" ),
            NULL,
            _T( "B   2   4 5 6" ),
            _T( "Y     3   5 6" ),
            NULL,
            _T( "C     3     6" ),
            _T( "X       4 5 6" )
        };
        
        m_pSmall = _CLNEW RAMDirectory();
        Analyzer * pAnalyzer = _CLNEW WhitespaceAnalyzer();
        IndexWriter * pWriter = _CLNEW IndexWriter( m_pSmall, pAnalyzer, true );
        
        for( size_t i = 0; i < sizeof( data ) / sizeof( data[0] ); i++ )
        {
            _itot( i, tbuffer, 10 ); 
            Document doc;
            doc.add( * _CLNEW Field( _T( "id" ), tbuffer, Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
            doc.add( * _CLNEW Field( _T( "all" ), _T( "all" ), Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
            if( data[ i ] )
                doc.add( * _CLNEW Field( _T( "data" ), data[ i ], Field::STORE_YES | Field::INDEX_TOKENIZED ));

            pWriter->addDocument( &doc );
        }
        
        pWriter->optimize();
        pWriter->close();
        _CLDELETE( pWriter );
        _CLDELETE( pAnalyzer );
    }


    /////////////////////////////////////////////////////////////////////////////
    Query * csrq(const TCHAR* f, const TCHAR* l, const TCHAR* h, bool il, bool ih) 
    {
        return _CLNEW ConstantScoreRangeQuery( f, l, h, il, ih );
    }


    /////////////////////////////////////////////////////////////////////////////
    void testBasics() 
    {
        Query * q1 = csrq( _T( "data" ), _T( "1" ), _T( "6" ), true, true );
        Query * q2 = csrq( _T( "data" ), _T( "A" ) ,_T( "Z" ), true, true );
        QueryUtils::check( tc, q1 );
        QueryUtils::check( tc, q2 );
        QueryUtils::checkUnequal( tc, q1, q2 );
        _CLLDELETE( q2 );
        _CLLDELETE( q1 );
    }

    /////////////////////////////////////////////////////////////////////////////
    void testEqualScores() 
    {
        // NOTE: uses index build in *this* setUp
        
        IndexReader * pReader = IndexReader::open( m_pSmall );
	    IndexSearcher * pSearch = _CLNEW IndexSearcher( pReader );

	    Hits * pResult;

        // some hits match more terms then others, score should be the same
        Query * q = csrq( _T( "data" ), _T( "1" ), _T( "6" ), true, true );
        pResult = pSearch->search( q );
        size_t numHits = pResult->length();
        assertEqualsMsg( _T( "wrong number of results" ), 6, numHits );
        float_t score = pResult->score( 0 );
        for( size_t i = 1; i < numHits; i++ )
        {
            assertTrueMsg( _T( "score was not the same" ), score == pResult->score( i ));
        }
        _CLDELETE( pResult );
        _CLDELETE( q );

        pSearch->close();
        _CLDELETE( pSearch );

        pReader->close();
        _CLDELETE( pReader );
    }

    
    /////////////////////////////////////////////////////////////////////////////
    void testBoost()
    {
        // NOTE: uses index build in *this* setUp

        IndexReader * pReader = IndexReader::open( m_pSmall );
	    IndexSearcher * pSearch = _CLNEW IndexSearcher( pReader );
	    Hits * pResult;

        // test for correct application of query normalization
        // must use a non score normalizing method for this.
        Query * q = csrq( _T( "data" ), _T( "1" ), _T( "6" ), true, true );
        q->setBoost( 100 );
        pResult = pSearch->search( q );
        for( size_t i = 1; i < pResult->length(); i++ )
        {
            assertTrueMsg( _T( "score was not was not correct" ), 1.0f == pResult->score( i ));
        }
        _CLDELETE( pResult );
        _CLDELETE( q );


        //
        // Ensure that boosting works to score one clause of a query higher
        // than another.
        //
        Query * q1 = csrq( _T( "data" ), _T( "A" ), _T( "A" ), true, true );  // matches document #0
        q1->setBoost( .1f );
        Query * q2 = csrq( _T( "data" ), _T( "Z" ), _T( "Z" ), true, true );  // matches document #1
        BooleanQuery * bq = _CLNEW BooleanQuery( true );
        bq->add( q1, true, BooleanClause::SHOULD );
        bq->add( q2, true, BooleanClause::SHOULD );

        pResult = pSearch->search( bq );
        assertEquals( 1, pResult->id( 0 ));
        assertEquals( 0, pResult->id( 1 ));
        assertTrue( pResult->score( 0 ) > pResult->score( 1 ));
        _CLDELETE( pResult );
        _CLDELETE( bq );

        q1 = csrq( _T( "data" ), _T( "A" ), _T( "A" ), true, true );  // matches document #0
        q1->setBoost( 10.0f );
        q2 = csrq( _T( "data" ), _T( "Z" ), _T( "Z" ), true, true );  // matches document #1
        bq = _CLNEW BooleanQuery( true );
        bq->add( q1, true, BooleanClause::SHOULD );
        bq->add( q2, true, BooleanClause::SHOULD );

        pResult = pSearch->search( bq );
        assertEquals( 0, pResult->id( 0 ));
        assertEquals( 1, pResult->id( 1 ));
        assertTrue( pResult->score( 0 ) > pResult->score( 1 ));
        _CLDELETE( pResult );
        _CLDELETE( bq );

        pSearch->close();
        _CLDELETE( pSearch );

        pReader->close();
        _CLDELETE( pReader );
    }

    
    /////////////////////////////////////////////////////////////////////////////
    void testBooleanOrderUnAffected()
    {
        // NOTE: uses index build in *this* setUp

        IndexReader * pReader = IndexReader::open( m_pSmall );
	    IndexSearcher * pSearch = _CLNEW IndexSearcher( pReader );

        // first do a regular RangeQuery which uses term expansion so
        // docs with more terms in range get higher scores
        Term * pLower = _CLNEW Term( _T( "data" ), _T( "1" ));
        Term * pUpper = _CLNEW Term( _T( "data" ), _T( "4" ));
        Query * rq = _CLNEW RangeQuery( pLower, pUpper, true );
        _CLLDECDELETE( pUpper );
        _CLLDECDELETE( pLower );

        Hits * pExpected = pSearch->search( rq );
        size_t numHits = pExpected->length();
 
        // now do a boolean where which also contains a
        // ConstantScoreRangeQuery and make sure the order is the same
        
        BooleanQuery * q = _CLNEW BooleanQuery();
        q->add( rq, true, BooleanClause::MUST );
        q->add( csrq( _T( "data" ), _T( "1" ), _T( "6" ), true, true ), true, BooleanClause::MUST );
 
        Hits * pActual = pSearch->search( q );
        assertEqualsMsg( _T( "wrong number of hits" ), numHits, pActual->length() );
        for( size_t i = 0; i < numHits; i++ )
        {
            assertEqualsMsg( _T( "mismatch in docid for a hit" ), pExpected->id( i ), pActual->id( i ));
        }
        _CLDELETE( pActual );
        _CLDELETE( pExpected );
        _CLDELETE( q );

        pSearch->close();
        _CLDELETE( pSearch );

        pReader->close();
        _CLDELETE( pReader );
    }

    /////////////////////////////////////////////////////////////////////////////
    void testRangeQueryId()
    {
        // NOTE: uses index build in *super* setUp

        IndexReader * pReader = IndexReader::open( index );
	    IndexSearcher * pSearch = _CLNEW IndexSearcher( pReader );

        int32_t medId = ((maxId - minId) / 2);
        
        std::tstring sMinIP = pad(minId);
        std::tstring sMaxIP = pad(maxId);
        std::tstring sMedIP = pad(medId);
        const TCHAR* minIP = sMinIP.c_str();
        const TCHAR* maxIP = sMaxIP.c_str();
        const TCHAR* medIP = sMedIP.c_str();
    
        size_t numDocs = static_cast<size_t>( pReader->numDocs() );
        assertEqualsMsg( _T("num of docs"), numDocs, static_cast<size_t>(1+ maxId - minId));
        
	    Hits * pResult;
        Query * q;
        // test id, bounded on both ends
        
        q = csrq( _T( "id" ), minIP, maxIP, true, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "find all" ), numDocs, pResult->length() );
        _CLDELETE( pResult ); _CLDELETE( q );

	    q = csrq( _T( "id" ), minIP, maxIP, true, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "all but last" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

	    q = csrq( _T( "id" ), minIP, maxIP, false, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "all but first" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
            
	    q = csrq( _T( "id" ), minIP, maxIP, false,false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "all but ends" ), numDocs-2, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
    
        q = csrq( _T( "id" ), medIP, maxIP, true, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "med and up" ), 1+maxId-medId, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
        
        q = csrq( _T( "id" ), minIP, medIP, true, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "up to med" ), 1+medId-minId, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        // unbounded id

	    q = csrq( _T( "id" ), minIP, NULL, true, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "min and up" ), numDocs, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

	    q = csrq( _T( "id" ), NULL, maxIP, false, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "max and down" ), numDocs, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

	    q = csrq( _T( "id" ), minIP, NULL, false, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "not min, but up" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
            
	    q = csrq( _T( "id" ), NULL, maxIP, false, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "not max, but down" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
            
        q = csrq( _T( "id" ), medIP, maxIP, true, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "med and up, not max" ), maxId-medId, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
        
        q = csrq( _T( "id" ), minIP, medIP, false,true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "not min, up to med" ), medId-minId, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        // very small sets

	    q = csrq( _T( "id" ), minIP, minIP, false, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "min,min,F,F" ), 0, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "id" ), medIP, medIP, false, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "med,med,F,F" ), 0, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "id") , maxIP, maxIP, false, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "max,max,F,F" ), 0, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
                         
	    q = csrq( _T( "id" ), minIP, minIP, true, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "min,min,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "id" ), NULL, minIP, false, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "nul,min,F,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

	    q = csrq( _T( "id" ), maxIP, maxIP, true, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "max,max,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "id" ), maxIP, NULL, true, false );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "max,nul,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

	    q = csrq( _T( "id" ), medIP, medIP, true, true );
	    pResult = pSearch->search( q );
	    assertEqualsMsg( _T( "med,med,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
            
        pSearch->close();
        _CLDELETE( pSearch );

        pReader->close();
        _CLDELETE( pReader );
    }

    
    /////////////////////////////////////////////////////////////////////////////
    void testRangeQueryRand()
    {
        // NOTE: uses index build in *super* setUp

        IndexReader * pReader = IndexReader::open( index );
	    IndexSearcher * pSearch = _CLNEW IndexSearcher( pReader );

        std::tstring sMinRP = pad(minR);
        std::tstring sMaxRP = pad(maxR);
        const TCHAR* minRP = sMinRP.c_str();
        const TCHAR* maxRP = sMaxRP.c_str();
    
        size_t numDocs = static_cast<size_t>( pReader->numDocs() );
        assertEqualsMsg( _T("num of docs"), numDocs, static_cast<size_t>(1+ maxId - minId));
        
    	Hits * pResult;
        Query * q;

        // test extremes, bounded on both ends
        
        q = csrq( _T( "rand" ), minRP, maxRP, true, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "find all" ), numDocs, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), minRP, maxRP, true, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "all but biggest" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), minRP, maxRP, false, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "all but smallest" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), minRP, maxRP, false, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "all but extremes" ), numDocs-2, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
    
        // unbounded

        q = csrq( _T( "rand" ), minRP, NULL, true, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "smallest and up" ), numDocs, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), NULL, maxRP, false, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "biggest and down" ), numDocs, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), minRP, NULL, false, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "not smallest, but up" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
            
        q = csrq( _T( "rand" ), NULL, maxRP, false, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "not biggest, but down" ), numDocs-1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
        
        // very small sets

        q = csrq( _T( "rand" ), minRP, minRP, false, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "min,min,F,F" ), 0, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), maxRP, maxRP, false, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "max,max,F,F" ), 0, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );
                         
        q = csrq( _T( "rand" ), minRP, minRP, true, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "min,min,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), NULL, minRP, false, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "nul,min,F,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), maxRP, maxRP, true, true );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "max,max,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        q = csrq( _T( "rand" ), maxRP, NULL, true, false );
	    pResult = pSearch->search( q );
        assertEqualsMsg( _T( "max,nul,T,T" ), 1, pResult->length());
        _CLDELETE( pResult ); _CLDELETE( q );

        pSearch->close();
        _CLDELETE( pSearch );

        pReader->close();
        _CLDELETE( pReader );
    }


    /////////////////////////////////////////////////////////////////////////////
    // CLucene specific 
    // Visual Studio 2005 shows memory leaks for this test, but some other 
    // tools do not detect any memory leaks. So what is right?
    // IN VC80 shows memory leaks ONLY if both sub-queries are added as 
    // MUST BooleanClauses. 
    void testBooleanMemLeaks()
    {
        IndexReader * pReader = IndexReader::open( m_pSmall );
	    IndexSearcher * pSearch = _CLNEW IndexSearcher( pReader );
 
        Query * q1 = csrq( _T( "data" ), _T( "A" ), _T( "A" ), true, true );  // matches document #0
        Query * q2 = csrq( _T( "data" ), _T( "Z" ), _T( "Z" ), true, true );  // matches document #1
        BooleanQuery * bq = _CLNEW BooleanQuery( true );
        bq->add( q1, true, BooleanClause::MUST );
        bq->add( q2, true, BooleanClause::MUST );

        Hits * pResult = pSearch->search( bq );

        _CLDELETE( pResult );
        _CLDELETE( bq );

        pSearch->close();
        _CLDELETE( pSearch );

        pReader->close();
        _CLDELETE( pReader );
    }


    /////////////////////////////////////////////////////////////////////////////
    void runTests()
    {
        setUp();
        testBasics();
        testEqualScores(); 
        testBoost();
        testBooleanOrderUnAffected();
        testRangeQueryId();
        testRangeQueryRand();
        testBooleanMemLeaks();
    }
};


/////////////////////////////////////////////////////////////////////////////
void testConstantScoreRangeQuery( CuTest * pTc )
{
	/// Run Java Lucene tests
	TestConstantScoreRangeQuery tester( pTc );
	tester.runTests();
}


/////////////////////////////////////////////////////////////////////////////
CuSuite *testConstantScoreQueries(void)
{
	CuSuite *suite = CuSuiteNew( _T( "CLucene ConstScoreQuery Test" ));

	SUITE_ADD_TEST(suite, testConstantScoreRangeQuery);

	return suite; 
}


// EOF
