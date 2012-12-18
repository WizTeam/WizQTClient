/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jiri Splichal and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"


/////////////////////////////////////////////////////////////////////////////
Directory * setUpIndex()
{
    TCHAR       tbuffer[16];
    const TCHAR*      data[] = 
    {   
        _T( "aaaaa aaaab aaabb aabbb abbbb bbbbb" ),
        _T( "aaaaa aaaac aaacc aaccc acccc ccccc" )
    };
    
    RAMDirectory * pDirectory = _CLNEW RAMDirectory();
    WhitespaceAnalyzer analyzer;
    IndexWriter writer( pDirectory, &analyzer, true );
    for( int i = 0; i < sizeof( data ) / sizeof( data[0] ); i++ )
    {
        _itot( i, tbuffer, 10 ); 
        Document doc;
        doc.add( * _CLNEW Field( _T( "id" ), tbuffer, Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
        doc.add( * _CLNEW Field( _T( "data" ), data[ i ], Field::STORE_YES | Field::INDEX_TOKENIZED ));
        writer.addDocument( &doc );
    }
    
    writer.optimize();
    writer.close();
    return pDirectory;
}


/////////////////////////////////////////////////////////////////////////////
void closeIndex( Directory * pDirectory )
{
    if( pDirectory )
    {
        pDirectory->close();
        _CLLDECDELETE( pDirectory );
    }
}


/////////////////////////////////////////////////////////////////////////////
void clearTermSet( TermSet& termSet )
{
    for( TermSet::iterator itTerms = termSet.begin(); itTerms != termSet.end(); itTerms++ )
    {
        Term * pTerm = *itTerms;
        _CLLDECDELETE( pTerm );
    }
    termSet.clear();
}


/////////////////////////////////////////////////////////////////////////////
void testExtractFromTermQuery( CuTest * tc )
{
    Directory *     pIndex  = setUpIndex();
    IndexReader *   pReader = IndexReader::open( pIndex );
    TermSet         termSet;
 
    Term * t1 = _CLNEW Term( _T("data"), _T("aaaaa") );
    Term * t2 = _CLNEW Term( _T("data"), _T("bbbbb") );
    Query * q1 = _CLNEW TermQuery( t1 );
    Query * q2 = _CLNEW TermQuery( t2 );
    Query * rewrite1 = q1->rewrite( pReader );
    Query * rewrite2 = q2->rewrite( pReader );

    rewrite1->extractTerms( &termSet );
    assertEqualsMsg( _T( "wrong number of terms" ), 1, termSet.size() );
    assertEqualsMsg( _T( "wrong term" ), 0, t1->compareTo( *(termSet.begin())) );
    clearTermSet( termSet );

    rewrite2->extractTerms( &termSet );
    assertEqualsMsg( _T( "wrong number of terms" ), 1, termSet.size() );
    assertEqualsMsg( _T( "wrong term" ), 0, t2->compareTo( *(termSet.begin())) );
    clearTermSet( termSet );

    _CLLDECDELETE( t1 );
    _CLLDECDELETE( t2 );

    if( q1 != rewrite1 )
        _CLDELETE( rewrite1 );
    _CLDELETE( q1 );
    
    if( q2 != rewrite2 )
        _CLDELETE( rewrite2 );
    _CLDELETE( q2 );
    
    pReader->close();
    _CLDELETE( pReader );

    closeIndex( pIndex );
    pIndex = NULL;
}


/////////////////////////////////////////////////////////////////////////////
void testExtractFromPhraseQuery( CuTest * tc )
{
    Directory *     pIndex  = setUpIndex();
    IndexReader *   pReader = IndexReader::open( pIndex );
    TermSet         termSet;
 
    Term * t1 = _CLNEW Term( _T("data"), _T("aaaab") );
    Term * t2 = _CLNEW Term( _T("data"), _T("ccccc") );
    Term * t3 = _CLNEW Term( _T("data"), _T("aaaab") );
    PhraseQuery * phrase = _CLNEW PhraseQuery();
    phrase->add( t1 ); 
    phrase->add( t2 ); 
    phrase->add( t3 ); 

    Query * rewrite = phrase->rewrite( pReader );

    rewrite->extractTerms( &termSet );
    assertEqualsMsg( _T( "wrong number of terms" ), 2, termSet.size() );
    for( TermSet::iterator itTerms = termSet.begin(); itTerms != termSet.end(); itTerms++ )
    {
        Term * pTerm = *itTerms;
        assertTrueMsg( _T( "wrong term" ), ( 0 == t1->compareTo( pTerm ) || 0 == t2->compareTo( pTerm )));
    }
    clearTermSet( termSet );

    _CLLDECDELETE( t1 );
    _CLLDECDELETE( t2 );
    _CLLDECDELETE( t3 );

    if( rewrite != phrase )
        _CLDELETE( rewrite );
    _CLDELETE( phrase );
    
    pReader->close();
    _CLDELETE( pReader );

    closeIndex( pIndex );
    pIndex = NULL;
}


/////////////////////////////////////////////////////////////////////////////
void testExtractFromBooleanQuery( CuTest * tc )
{
    Directory *     pIndex  = setUpIndex();
    IndexReader *   pReader = IndexReader::open( pIndex );
    TermSet         termSet;
 
    Term * t1 = _CLNEW Term( _T("data"), _T("aaaab") );
    Term * t2 = _CLNEW Term( _T("data"), _T("aaabb") );
    Term * t3 = _CLNEW Term( _T("data"), _T("aaabb") );
    BooleanQuery * bq = _CLNEW BooleanQuery();
    bq->add( _CLNEW TermQuery( t1 ), true, BooleanClause::SHOULD );
    bq->add( _CLNEW TermQuery( t2 ), true, BooleanClause::SHOULD );
    bq->add( _CLNEW TermQuery( t3 ), true, BooleanClause::SHOULD );

    Query * rewrite = bq->rewrite( pReader );

    rewrite->extractTerms( &termSet );
    assertEqualsMsg( _T( "wrong number of terms" ), 2, termSet.size() );
    for( TermSet::iterator itTerms = termSet.begin(); itTerms != termSet.end(); itTerms++ )
    {
        Term * pTerm = *itTerms;
        assertTrueMsg( _T( "wrong term" ), ( 0 == t1->compareTo( pTerm ) || 0 == t2->compareTo( pTerm )));
    }
    clearTermSet( termSet );

    _CLLDECDELETE( t1 );
    _CLLDECDELETE( t2 );
    _CLLDECDELETE( t3 );

    if( rewrite != bq )
        _CLDELETE( rewrite );
    _CLDELETE( bq );
    
    pReader->close();
    _CLDELETE( pReader );

    closeIndex( pIndex );
    pIndex = NULL;
}


/////////////////////////////////////////////////////////////////////////////
void testExtractFromWildcardQuery( CuTest * tc )
{
    Directory *     pIndex  = setUpIndex();
    IndexReader *   pReader = IndexReader::open( pIndex );
    TermSet         termSet;
    WildcardQuery * wildcard;
    Term *          t1;
    Query *         rewrite;


    t1 = _CLNEW Term( _T("data"), _T("aaaa?") );
    wildcard = _CLNEW WildcardQuery( t1 );
    rewrite = wildcard->rewrite( pReader );
    rewrite->extractTerms( &termSet );
    _CLLDECDELETE( t1 );

    assertEqualsMsg( _T( "wrong number of terms" ), 3, termSet.size() );
    for( TermSet::iterator itTerms = termSet.begin(); itTerms != termSet.end(); itTerms++ )
    {
        Term * pTerm = *itTerms;
        if(    0 != _tcscmp( _T( "aaaaa" ), pTerm->text()) 
            && 0 != _tcscmp( _T( "aaaab" ), pTerm->text())
            && 0 != _tcscmp( _T( "aaaac" ), pTerm->text()))
        {
            assertTrueMsg( _T( "wrong term" ), false );
        }
    }

    clearTermSet( termSet );
    if( rewrite != wildcard )
        _CLDELETE( rewrite );
    _CLDELETE( wildcard );
    

    t1 = _CLNEW Term( _T("data"), _T("aaa*") );
    wildcard = _CLNEW WildcardQuery( t1 );
    rewrite = wildcard->rewrite( pReader );
    rewrite->extractTerms( &termSet );
    _CLLDECDELETE( t1 );

    assertEqualsMsg( _T( "wrong number of terms" ), 5, termSet.size() );
    for( TermSet::iterator itTerms = termSet.begin(); itTerms != termSet.end(); itTerms++ )
    {
        Term * pTerm = *itTerms;
        assertTrueMsg( _T( "wrong term" ), ( 0 == _tcsncmp( _T( "aaa" ), pTerm->text(), 3 )));
    }

    clearTermSet( termSet );
    if( rewrite != wildcard )
        _CLDELETE( rewrite );
    _CLDELETE( wildcard );


    pReader->close();
    _CLDELETE( pReader );

    closeIndex( pIndex );
    pIndex = NULL;
}


/////////////////////////////////////////////////////////////////////////////
void testExtractFromFuzzyQuery( CuTest * tc )
{
    Directory *     pIndex  = setUpIndex();
    IndexReader *   pReader = IndexReader::open( pIndex );
    TermSet         termSet;
    FuzzyQuery *    fuzzy;
    Term *          t1;
    Query *         rewrite;


    t1 = _CLNEW Term( _T("data"), _T("aaaab") );
    fuzzy = _CLNEW FuzzyQuery( t1, 0.7f );
    rewrite = fuzzy->rewrite( pReader );
    rewrite->extractTerms( &termSet );
    _CLLDECDELETE( t1 );

    assertEqualsMsg( _T( "wrong number of terms" ), 4, termSet.size() );
    for( TermSet::iterator itTerms = termSet.begin(); itTerms != termSet.end(); itTerms++ )
    {
        Term * pTerm = *itTerms;
        if(    0 != _tcscmp( _T( "aaaaa" ), pTerm->text()) 
            && 0 != _tcscmp( _T( "aaaab" ), pTerm->text())
            && 0 != _tcscmp( _T( "aaabb" ), pTerm->text())
            && 0 != _tcscmp( _T( "aaaac" ), pTerm->text()))
        {
            assertTrueMsg( _T( "wrong term" ), false );
        }
    }

    clearTermSet( termSet );
    if( rewrite != fuzzy )
        _CLDELETE( rewrite );
    _CLDELETE( fuzzy );
    
    pReader->close();
    _CLDELETE( pReader );

    closeIndex( pIndex );
    pIndex = NULL;
}


/////////////////////////////////////////////////////////////////////////////
// Custom CLucene tests
CuSuite *testExtractTerms(void)
{
	CuSuite *suite = CuSuiteNew( _T( "CLucene ExtractTerms Test" ));

	SUITE_ADD_TEST( suite, testExtractFromTermQuery );
	SUITE_ADD_TEST( suite, testExtractFromPhraseQuery );
	SUITE_ADD_TEST( suite, testExtractFromBooleanQuery );
	SUITE_ADD_TEST( suite, testExtractFromWildcardQuery );
	SUITE_ADD_TEST( suite, testExtractFromFuzzyQuery );

	return suite; 
}


// EOF
