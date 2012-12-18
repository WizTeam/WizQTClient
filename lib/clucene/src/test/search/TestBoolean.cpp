/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/search/_BooleanScorer2.h"
#include "CLucene/search/Similarity.h"
#include "MockScorer.h"
#include "MockHitCollector.h"

/// TestBooleanQuery.java, ported 5/9/2009
void testEquality(CuTest *tc) {
    BooleanQuery* bq1 = _CLNEW BooleanQuery();
    Term* t = _CLNEW Term(_T("field"), _T("value1"));
    bq1->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    t = _CLNEW Term(_T("field"), _T("value2"));
    bq1->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    BooleanQuery* nested1 = _CLNEW BooleanQuery();
    t = _CLNEW Term(_T("field"), _T("nestedvalue1"));
    nested1->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    t = _CLNEW Term(_T("field"), _T("nestedvalue2"));
    nested1->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    bq1->add(nested1, true, BooleanClause::SHOULD);

    BooleanQuery* bq2 = _CLNEW BooleanQuery();
    t = _CLNEW Term(_T("field"), _T("value1"));
    bq2->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    t = _CLNEW Term(_T("field"), _T("value2"));
    bq2->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    BooleanQuery* nested2 = _CLNEW BooleanQuery();
    t = _CLNEW Term(_T("field"), _T("nestedvalue1"));
    nested2->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    t = _CLNEW Term(_T("field"), _T("nestedvalue2"));
    nested2->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
    _CLDECDELETE(t);
    bq2->add(nested2, true, BooleanClause::SHOULD);

    CLUCENE_ASSERT(bq1->equals(bq2));

    _CLLDELETE(bq1);
    _CLLDELETE(bq2);
}
void testException(CuTest *tc) {
    try {
        BooleanQuery::setMaxClauseCount(0);
        CuFail(tc, _T("setMaxClauseCount(0) did not throw an exception"));
    } catch (CLuceneError&) {
        // okay
    }
}

/// TestBooleanScorer.java, ported 5/9/2009
void testBooleanScorer(CuTest *tc) {
    const TCHAR* FIELD = _T("category");
    RAMDirectory directory;

    const TCHAR* values[] = { _T("1"), _T("2"), _T("3"), _T("4"), NULL};

    try {
        WhitespaceAnalyzer a;
        IndexWriter* writer = _CLNEW IndexWriter(&directory, &a, true);
        for (size_t i = 0; values[i]!=NULL; i++) {
            Document* doc = _CLNEW Document();
            doc->add(*_CLNEW Field(FIELD, values[i], Field::STORE_YES | Field::INDEX_TOKENIZED));
            writer->addDocument(doc);
            _CLLDELETE(doc);
        }
        writer->close();
        _CLLDELETE(writer);

        BooleanQuery* booleanQuery1 = _CLNEW BooleanQuery();
        Term *t = _CLNEW Term(FIELD, _T("1"));
        booleanQuery1->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
        _CLDECDELETE(t);
        t = _CLNEW Term(FIELD, _T("2"));
        booleanQuery1->add(_CLNEW TermQuery(t), true, BooleanClause::SHOULD);
        _CLDECDELETE(t);

        BooleanQuery* query = _CLNEW BooleanQuery();
        query->add(booleanQuery1, true, BooleanClause::MUST);
        t = _CLNEW Term(FIELD, _T("9"));
        query->add(_CLNEW TermQuery(t), true, BooleanClause::MUST_NOT);
        _CLDECDELETE(t);

        IndexSearcher *indexSearcher = _CLNEW IndexSearcher(&directory);
        Hits *hits = indexSearcher->search(query);
        CLUCENE_ASSERT(2 == hits->length()); // Number of matched documents
        _CLLDELETE(hits);
        _CLLDELETE(indexSearcher);

        _CLLDELETE(query);
    }
    catch (CLuceneError& e) {
        CuFail(tc, e.twhat());
    }
}

/// TestBooleanPrefixQuery.java, ported 5/9/2009
void testBooleanPrefixQuery(CuTest* tc) {
    RAMDirectory directory;
    WhitespaceAnalyzer a;

    const TCHAR* categories[] = {_T("food"), _T("foodanddrink"),
        _T("foodanddrinkandgoodtimes"), _T("food and drink"), NULL};

    Query* rw1 = NULL;
    Query* rw2 = NULL;
    try {
        IndexWriter* writer = _CLNEW IndexWriter(&directory, &a, true);
        for (size_t i = 0; categories[i]!=NULL; i++) {
            Document* doc = new Document();
            doc->add(*_CLNEW Field(_T("category"), categories[i], Field::STORE_YES | Field::INDEX_UNTOKENIZED));
            writer->addDocument(doc);
            _CLLDELETE(doc);
        }
        writer->close();
        _CLLDELETE(writer);

        IndexReader* reader = IndexReader::open(&directory);
        Term* t = _CLNEW Term(_T("category"), _T("foo"));
        PrefixQuery* query = _CLNEW PrefixQuery(t);
        _CLDECDELETE(t);

        rw1 = query->rewrite(reader);

        BooleanQuery* bq = _CLNEW BooleanQuery();
        bq->add(query, true, BooleanClause::MUST);

        rw2 = bq->rewrite(reader);

        reader->close(); // TODO: check necessity (_CLLDELETE(reader) alone will not do the same cleanup)

        _CLLDELETE(reader);
        _CLLDELETE(bq);
    } catch (CLuceneError& e) {
        CuFail(tc, e.twhat());
    }

    BooleanQuery* bq1 = NULL;
    if (rw1->instanceOf(BooleanQuery::getClassName())) {
        bq1 = (BooleanQuery*) rw1;
    }

    BooleanQuery* bq2 = NULL;
    if (rw2->instanceOf(BooleanQuery::getClassName())) {
        bq2 = (BooleanQuery*) rw2;
    } else {
        CuFail(tc, _T("Rewrite"));
    }

    bool bClausesMatch = bq1->getClauseCount() == bq2->getClauseCount();

    _CLLDELETE(rw1);
    _CLLDELETE(rw2);

    if (!bClausesMatch) {
        CuFail(tc, _T("Number of Clauses Mismatch"));
    }
}

void testBooleanScorer2WithProhibitedScorer(CuTest* tc) {
    CL_NS(search)::DefaultSimilarity similarity;
    BooleanScorer2 scorer(&similarity, 0, true);
    MockScorer prohibitedScorer(&similarity);
    scorer.add(&prohibitedScorer, false, true);
    CL_NS(search)::MockHitCollector collector;
    scorer.score(&collector);

    CuAssertIntEquals(tc, _T("Unexpected calls of next()!"), 1, prohibitedScorer.getNextCalls());
}

CuSuite *testBoolean(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene Boolean Tests"));

    SUITE_ADD_TEST(suite, testEquality);
    SUITE_ADD_TEST(suite, testException);

    SUITE_ADD_TEST(suite, testBooleanScorer);

    SUITE_ADD_TEST(suite, testBooleanPrefixQuery);
    SUITE_ADD_TEST(suite, testBooleanScorer2WithProhibitedScorer);

    //_CrtSetBreakAlloc(1179);

	return suite; 
}
// EOF
