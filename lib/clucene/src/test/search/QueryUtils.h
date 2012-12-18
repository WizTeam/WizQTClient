/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_QueryUtils
#define _lucene_search_QueryUtils

#include "test.h"

class QueryUtils 
{
public:
    static int32_t skip_op;
    static int32_t next_op;
    static float_t maxDiff;

public:
    /** Check the types of things query objects should be able to do. */
    static void check( CuTest* tc, Query * q );

    /** check very basic hashCode and equals */
    static void checkHashEquals( CuTest* tc, Query * q );
    static void checkEqual( CuTest* tc, Query * q1, Query * q2 );
    static void checkUnequal( CuTest* tc, Query * q1, Query * q2);
  
    /** deep check that explanations of a query 'score' correctly */
    static void checkExplanations( CuTest* tc, Query * q, Searcher * s );
  
    /** 
     * various query sanity checks on a searcher, including explanation checks.
     * @see #checkExplanations
     * @see #checkSkipTo
     * @see #check(Query)
     */
    static void check( CuTest* tc, Query * q1, Searcher * s );

    /** alternate scorer skipTo(),skipTo(),next(),next(),skipTo(),skipTo(), etc
     * and ensure a hitcollector receives same docs and scores
     */
    static void checkSkipTo( CuTest* tc, Query * q, IndexSearcher * s );

private:
    /** check that the query weight is serializable. 
     * @throws IOException if serialization check fail. 
     */
    static void checkSerialization( CuTest* tc, Query * q, Searcher * s );
    
    // check that first skip on just created scorers always goes to the right doc
    static void checkFirstSkipTo( CuTest* tc, Query * q, IndexSearcher * s );
};
#endif

