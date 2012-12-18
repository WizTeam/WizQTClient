/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "TestSpansAdvanced.h"
#include "../QueryUtils.h"
#include "CLucene/search/spans/SpanTermQuery.h"

CL_NS_USE2(search,spans);
CL_NS_USE(search);

const TCHAR * TestSpansAdvanced::field_id = _T( "ID" );
const TCHAR * TestSpansAdvanced::field_text = _T( "TEXT" );

TestSpansAdvanced::TestSpansAdvanced( CuTest* tc )
{
    this->tc = tc;
    this->searcher = NULL;
    this->directory = NULL;
}

TestSpansAdvanced::~TestSpansAdvanced()
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

void TestSpansAdvanced::setUp()
{
    directory = _CLNEW RAMDirectory();
    Analyzer * analyzer = _CLNEW StandardAnalyzer();
    IndexWriter * writer = _CLNEW IndexWriter( directory, analyzer, true );

    addDocuments( writer );

    writer->close();
    _CLDELETE( writer );
    _CLDELETE( analyzer );

    searcher = _CLNEW IndexSearcher( directory );
}

void TestSpansAdvanced::addDocuments( IndexWriter * writer )
{
    addDocument( writer, _T( "1" ), _T( "I think it should work." ));
    addDocument( writer, _T( "2" ), _T( "I think it should work." ));
    addDocument( writer, _T( "3" ), _T( "I think it should work." ));
    addDocument( writer, _T( "4" ), _T( "I think it should work." ));
}


void TestSpansAdvanced::addDocument( IndexWriter * writer, const TCHAR * id, const TCHAR * text )
{
    Document document;
    document.add( * _CLNEW Field( field_id, id, Field::STORE_YES | Field::INDEX_UNTOKENIZED ));
    document.add( * _CLNEW Field( field_text, text, Field::STORE_YES | Field::INDEX_TOKENIZED ));
    writer->addDocument( &document );
}

void TestSpansAdvanced::testBooleanQueryWithSpanQueries()
{
    doTestBooleanQueryWithSpanQueries( 0.3884282f );
}

void TestSpansAdvanced::doTestBooleanQueryWithSpanQueries( const float_t expectedScore )
{
    Term * t1 = _CLNEW Term( field_text, _T( "work" ));
    Query * spanQuery = _CLNEW SpanTermQuery( t1 );
    BooleanQuery * query = _CLNEW BooleanQuery();

    query->add( spanQuery, false, BooleanClause::MUST );
    query->add( spanQuery, false, BooleanClause::MUST );

    const TCHAR * expectedIds[] = { _T( "1" ), _T( "2" ), _T( "3" ), _T( "4" ) };
    float_t expectedScores[] = { expectedScore, expectedScore, expectedScore, expectedScore };

    assertHits( query, _T( "two span queries" ), expectedIds, expectedScores, 4 );

    _CLLDECDELETE( t1 );
    _CLLDELETE( spanQuery );
    _CLLDELETE( query );
}

void TestSpansAdvanced::assertHits( Query * query, const TCHAR * description, const TCHAR ** expectedIds, float_t * expectedScores, size_t expectedCount )
{
    float_t tolerance = 1e-5f;

    QueryUtils::check( tc, query, searcher );

    // Hits hits = searcher.search(query);
    // hits normalizes and throws things off if one score is greater than 1.0
    TopDocs * topdocs = searcher->_search( query, NULL, 10000 );

    /*****
    // display the hits
    System.out.println(hits.length() + " hits for search: \"" + description + '\"');
    for (int i = 0; i < hits.length(); i++) {
        System.out.println("  " + FIELD_ID + ':' + hits.doc(i).get(FIELD_ID) + " (score:" + hits.score(i) + ')');
    }
    *****/

    // did we get the hits we expected
    assertEquals( expectedCount, topdocs->totalHits );
    for( size_t i = 0; i < expectedCount; i++ )
    {
        //System.out.println(i + " exp: " + expectedIds[i]);
        //System.out.println(i + " field: " + hits.doc(i).get(FIELD_ID));

        int32_t id = topdocs->scoreDocs[ i ].doc;
        float_t score = topdocs->scoreDocs[ i ].score;
        Document doc;
        searcher->doc( id, doc );
        assertTrueMsg( _T( "actual document does not match the expected one" ), 0 == _tcscmp( expectedIds[ i ], doc.get( field_id )));
        assertTrueMsg( _T( "score does not match" ), ( expectedScores[ i ] > score ? expectedScores[ i ] - score : score - expectedScores[ i ] ) < tolerance );
        
        Explanation exp;
        searcher->explain( query, id, &exp );
        
        float_t sd = exp.getDetail( 0 )->getValue() - score;
        if ( sd < 0 ) sd *= -1;
        assertTrueMsg( _T( "explained score does not match" ), sd < tolerance );
    }

    _CLLDELETE( topdocs );
}
