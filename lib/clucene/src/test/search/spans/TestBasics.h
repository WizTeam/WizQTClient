/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TestBasics
#define _lucene_search_spans_TestBasics

#include "test.h"

/**
 * Tests basic search capabilities.
 *
 * <p>Uses a collection of 1000 documents, each the english rendition of their
 * document number.  For example, the document numbered 333 has text "three
 * hundred thirty three".
 *
 * <p>Tests are each a single query, and its hits are checked to ensure that
 * all and only the correct documents are returned, thus providing end-to-end
 * testing of the indexing and search code.
 *
 * @author Doug Cutting
 */
class TestBasics 
{
private:
    CL_NS(search)::IndexSearcher *  searcher;
    CL_NS(store)::RAMDirectory *   directory;

public:
    CuTest *                        tc;

public:
    TestBasics( CuTest* tc );
    virtual ~TestBasics();

    void setUp();

    void testTerm();
    void testTerm2();
    void testPhrase();
    void testPhrase2();
    void testBoolean();
    void testBoolean2();
    void testSpanNearExact();
    void testSpanNearUnordered();
    void testSpanNearOrdered();
    void testSpanNot();
    void testSpanWithMultipleNotSingle();
    void testSpanWithMultipleNotMany();
    void testNpeInSpanNearWithSpanNot();
    void testNpeInSpanNearInSpanFirstInSpanNot();
    void testSpanFirst();
    void testSpanOr();
    void testSpanExactNested(); 
    void testSpanNearOr();
    void testSpanComplex1();

private:
    void checkHits( Query * query, int32_t * results, size_t resultsCount );
};
#endif

