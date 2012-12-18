/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_TestExplanations
#define _lucene_search_TestExplanations

#include "test.h"

CL_CLASS_DEF2(search,spans,SpanQuery);
CL_CLASS_DEF2(search,spans,SpanOrQuery);
CL_CLASS_DEF2(search,spans,SpanTermQuery);
CL_CLASS_DEF2(search,spans,SpanNotQuery);
CL_CLASS_DEF2(search,spans,SpanNearQuery);
CL_CLASS_DEF2(search,spans,SpanFirstQuery);

CL_NS_USE2(search,spans);

/**
 * Tests primative queries (ie: that rewrite to themselves) to
 * insure they match the expected set of docs, and that the score of each
 * match is equal to the value of the scores explanation.
 *
 * <p>
 * The assumption is that if all of the "primative" queries work well,
 * then anythingthat rewrites to a primative will work well also.
 * </p>
 *
 * @see "Subclasses for actual tests"
 */
class TestExplanations 
{
public:
    /** A filter that only lets the specified document numbers pass */
    class ItemizedFilter : public CL_NS(search)::Filter
    {
    public:
        int32_t * docs;
        size_t    docsCount;

    public:
        ItemizedFilter( int32_t * docs, size_t docsCount );
        virtual ~ItemizedFilter();
        CL_NS(util)::BitSet * bits( CL_NS(index)::IndexReader * r );
    };

protected:
    CL_NS(search)::IndexSearcher *      searcher;
    CL_NS(store)::RAMDirectory *        directory;
    CL_NS(queryParser)::QueryParser *   qp;
    CL_NS(analysis)::Analyzer *         qpAnalyzer;

    static const TCHAR *                docFields[];
    static const TCHAR *                field;

    CuTest *                            tc;

public:
    TestExplanations( CuTest* tc );
    virtual ~TestExplanations();

    void setUp();
   
    Query * makeQuery( const TCHAR * queryText );

    /** check the expDocNrs first, then check the query (and the explanations) */
    virtual void qtest( const TCHAR * queryText, int32_t * expDocNrs, size_t expDocNrsCount );
  
    /** check the expDocNrs first, then check the query (and the explanations) */
    virtual void qtest( Query * q, int32_t * expDocNrs, size_t expDocNrsCount );

    /**
     * Tests a query using qtest after wrapping it with both optB and reqB
     * @see #qtest
     * @see #reqB
     * @see #optB
     */
    virtual void bqtest( Query * q, int32_t * expDocNrs, size_t expDocNrsCount );
    
    /**
     * Tests a query using qtest after wrapping it with both optB and reqB
     * @see #qtest
     * @see #reqB
     * @see #optB
     */
    virtual void bqtest( const TCHAR * queryText, int32_t * expDocNrs, size_t expDocNrsCount );

    /** helper for generating MultiPhraseQueries */
    static Term ** ta( const TCHAR ** s, size_t count );

    /** MACRO for SpanTermQuery */
    CL_NS2(search,spans)::SpanTermQuery * st( const TCHAR * s);

    /** MACRO for SpanNotQuery */
    SpanNotQuery * snot( SpanQuery * i, SpanQuery * e);

    /** MACRO for SpanOrQuery containing two SpanTerm queries */
    SpanOrQuery * sor( const TCHAR * s, const TCHAR * e );

    /** MACRO for SpanOrQuery containing two SpanQueries */
    SpanOrQuery * sor( SpanQuery * s, SpanQuery * e );

    /** MACRO for SpanOrQuery containing three SpanTerm queries */
    SpanOrQuery * sor( const TCHAR * s, const TCHAR * m, const TCHAR * e );

    /** MACRO for SpanOrQuery containing two SpanQueries */
    SpanOrQuery * sor( SpanQuery * s, SpanQuery * m, SpanQuery * e );

    /** MACRO for SpanNearQuery containing two SpanTerm queries */
    SpanNearQuery * snear( const TCHAR * s, const TCHAR * e, int32_t slop, bool inOrder );

    /** MACRO for SpanNearQuery containing two SpanQueries */
    SpanNearQuery * snear( SpanQuery * s, SpanQuery * e, int32_t slop, bool inOrder );
  
    /** MACRO for SpanNearQuery containing three SpanTerm queries */
    SpanNearQuery * snear( const TCHAR * s, const TCHAR * m, const TCHAR * e, int32_t slop, bool inOrder );
    
    /** MACRO for SpanNearQuery containing three SpanQueries */
    SpanNearQuery * snear( SpanQuery * s, SpanQuery * m, SpanQuery * e, int32_t slop, bool inOrder );

    /** MACRO for SpanFirst(SpanTermQuery) */
    SpanFirstQuery * sf( const TCHAR * s, int32_t b );

    /**
     * MACRO: Wraps a Query in a BooleanQuery so that it is optional, along
     * with a second prohibited clause which will never match anything
     */
    Query * optB( const TCHAR * q );

    /**
     * MACRO: Wraps a Query in a BooleanQuery so that it is optional, along
     * with a second prohibited clause which will never match anything
     */
    Query * optB( Query * q );

    /**
     * MACRO: Wraps a Query in a BooleanQuery so that it is required, along
     * with a second optional clause which will match everything
     */
    Query * reqB( const TCHAR * q );

    /**
     * MACRO: Wraps a Query in a BooleanQuery so that it is required, along
     * with a second optional clause which will match everything
     */
    Query * reqB( Query * q );
};
#endif

