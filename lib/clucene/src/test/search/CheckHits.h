/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_CheckHits
#define _lucene_search_CheckHits

#include "test.h"

class CheckHits 
{
public:
    /**
     * Some explains methods calculate their values though a slightly
     * different  order of operations from the actaul scoring method ...
     * this allows for a small amount of variation
     */
    static float_t EXPLAIN_SCORE_TOLERANCE_DELTA;
    
public:
    /**
     * Tests that all documents up to maxDoc which are *not* in the
     * expected result set, have an explanation which indicates no match
     * (ie: Explanation value of 0.0f)
     */
    static void checkNoMatchExplanations( CuTest* tc, Query * q, const TCHAR * defaultFieldName, Searcher * searcher, int32_t * results, size_t resultsCount );

    /**
     * Tests that a query matches the an expected set of documents using a
     * HitCollector.
     *
     * <p>
     * Note that when using the HitCollector API, documents will be collected
     * if they "match" regardless of what their score is.
     * </p>
     * @param query the query to test
     * @param searcher the searcher to test the query against
     * @param defaultFieldName used for displaing the query in assertion messages
     * @param results a list of documentIds that must match the query
     * @see Searcher#search(Query,HitCollector)
     * @see #checkHits
     */
    static void checkHitCollector( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher, int32_t * results, size_t resultsCount );
  
    /**
     * Tests that a query matches the an expected set of documents using Hits.
     *
     * <p>
     * Note that when using the Hits API, documents will only be returned
     * if they have a positive normalized score.
     * </p>
     * @param query the query to test
     * @param searcher the searcher to test the query against
     * @param defaultFieldName used for displaing the query in assertion messages
     * @param results a list of documentIds that must match the query
     * @see Searcher#search(Query)
     * @see #checkHitCollector
     */
    static void checkHits( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher, int32_t * results, size_t resultsCount );

    /** Tests that a Hits has an expected order of documents */
    static void checkDocIds( CuTest* tc, const TCHAR * mes, int32_t * results, size_t resultsCount, Hits * hits );

    /** Tests that two queries have an expected order of documents,
     * and that the two queries have the same score values.
     */
    static void checkHitsQuery( CuTest* tc, Query * query, Hits * hits1, Hits * hits2, int32_t * results, size_t resultsCount );

    static void checkEqual( CuTest* tc, Query * query, Hits * hits1, Hits * hits2 );
    static void appendHits( StringBuffer& buffer, Hits * hits1, Hits * hits2, size_t start, size_t end );
    static void appendTopdocs( StringBuffer& buffer, TopDocs * docs, size_t start, size_t end );

    /**
     * Asserts that the explanation value for every document matching a
     * query corresponds with the true score. 
     *
     * @see ExplanationAsserter
     * @see #checkExplanations(Query, String, Searcher, boolean) for a
     * "deep" testing of the explanation details.
     *   
     * @param query the query to test
     * @param searcher the searcher to test the query against
     * @param defaultFieldName used for displaing the query in assertion messages
     */
    static void checkExplanations( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher );

    /**
     * Asserts that the explanation value for every document matching a
     * query corresponds with the true score.  Optionally does "deep" 
     * testing of the explanation details.
     *
     * @see ExplanationAsserter
     * @param query the query to test
     * @param searcher the searcher to test the query against
     * @param defaultFieldName used for displayng the query in assertion messages
     * @param deep indicates whether a deep comparison of sub-Explanation details should be executed
     */
    static void checkExplanations( CuTest* tc, Query * query, const TCHAR * defaultFieldName, Searcher * searcher, bool deep );

    /** 
     * Assert that an explanation has the expected score, and optionally that its
     * sub-details max/sum/factor match to that score.
     *
     * @param q String representation of the query for assertion messages
     * @param doc Document ID for assertion messages
     * @param score Real score value of doc with query q
     * @param deep indicates whether a deep comparison of sub-Explanation details should be executed
     * @param expl The Explanation to match against score
     */
    static void verifyExplanation( CuTest* tc, const TCHAR * q, int32_t doc, float_t score, bool deep, Explanation * expl );

    static bool setEquals( set<int32_t>& s1, set<int32_t>& s2 );
    static bool stringEndsWith( const TCHAR* tszStr, const TCHAR * tszEnd );

};

#endif

