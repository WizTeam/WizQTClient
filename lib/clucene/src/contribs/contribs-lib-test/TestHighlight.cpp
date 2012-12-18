/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

#include "CLucene/store/RAMDirectory.h"
#include "CLucene/highlighter/QueryTermExtractor.h"
#include "CLucene/highlighter/QueryScorer.h"
#include "CLucene/highlighter/Highlighter.h"
#include "CLucene/highlighter/TokenGroup.h"
#include "CLucene/highlighter/SimpleHTMLFormatter.h"
#include "CLucene/highlighter/SimpleFragmenter.h"

CL_NS_USE2(search, highlight);

RAMDirectory hl_ramDir;
StandardAnalyzer hl_analyzer;


const TCHAR* hl_FIELD_NAME = _T("contents");
Query* hl_originalquery = NULL;
Query* hl_query = NULL;
Query* hl_rewrittenquery = NULL;
IndexReader* hl_reader = NULL;
Searcher* hl_searcher = NULL;
Hits* hl_hits = NULL;

class hl_formatterCls : public Formatter {
public:
    int numHighlights;

    hl_formatterCls() {
        numHighlights = 0;
    }

    ~hl_formatterCls() {
    }

    TCHAR* highlightTerm(const TCHAR* originalText, const TokenGroup* group) {
        if (group->getTotalScore() <= 0) {
            return STRDUP_TtoT(originalText);
        }
        numHighlights++; //update stats used in assertions

        int len = _tcslen(originalText) + 7;
        TCHAR* ret = _CL_NEWARRAY(TCHAR, len + 1);
        _tcscpy(ret, _T("<b>"));
        _tcscat(ret, originalText);
        _tcscat(ret, _T("</b>"));

        return ret;
    }
};
hl_formatterCls hl_formatter;


const TCHAR* hl_texts[6] ={
    _T("Hello this is a piece of text that is very long and contains too much preamble and the meat is really here which says kennedy has been shot"),
    _T("This piece of text refers to Kennedy at the beginning then has a longer piece of text that is very long in the middle and finally ends with another reference to Kennedy"),
    _T("JFK has been shot"),
    _T("John Kennedy has been shot"),
    _T("This text has a typo in referring to Keneddy"),
    NULL
};

void doStandardHighlights(CuTest* tc) {
    QueryScorer scorer(hl_query);
    Highlighter highlighter(&hl_formatter, &scorer);
    SimpleFragmenter frag(20);
    highlighter.setTextFragmenter(&frag);

    for (int i = 0; i < hl_hits->length(); i++) {
        const TCHAR* text = hl_hits->doc(i).get(hl_FIELD_NAME);
        int maxNumFragmentsRequired = 2;
        const TCHAR* fragmentSeparator = _T("...");
        StringReader reader(text);
        TokenStream* tokenStream = hl_analyzer.tokenStream(hl_FIELD_NAME, &reader);

        TCHAR* result =
                highlighter.getBestFragments(
                tokenStream,
                text,
                maxNumFragmentsRequired,
                fragmentSeparator);

        CuMessage(tc, _T("%s\n"), result == NULL ? _T("") : result);
        _CLDELETE_CARRAY(result);
        _CLDELETE(tokenStream);
    }
}

void doSearching(CuTest* tc, const TCHAR* queryString) {
    if (hl_searcher == NULL)
        hl_searcher = _CLNEW IndexSearcher(&hl_ramDir);

    if (hl_rewrittenquery != NULL && hl_originalquery != NULL) {
        if (hl_originalquery != hl_rewrittenquery)
            _CLDELETE(hl_rewrittenquery);
        _CLDELETE(hl_originalquery);
    }
    hl_originalquery = QueryParser::parse(queryString, hl_FIELD_NAME, &hl_analyzer);

    //for any multi-term queries to work (prefix, wildcard, range,fuzzy etc) you must use a rewritten query!
    hl_rewrittenquery = hl_originalquery->rewrite(hl_reader);
    hl_query = hl_rewrittenquery;

    TCHAR* s = hl_originalquery->toString(hl_FIELD_NAME);
    CuMessage(tc, _T("Searching for: %s\n"), s == NULL ? _T("") : s);
    _CLDELETE_CARRAY(s);

    s = hl_rewrittenquery->toString(hl_FIELD_NAME);
    CuMessage(tc, _T("Rewritten query: %s\n"), s == NULL ? _T("") : s);
    _CLDELETE_CARRAY(s);

    if (hl_hits != NULL)
        _CLDELETE(hl_hits);
    hl_hits = hl_searcher->search(hl_query);
    hl_formatter.numHighlights = 0;
}

void testSimpleHighlighter(CuTest *tc) {
    doSearching(tc, _T("Kennedy"));
    QueryScorer scorer(hl_query);
    Highlighter highlighter(&scorer);
    SimpleFragmenter fragmenter(40);

    highlighter.setTextFragmenter(&fragmenter);
    int maxNumFragmentsRequired = 2;
    for (int i = 0; i < hl_hits->length(); i++) {
        const TCHAR* text = hl_hits->doc(i).get(hl_FIELD_NAME);
        StringReader reader(text);
        TokenStream* tokenStream = hl_analyzer.tokenStream(hl_FIELD_NAME, &reader);

        TCHAR* result = highlighter.getBestFragments(tokenStream, text, maxNumFragmentsRequired, _T("..."));
        CuMessage(tc, _T("%s\n"), result == NULL ? _T("") : result);
        _CLDELETE_CARRAY(result);
        _CLDELETE(tokenStream);
    }
    //Not sure we can assert anything here - just running to check we dont throw any exceptions
}

void testGetFuzzyFragments(CuTest *tc) {
    doSearching(tc, _T("Kinnedy~"));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 5);
}

void testGetWildCardFragments(CuTest *tc) {
    doSearching(tc, _T("K?nnedy"));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 4);
}

void testGetBestFragmentsSimpleQuery(CuTest *tc) {
    doSearching(tc, _T("Kennedy"));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 4);
}

void testGetMidWildCardFragments(CuTest *tc) {
    doSearching(tc, _T("K*dy"));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 5);
}

void testGetBestFragmentsPhrase(CuTest *tc) {
    doSearching(tc, _T("\"John Kennedy\""));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 2);
}

void testGetBestFragmentsMultiTerm(CuTest *tc) {
    doSearching(tc, _T("John Ken*"));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 6);
}

void testGetBestFragmentsWithOr(CuTest *tc) {
    doSearching(tc, _T("JFK OR Kennedy"));
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 5);
}

void testGetRangeFragments(CuTest *tc) {
    TCHAR qry[200];
    _sntprintf(qry, 200, _T("%s:[Kannedy TO Kznnedy]"), hl_FIELD_NAME); //bug?needs lower case

    doSearching(tc, qry);
    doStandardHighlights(tc);

    TCHAR msg[1024];
    _sntprintf(msg, 1024, _T("Failed to find correct number of highlights %d found"), hl_formatter.numHighlights);
    CuAssert(tc, msg, hl_formatter.numHighlights == 5);
}

void setupHighlighter(CuTest *tc) {
    IndexWriter writer(&hl_ramDir, &hl_analyzer, true);
    for (int i = 0; hl_texts[i] != NULL; i++) {
        Document d;
        d.add(*_CLNEW Field(hl_FIELD_NAME, hl_texts[i], Field::STORE_YES | Field::INDEX_TOKENIZED));
        writer.addDocument(&d);
    }

    writer.optimize();
    writer.close();

    hl_reader = IndexReader::open(&hl_ramDir);
}

void cleanupHighlighter(CuTest *tc) {

    if (hl_originalquery != hl_rewrittenquery)
        _CLDELETE(hl_rewrittenquery);
    _CLDELETE(hl_originalquery);
    _CLDELETE(hl_hits);

    if (hl_reader != NULL) {
        hl_reader->close();
        _CLDELETE(hl_reader);
    }
    hl_ramDir.close();

    if (hl_searcher != NULL)
        _CLDELETE(hl_searcher);
}

CuSuite *testhighlighter(void) {
    CuSuite *suite = CuSuiteNew(_T("CLucene Highlight Test"));

    SUITE_ADD_TEST(suite, setupHighlighter);

    SUITE_ADD_TEST(suite, testSimpleHighlighter);
    SUITE_ADD_TEST(suite, testGetBestFragmentsSimpleQuery);
    SUITE_ADD_TEST(suite, testGetFuzzyFragments);
    SUITE_ADD_TEST(suite, testGetWildCardFragments);
    SUITE_ADD_TEST(suite, testGetMidWildCardFragments);
    SUITE_ADD_TEST(suite, testGetRangeFragments);
    SUITE_ADD_TEST(suite, testGetBestFragmentsPhrase);
    SUITE_ADD_TEST(suite, testGetBestFragmentsMultiTerm);
    SUITE_ADD_TEST(suite, testGetBestFragmentsWithOr);


    SUITE_ADD_TEST(suite, cleanupHighlighter);
    return suite;
}
