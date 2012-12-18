/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CheckHits.h"
#include "QueryUtils.h"


/////////////////////////////////////////////////////////////////////////////
float_t CheckHits::EXPLAIN_SCORE_TOLERANCE_DELTA = 0.00005f;


/////////////////////////////////////////////////////////////////////////////
/**
* Asserts that the score explanation for every document matching a
* query corresponds with the true score.
*
* NOTE: this HitCollector should only be used with the Query and Searcher
* specified at when it is constructed.
*
* @see CheckHits#verifyExplanation
*/
class ExplanationAsserter : public HitCollector 
{
private:
    Query *         q;
    Searcher *      s;
    TCHAR *         d;
    bool            deep;
    CuTest *        tc;

public:
    /** Constructs an instance which does shallow tests on the Explanation */
    ExplanationAsserter( Query * q, const TCHAR * defaultFieldName, Searcher * s, CuTest * tc )
    {
        this->q = q;
        this->s = s;
        this->d = q->toString( defaultFieldName );
        this->deep = false;
        this->tc = tc;
    }

    ExplanationAsserter( Query * q, const TCHAR * defaultFieldName, Searcher * s, bool deep, CuTest * tc )
    {
        this->q = q;
        this->s = s;
        this->d = q->toString( defaultFieldName );
        this->deep = deep;
        this->tc = tc;
    }      

    virtual ~ExplanationAsserter()
    {
        _CLDELETE_LARRAY( d );
    }

    void collect( const int32_t doc, const float_t score )
    {
        Explanation exp;
        s->explain( q, doc, &exp );
  
        if( exp.getDetailsLength() == 0 )                                                   // ToDo: Fix IndexSearcher::explain() method
        {
            StringBuffer buffer;
            buffer.append( _T( "Explanation of [[" ));
            buffer.append( d );
            buffer.append( _T( "]] for #" ));
            buffer.appendInt( doc );
            buffer.append( _T( " is null" ));
            assertTrueMsg( buffer.getBuffer(), false );
        }

        assertTrue( exp.getDetailsLength() == 1 );
        CheckHits::verifyExplanation( tc, d, doc, score, deep, exp.getDetail( 0 ) );        // ToDo: Fix IndexSearcher::explain() method
    }
};

/////////////////////////////////////////////////////////////////////////////
/**
* an IndexSearcher that implicitly checks hte explanation of every match
* whenever it executes a search.
*
* @see ExplanationAsserter
*/
class ExplanationAssertingSearcher : public CL_NS(search)::IndexSearcher
{
private:
    CuTest * tc;
public:
    ExplanationAssertingSearcher( Directory * d, CuTest * tc ) : IndexSearcher( d ) { this->tc = tc; }
    ExplanationAssertingSearcher( IndexReader * r, CuTest * tc ) : IndexSearcher( r ) { this->tc = tc; }
    virtual ~ExplanationAssertingSearcher() {}
    
    Hits * search( Query * query, Filter * filter )              
        { checkExplanations( query ); return IndexSearcher::search( query, filter ); }

    Hits * search( Query * query, Sort * sort )                  
        { checkExplanations( query ); return IndexSearcher::search( query, sort ); }

    Hits * search( Query * query, Filter * filter, Sort * sort ) 
        { checkExplanations( query );  return IndexSearcher::search( query, filter, sort ); }

    TopFieldDocs * _search( Query * query, Filter * filter, int32_t n, Sort * sort )
        { checkExplanations( query ); return IndexSearcher::_search( query, filter, n, sort ); }

    void _search( Query * query, HitCollector * results )
        { checkExplanations( query ); IndexSearcher::_search( query, NULL, results ); }

    void _search( Query * query, Filter * filter, HitCollector * results )
        { checkExplanations( query ); IndexSearcher::_search( query, filter, results ); }

    TopDocs * search_( Query * query, Filter * filter, int32_t n )
        { checkExplanations( query ); return IndexSearcher::_search( query, filter, n ); }

protected:
    void checkExplanations( Query * q ) 
        { IndexSearcher::_search( q, NULL, _CLNEW ExplanationAsserter( q, NULL, this, tc )); }
};
    
/////////////////////////////////////////////////////////////////////////////
class HitSetCollector : public HitCollector
{
public:
    set<int32_t>        actual;

public:
    HitSetCollector() : HitCollector() {};
    virtual ~HitSetCollector() {};

    virtual void collect( const int32_t doc, const float_t score ) { actual.insert( doc ); }
};


/////////////////////////////////////////////////////////////////////////////
bool CheckHits::setEquals( set<int32_t>& s1, set<int32_t>& s2 )
{
    bool bEquals = ( s1.size() == s2.size() );
    
    set<int32_t>::iterator iS1 = s1.begin();
    set<int32_t>::iterator iS2 = s2.begin();
    for( ; bEquals && ( iS1 != s1.end() ); iS1++, iS2++ )
        bEquals = ( *iS1 == *iS2 );

    return bEquals;
}

bool CheckHits::stringEndsWith( const TCHAR* tszStr, const TCHAR * tszEnd )
{
    if( ! tszStr || ! tszEnd )
        return false;

    size_t lenStr = _tcslen( tszStr );
    size_t lenEnd = _tcslen( tszEnd );
    if( lenEnd > lenStr )
        return false;

    const TCHAR * tszReadEnd = tszEnd + lenEnd;
    const TCHAR * tszReadStr = tszStr + lenStr;
    while( tszEnd != tszReadEnd )
    {
        if( *tszReadEnd-- != *tszReadStr-- )
            return false;
    }

    return true;
}

void CheckHits::checkNoMatchExplanations( CuTest* tc, Query * q, const TCHAR * defaultFieldName, Searcher * searcher, int32_t * results, size_t resultsCount )
{
    TCHAR * d = q->toString( defaultFieldName );
    
    set<int32_t> ignore;
    for( size_t i = 0; i < resultsCount; i++ )
        ignore.insert( results[ i ] );

    int32_t maxDoc = searcher->maxDoc();
    for( int32_t doc = 0; doc < maxDoc; doc++ )
    {
        if( ignore.find( doc ) != ignore.end() )
            continue;

        Explanation exp;
        searcher->explain( q, doc, &exp );
        
        if( 0.0f != exp.getValue() )
        {
            StringBuffer buffer;
            TCHAR * tszExp = exp.toString();

            buffer.append( _T( "Explanation of [[" ));
            buffer.append( d );
            buffer.append( _T( "]] for #" ));
            buffer.appendInt( doc );
            buffer.append( _T( " doesn't indicate non-match: " ));

            buffer.append( tszExp );
            _CLDELETE_LARRAY( tszExp );

            assertTrueMsg( buffer.getBuffer(), false );
        }
    }

    _CLDELETE_LARRAY( d );
}

void CheckHits::checkHitCollector( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher, int32_t * results, size_t resultsCount )
{
    set<int32_t> correct;
    for( size_t i = 0; i < resultsCount; i++ )
        correct.insert( results[ i ] );
    
    HitSetCollector hitSet;
    searcher->_search( query, &hitSet );

    if( ! setEquals( correct, hitSet.actual ))
    {
        TCHAR * tszQry = query->toString( defaultFieldName );
        assertTrueMsg( tszQry, false );
        _CLDELETE_LARRAY( tszQry );
    }
    
    QueryUtils::check( tc, query, searcher );
}
  
/////////////////////////////////////////////////////////////////////////////
void CheckHits::checkHits( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher, int32_t * results, size_t resultsCount )
{
    if( searcher->getObjectName() == IndexSearcher::getClassName() )
        QueryUtils::check( tc, query, (IndexSearcher *) searcher );

//     Hits * hits = searcher->search( query );
// 
//     set<int32_t> correct;
//     for( size_t i = 0; i < resultsCount; i++ )
//         correct.insert( results[ i ] );
// 
//     set<int32_t> actual;
//     for( size_t i = 0; i < hits->length(); i++ )
//         actual.insert( hits->id( i ));
// 
//     _CLLDELETE( hits );
// 
//     if( ! setEquals( correct, actual ))
//     {
//         TCHAR * tszQry = query->toString( defaultFieldName );
//         assertTrueMsg( tszQry, false );
//         _CLDELETE_LARRAY( tszQry );
//     }
//     
//     QueryUtils::check( tc, query, searcher );
}

void CheckHits::checkDocIds( CuTest* tc, const TCHAR * mes, int32_t * results, size_t resultsCount, Hits * hits )
{
    StringBuffer buffer;

    if( resultsCount != hits->length() )
    {
        buffer.append( mes );
        buffer.append( _T( " nr of hits" ));
        assertTrueMsg( buffer.getBuffer(), false );
    }

    for( size_t i = 0; i < resultsCount; i++ )
    {
        if( results[ i ] != hits->id( i ))
        {
            buffer.clear();
            buffer.append( mes );
            buffer.append( _T( " doc nrs for hit " ));
            buffer.appendInt( i );
            assertTrueMsg( buffer.getBuffer(), false );
        }
    }
}

void CheckHits::checkHitsQuery( CuTest* tc, Query * query, Hits * hits1, Hits * hits2, int32_t * results, size_t resultsCount )
{
    checkDocIds( tc, _T( "hits1" ), results, resultsCount, hits1 );
    checkDocIds( tc, _T( "hits2" ), results, resultsCount, hits2 );
    checkEqual( tc, query, hits1, hits2 );
}

void CheckHits::checkEqual( CuTest* tc, Query * query, Hits * hits1, Hits * hits2 )
{
    float_t scoreTolerance = 1.0e-6f;
    assertTrueMsg( _T( "Unequal lengths of supplied hits." ), hits1->length() == hits2->length() );

    for( size_t i = 0; i < hits1->length(); i++ )
    {
        if( hits1->id( i ) != hits2->id( i ))
        {
            StringBuffer buffer;
            buffer.append( _T( "Hit " ));
            buffer.appendInt( i );
            buffer.append( _T( " docnumbers don't match\n" ));
            appendHits( buffer, hits1, hits2, 0, 0 );
            buffer.append( _T( "for query:" ));
            TCHAR * tszQry = query->toString();
            buffer.append( tszQry );
            _CLDELETE_LARRAY( tszQry );

            assertTrueMsg( buffer.getBuffer(), false );
        }

        float_t sd = hits1->score( i ) -  hits2->score( i );
        if ( sd < 0 ) sd *= -1;
        
        if(( hits1->id( i ) != hits2->id( i ))
            || sd > scoreTolerance )
        {
            StringBuffer buffer;
            buffer.append( _T( "Hit " ));
            buffer.appendInt( i );
            buffer.append( _T( ", doc nrs " ));
            buffer.appendInt( hits1->id( i ));
            buffer.append( _T( " and "  ));
            buffer.appendInt( hits2->id( i ));
            buffer.append( _T( "\nunequal       : " ));
            buffer.appendFloat( hits1->score( i ), 2 );
            buffer.append( _T( "\n           and: " ));
            buffer.appendFloat( hits2->score( i ), 2 );
            buffer.append( _T( "\nfor query:" ));
            TCHAR *tszQry = query->toString();
            buffer.append( tszQry );
            _CLDELETE_LARRAY( tszQry );

            assertTrueMsg( buffer.getBuffer(), false );
        }
    }
}

void CheckHits::appendHits( StringBuffer& buffer, Hits * hits1, Hits * hits2, size_t start, size_t end )
{
    size_t len1 = hits1 ? hits1->length() : 0;
    size_t len2 = hits2 ? hits2->length() : 0;
    if( end <= 0 )
        end = len1 < len2 ? len2 : len1;    // max

    buffer.append( _T( "Hits length1=" ));
    buffer.appendInt( len1 );
    buffer.append( _T( "\tlength2=" ));
    buffer.appendInt( len2 );

    buffer.append( _T( "\n" ));
    for( size_t i = start; i < end; i++ )
    {
        buffer.append( _T( "hit=" ));
        buffer.appendInt( i );
        buffer.append( _T( ":" ));
        if( i < len1 )
        {
            buffer.append( _T( " doc" ));
            buffer.appendInt( hits1->id( i ) );
            buffer.append( _T( "=" ));
            buffer.appendFloat( hits1->score( i ), 2);
        } 
        else 
        {
            buffer.append( _T( "               " ));
        }

        buffer.append( _T( ",\t" ));
        if( i < len2 )
        {
            buffer.append( _T( " doc" ));
            buffer.appendInt( hits2->id( i ));
            buffer.append( _T( "= "));
            buffer.appendFloat( hits2->score( i ), 2 );
        }
        
        buffer.append( _T( "\n" ));
    }
}

void CheckHits::appendTopdocs( StringBuffer& buffer, TopDocs * docs, size_t start, size_t end )
{
    buffer.append( _T( "TopDocs totalHits=" ));
    buffer.appendInt( docs->totalHits );
    buffer.append( _T( " top=" ));
    buffer.appendInt( docs->scoreDocsLength );
    buffer.append( _T( "\n" ));
    if( end <= 0 )
        end = docs->scoreDocsLength;
    else 
        end = end < (size_t)docs->scoreDocsLength ? end : (size_t)docs->scoreDocsLength; // min

    for( size_t i = start; i < end; i++ )
    {
        buffer.append( _T( "\t" ));
        buffer.appendInt( i );
        buffer.append( _T( ") doc=" ));
        buffer.appendInt( docs->scoreDocs[ i ].doc );
        buffer.append( _T( "\tscore=" ));
        buffer.appendFloat( docs->scoreDocs[ i ].score, 2 );
        buffer.append( _T( "\n" ));
    }
}

void CheckHits::checkExplanations( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher )
{
    checkExplanations( tc, query, defaultFieldName, searcher, false );
}

void CheckHits::checkExplanations( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher, bool deep )
{
    ExplanationAsserter asserter( query, defaultFieldName, searcher, deep, tc );
    searcher->_search( query, &asserter );
}


void CheckHits::verifyExplanation( CuTest* tc, const TCHAR * q, int32_t doc, float_t score, bool deep, Explanation * expl )
{
    StringBuffer buffer;
    TCHAR * tmp;
    
    float_t value = expl->getValue();

    if( ( score > value ? score - value : value - score ) > EXPLAIN_SCORE_TOLERANCE_DELTA )
    {
        buffer.append( q );
        buffer.append( _T( ": score(doc=" ));
        buffer.appendInt( doc );
        buffer.append( _T( ")=" ));
        buffer.appendFloat( score, 2 );
        buffer.append( _T( " != explanationScore=" ));
        buffer.appendFloat( value, 2 );
        buffer.append( _T( " Explanation: " ));
        tmp = expl->toString();
        buffer.append( tmp );
        _CLDELETE_LARRAY( tmp );

        assertTrueMsg( buffer.getBuffer(), false );
    }

    if( ! deep )
        return;

    Explanation ** detail = _CL_NEWARRAY( Explanation *, expl->getDetailsLength() + 1 );
    expl->getDetails( detail );
    if( expl->getDetailsLength() > 0 )
    {
        if( expl->getDetailsLength() == 1 )
        {
            // simple containment, no matter what the description says, 
            // just verify contained expl has same score
            verifyExplanation( tc, q, doc, score, deep, detail[ 0 ] );
        } 
        else 
        {
            // explanation must either:
            // - end with one of: "product of:", "sum of:", "max of:", or
            // - have "max plus <x> times others" (where <x> is float).
            float_t x = 0;
            const TCHAR* descr = expl->getDescription();
            TCHAR* descrLwr = STRDUP_TtoT( descr );
            _tcslwr( descrLwr );
            
            bool productOf = stringEndsWith( descr, _T( "product of:" ));
            bool sumOf = stringEndsWith( descr, _T( "sum of:" ));
            bool maxOf = stringEndsWith( descr, _T( "max of:" ));
            bool maxTimesOthers = false;
            if( ! ( productOf || sumOf || maxOf ))
            {
                // maybe 'max plus x times others'
                const TCHAR * k1 = _tcsstr( descr, _T( "max plus " ));
                if( k1 )
                {
                    k1 += 9; // "max plus ".length();
                    const TCHAR * k2 = _tcsstr( k1, _T( " " ));

                     x = _tcstod( k1, NULL );   // Float::parseFloat( descr.substring(k1,k2).trim());

                    if( k2 && 0 == _tcscmp( k2, _T( " times others of:" ))) 
                        maxTimesOthers = true;
                }
            }

            if( ! ( productOf || sumOf || maxOf || maxTimesOthers ))
            {
                buffer.clear();
                buffer.append( q );
                buffer.append( _T( ": multi valued explanation description=\"" ));
                buffer.append( descrLwr );
                buffer.append( _T( "\" must be 'max of plus x times others' or end with 'product of' or 'sum of:' or 'max of:' - " ));
                tmp = expl->toString();
                buffer.append( tmp );
                _CLDELETE_LARRAY( tmp );

                assertTrueMsg( buffer.getBuffer(), false );
            }

            float_t sum = 0;
            float_t product = 1;
            float_t max = 0;
            for( size_t i = 0; i < expl->getDetailsLength(); i++ )
            {
                float_t dval = detail[ i ]->getValue();
                verifyExplanation( tc, q, doc, dval, deep, detail[ i ] );
                product *= dval;
                sum += dval;
                max = max > dval ? max : dval;      // max
            }
            float_t combined = 0;

            if( productOf )
                combined = product;
            else if( sumOf )
                combined = sum;
            else if( maxOf )
                combined = max;
            else if( maxTimesOthers )
                combined = max + x * (sum - max);
            else
            {
                assertTrueMsg( _T( "should never get here!" ), false );
            }

            if( ( combined > value ? combined - value : value - combined ) > EXPLAIN_SCORE_TOLERANCE_DELTA )
            {
                buffer.clear();
                buffer.append( q );
                buffer.append( _T( ": actual subDetails combined==" ));
                buffer.appendFloat( combined, 2 );
                buffer.append( _T( " != value=" ));
                buffer.appendFloat( value, 2 );
                buffer.append( _T( " Explanation: " ));
                tmp = expl->toString();
                buffer.append( tmp );
                _CLDELETE_LARRAY( tmp );

                assertTrueMsg( buffer.getBuffer(), false );
            }

            _CLDELETE_LARRAY( descrLwr );
        }
    }

    _CLDELETE_ARRAY_ALL( detail );
}
