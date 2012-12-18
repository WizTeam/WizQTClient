/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "QueryUtils.h"
#include "CLucene/search/Scorer.h"
#include "CheckHits.h"

/////////////////////////////////////////////////////////////////////////////
int32_t QueryUtils::skip_op = 0;
int32_t QueryUtils::next_op = 1;
float_t QueryUtils::maxDiff = 1e-5f;


/////////////////////////////////////////////////////////////////////////////
class WhackyQuery : public CL_NS(search)::Query
{
public:
    WhackyQuery() {};
    virtual ~WhackyQuery()  {}

    Query * clone() const
    { 
        return _CLNEW WhackyQuery(); 
    }

    static const char * getClassName()
    {
	    return "WhackyQuery";
    }

    const char * getObjectName() const 
    {
	    return getClassName();
    }
    
    TCHAR* toString(const TCHAR* field) const 
    { 
        return STRDUP_TtoT( _T( "My Whacky Query" )); 
    }
    
    bool equals(Query* other) const
    {
        if( this == other ) return true;
        if( other == NULL || !( other->instanceOf( WhackyQuery::getClassName() )))
	        return false;

        return true;
    }

    size_t hashCode() const 
    {
        size_t result = Similarity::floatToByte( getBoost() ) ^ 0x97AF937F;
        return result;
    }
};

/////////////////////////////////////////////////////////////////////////////
class QueryUtilsHitCollector1 : public CL_NS(search)::HitCollector
{
public:
    int32_t *   order;
    int32_t *   opidx;
    int32_t     orderLength;
    int32_t *   sdoc;
    Scorer *    scorer;
    Query *     q;
    CuTest *    tc;

public:
    void collect( const int32_t doc, const float_t score )
    {
        int32_t op = order[ (opidx[ 0 ]++ ) % orderLength ];
        bool more = ( op == QueryUtils::skip_op ) ? scorer->skipTo( sdoc[ 0 ] + 1 ) : scorer->next();
        sdoc[ 0 ] = scorer->doc();
        float_t scorerScore = scorer->score();
        float_t scorerScore2 = scorer->score();
        float_t scoreDiff = score > scorerScore ? score - scorerScore : scorerScore - score;
        float_t scorerDiff = scorerScore2 > scorerScore2 ? scorerScore2 - scorerScore : scorerScore - scorerScore2;
        if( ! more || doc != sdoc[ 0 ] || scoreDiff > QueryUtils::maxDiff || scorerDiff > QueryUtils::maxDiff )
        {
            StringBuffer buffer;
            buffer.append( _T( "ERROR matching docs:\n\t" ));
            
            buffer.append( doc != sdoc[ 0 ] ? _T( "--> doc=" ) : _T( "doc=" ));
            buffer.appendInt( sdoc[ 0 ] );

            buffer.append( ! more ? _T( "\n\t--> tscorer.more=" ) : _T( "\n\ttscorer.more=" ));
            buffer.appendBool( more );

            buffer.append( scoreDiff > QueryUtils::maxDiff ? _T( "\n\t--> scorerScore=" ) : _T( "\n\tscorerScore=" ));
            buffer.appendFloat( scorerScore, 2 );
            buffer.append( _T( " scoreDiff=" ));
            buffer.appendFloat( scoreDiff, 2 );
            buffer.append( _T( " maxDiff=" ));
            buffer.appendFloat( QueryUtils::maxDiff, 2 );

            buffer.append( scorerDiff > QueryUtils::maxDiff ? _T( "\n\t--> scorerScore2=" ) : _T( "\n\tscorerScore2=" ));
            buffer.appendFloat( scorerScore2, 2 );
            buffer.append( _T( " scorerDiff=" ));
            buffer.appendFloat( scorerDiff, 2 );

            buffer.append( _T( "\n\thitCollector.doc=" ));
            buffer.appendInt( doc );
            buffer.append( _T( " score=" ));
            buffer.appendFloat( score, 2 );

            buffer.append( _T( "\n\t Scorer=" ));
            TCHAR * tmp = scorer->toString();
            buffer.append( tmp );
            _CLDELETE_LARRAY( tmp );

            buffer.append( _T( "\n\t Query=" ));
            tmp = q->toString();
            buffer.append( tmp );
            _CLDELETE_ARRAY( tmp );

            buffer.append( _T( "\n\t Order=" ));
            for( int32_t i = 0; i < orderLength; i++) 
                buffer.append( order[ i ] == QueryUtils::skip_op ? _T( " skip()" ): _T( " next()" ));
            
            buffer.append( _T( "\n\t Op=" ));
            buffer.append( op == QueryUtils::skip_op ? _T( " skip()" ) : _T( " next()" ));

            assertTrueMsg( buffer.getBuffer(), false );
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
class QueryUtilsHitCollector2 : public CL_NS(search)::HitCollector
{
public:
    int32_t *           lastDoc;
    Query *             q;
    IndexSearcher *     s;
    CuTest *            tc;

public:
    void collect( const int32_t doc, const float_t score )
    {
        for( int32_t i = lastDoc[ 0 ] + 1; i <= doc; i++ )
        {
            Weight * w = q->weight( s );
            Scorer * scorer = w->scorer( s->getReader() );

            if( ! scorer->skipTo( i ) )
            {
                StringBuffer buffer;
                buffer.append( _T( "query collected " ));
                buffer.appendInt( doc );
                buffer.append( _T( " but skipTo(" ));
                buffer.appendInt( i );
                buffer.append( _T( ") says no more docs!" ));
                assertTrueMsg( buffer.getBuffer(), false );
            }

            if( doc != scorer->doc() )
            {
                StringBuffer buffer;
                buffer.append( _T( "query collected " ));
                buffer.appendInt( doc );
                buffer.append( _T( " but skipTo(" ));
                buffer.appendInt( i );
                buffer.append( _T( ") got to " ));
                buffer.appendInt( scorer->doc() );
                assertTrueMsg( buffer.getBuffer(), false );
            }

            float_t skipToScore = scorer->score();
            float_t sd = skipToScore - scorer->score();
            if( ( sd < 0 ? sd * -1 : sd ) > QueryUtils::maxDiff )
            {
                StringBuffer buffer;
                buffer.append( _T( "unstable skipTo(" ));
                buffer.appendInt( i );
                buffer.append( _T( ") score: " ));
                buffer.appendFloat( skipToScore, 2 );
                buffer.append( _T( "/") );
                buffer.appendFloat( QueryUtils::maxDiff, 2 );
                assertTrueMsg( buffer.getBuffer(), false );
            }

            if( ( skipToScore > score ? skipToScore - score : score - skipToScore ) > QueryUtils::maxDiff )
            {
                StringBuffer buffer;
                buffer.append( _T( "query assigned doc " ));
                buffer.appendInt( doc );
                buffer.append( _T( " a score of <" ));
                buffer.appendFloat( score, 2 );
                buffer.append( _T( "> but skipTo(" ));
                buffer.appendInt( i );
                buffer.append( _T( ") has <" ));
                buffer.appendFloat( skipToScore, 2 );
                buffer.append( _T( ">!" ));
                assertTrueMsg( buffer.getBuffer(), false );
            }

            _CLLDELETE( scorer );
            _CLLDELETE( w );
        }
        lastDoc[ 0 ] = doc;
    }
};

/////////////////////////////////////////////////////////////////////////////
void QueryUtils::check( CuTest* tc, Query * q )
{
    checkHashEquals( tc, q );
}

void QueryUtils::checkHashEquals( CuTest* tc, Query * q )
{
    Query * q2 = q->clone();
    checkEqual( tc, q, q2 );

    Query * q3 = q->clone();
    q3->setBoost( 7.21792348f );
    checkUnequal( tc,  q, q3 );

    // test that a class check is done so that no exception is thrown
    // in the implementation of equals()
    Query * whacky = _CLNEW WhackyQuery();
    whacky->setBoost( q->getBoost() );
    checkUnequal( tc, q, whacky );

    _CLLDELETE( q2 );
    _CLLDELETE( q3 );
    _CLLDELETE( whacky );
}

void QueryUtils::checkEqual( CuTest* tc, Query * q1, Query * q2 ) 
{
    assertTrue( q1->equals( q2 ));
    assertTrue( q2->equals( q1 ));
    assertTrue( q1->hashCode() == q2->hashCode() );
}

void QueryUtils::checkUnequal( CuTest* tc, Query * q1, Query * q2 )
{
    assertTrue( ! q1->equals( q2 ));
    assertTrue( ! q2->equals( q1 ));

    // possible this test can fail on a hash collision... if that
    // happens, please change test to use a different example.
    assertTrue( q1->hashCode() != q2->hashCode());
}
  
void QueryUtils::checkExplanations( CuTest* tc, Query * q, Searcher * s )
{
    CheckHits::checkExplanations( tc, q, NULL, s, true );
}
  
void QueryUtils::check( CuTest* tc, Query * q1, Searcher * s )
{
    check( tc, q1 );
    if( s )
    {
        if( s->getObjectName() == IndexSearcher::getClassName()) 
        {
            IndexSearcher * is = (IndexSearcher*) s;
            checkFirstSkipTo( tc, q1, is );
            checkSkipTo( tc, q1, is );
        }
     
        checkExplanations( tc, q1, s );
        checkSerialization( tc, q1, s );
    }
}

void QueryUtils::checkSerialization( CuTest* tc, Query * q, Searcher * s )
{
    Weight * w = q->weight( s );
    // TODO: Port this test
//     try {
//       ByteArrayOutputStream bos = new ByteArrayOutputStream();
//       ObjectOutputStream oos = new ObjectOutputStream(bos);
//       oos.writeObject(w.);
//       oos.close();
//       ObjectInputStream ois = new ObjectInputStream(new ByteArrayInputStream(bos.toByteArray()));
//       ois.readObject();
//       ois.close();
//       
//       //skip rquals() test for now - most weights don't overide equals() and we won't add this just for the tests.
//       //TestCase.assertEquals("writeObject(w) != w.  ("+w+")",w2,w);   
//       
//     } catch (Exception e) {
//       IOException e2 = new IOException("Serialization failed for "+w);
//       e2.initCause(e);
//       throw e2;
//     }
    _CLLDELETE( w );
}

void QueryUtils::checkSkipTo( CuTest* tc, Query * q, IndexSearcher * s )
{
    if( BooleanQuery::getAllowDocsOutOfOrder()) 
        return;  // in this case order of skipTo() might differ from that of next().

    int32_t order0[] = {next_op};
    int32_t order1[] = {skip_op};
    int32_t order2[] = {skip_op, next_op};
    int32_t order3[] = {next_op, skip_op};
    int32_t order4[] = {skip_op, skip_op, next_op, next_op};
    int32_t order5[] = {next_op, next_op, skip_op, skip_op};
    int32_t order6[] = {skip_op, skip_op, skip_op, next_op, next_op};
    int32_t   ordersLength[] = { 1, 1, 2, 2, 4, 4, 5 };
    int32_t * orders[] = { order0, order1, order2, order3, order4, order5, order6 };
    size_t ordersCount = 7;

    for( size_t k = 0; k < ordersCount; k++ )
    {
        int32_t * order = orders[ k ];
        int32_t opidx[] = { 0 };

        Weight * w = q->weight( s );
        Scorer * scorer = w->scorer( s->getReader() );
      
        // FUTURE: ensure scorer.doc()==-1

        int32_t * sdoc = _CL_NEWARRAY( int32_t, 1 );
        sdoc[ 0 ] = -1;

        QueryUtilsHitCollector1 hitCollector;
        hitCollector.order = order;
        hitCollector.opidx = opidx;
        hitCollector.orderLength = ordersLength[ k ];
        hitCollector.sdoc = sdoc;
        hitCollector.scorer = scorer;
        hitCollector.q = q;
        hitCollector.tc = tc;

        s->_search( q, NULL, &hitCollector );
      
        // make sure next call to scorer is false.
        int32_t op = order[ (opidx[ 0 ]++ ) % ordersLength[ k ] ];
        bool more = ( op == skip_op ) ? scorer->skipTo( sdoc[ 0 ] + 1 ) : scorer->next();
        assertTrue( ! more );

        _CLDELETE_LARRAY( sdoc );
        _CLLDELETE( scorer );
        _CLLDELETE( w );
    }
}
    
void QueryUtils::checkFirstSkipTo( CuTest* tc, Query * q, IndexSearcher * s )
{
    int32_t lastDoc[] = {-1};
    QueryUtilsHitCollector2 hitCollector;
    hitCollector.lastDoc = lastDoc;
    hitCollector.q = q;
    hitCollector.s = s;
    hitCollector.tc = tc;

    s->_search( q, NULL, &hitCollector );

    Weight * w = q->weight( s );
    Scorer * scorer = w->scorer( s->getReader() );
    bool more = scorer->skipTo( lastDoc[ 0 ] + 1 );
    if( more )
    {
        StringBuffer buffer;
        buffer.append( _T( "query's last doc was " ));
        buffer.appendInt( lastDoc[ 0 ] );
        buffer.append( _T( " but skipTo(" ));
        buffer.appendInt( lastDoc[ 0 ] + 1 );
        buffer.append( _T( ") got to " ));
        buffer.appendInt( scorer->doc() );
        assertTrueMsg( buffer.getBuffer(), false );
    }

    _CLLDELETE( scorer );
    _CLLDELETE( w );
}
