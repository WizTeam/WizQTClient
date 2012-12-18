/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

#include "CLucene/search/RangeFilter.h"
#include "BaseTestRangeFilter.h"


class TestRangeFilter : BaseTestRangeFilter {
public:
    TestRangeFilter(CuTest* _tc)
        : BaseTestRangeFilter(_tc)
    {
    }

    void testRangeFilterId() {

        IndexReader* reader = IndexReader::open(index);
        IndexSearcher* search = new IndexSearcher(reader);

        int medId = ((maxId - minId) / 2);

        std::tstring minIPstr = pad(minId);
        const TCHAR* minIP = minIPstr.c_str();

        std::tstring maxIPstr = pad(maxId);
        const TCHAR* maxIP = maxIPstr.c_str();

        std::tstring medIPstr = pad(medId);
        const TCHAR* medIP = medIPstr.c_str();

        size_t numDocs = static_cast<size_t>(reader->numDocs());

        assertEqualsMsg(_T("num of docs"), numDocs, static_cast<size_t>(1+ maxId - minId));

        Hits* result;
        Term* term = _CLNEW Term(_T("body"),_T("body"));
        Query* q = _CLNEW TermQuery(term);
        _CLDECDELETE(term);

        // test id, bounded on both ends

        Filter* f = _CLNEW RangeFilter(_T("id"),minIP,maxIP,T,T);
        result = search->search(q, f);
        assertEqualsMsg(_T("find all"), numDocs, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),minIP,maxIP,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("all but last"), numDocs-1, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f =_CLNEW RangeFilter(_T("id"),minIP,maxIP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("all but first"), numDocs-1, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),minIP,maxIP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("all but ends"), numDocs-2, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),medIP,maxIP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("med and up"), 1+ maxId-medId, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),minIP,medIP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("up to med"), 1+ medId-minId, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        // unbounded id

        f=_CLNEW RangeFilter(_T("id"),minIP,NULL,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("min and up"), numDocs, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),NULL,maxIP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("max and down"), numDocs, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),minIP,NULL,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("not min, but up"), numDocs-1, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),NULL,maxIP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("not max, but down"), numDocs-1, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),medIP,maxIP,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("med and up, not max"), maxId-medId, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),minIP,medIP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("not min, up to med"), medId-minId, result->length());
        _CLLDELETE(result);
        _CLLDELETE(f);

        // very small sets

        f=_CLNEW RangeFilter(_T("id"),minIP,minIP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("min,min,F,F"), 0, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("id"),medIP,medIP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("med,med,F,F"), 0, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("id"),maxIP,maxIP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("max,max,F,F"), 0, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),minIP,minIP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("min,min,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("id"),NULL,minIP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("nul,min,F,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),maxIP,maxIP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("max,max,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("id"),maxIP,NULL,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("max,nul,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("id"),medIP,medIP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("med,med,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        search->close();
        _CLLDELETE(search);

        reader->close();
        _CLLDELETE(reader);

        _CLLDELETE(q);
    }

    void testRangeFilterRand()
    {
        IndexReader* reader = IndexReader::open(index);
        IndexSearcher* search = _CLNEW IndexSearcher(reader);

        std::tstring minRPstr = pad(minR);
        const TCHAR* minRP = minRPstr.c_str();
        
        std::tstring maxRPstr = pad(maxR);
        const TCHAR* maxRP = maxRPstr.c_str();

        size_t numDocs = static_cast<size_t>(reader->numDocs());

        assertEqualsMsg(_T("num of docs"), numDocs, 1+ maxId - minId);

        Hits* result;
        Term* term = _CLNEW Term(_T("body"),_T("body"));
        Query* q = _CLNEW TermQuery(term);
        _CLDECDELETE(term);

        // test extremes, bounded on both ends

        Filter* f = _CLNEW RangeFilter(_T("rand"),minRP,maxRP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("find all"), numDocs, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),minRP,maxRP,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("all but biggest"), numDocs-1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),minRP,maxRP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("all but smallest"), numDocs-1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),minRP,maxRP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("all but extremes"), numDocs-2, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        // unbounded

        f=_CLNEW RangeFilter(_T("rand"),minRP,NULL,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("smallest and up"), numDocs, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),NULL,maxRP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("biggest and down"), numDocs, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),minRP,NULL,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("not smallest, but up"), numDocs-1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),NULL,maxRP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("not biggest, but down"), numDocs-1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        // very small sets

        f=_CLNEW RangeFilter(_T("rand"),minRP,minRP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("min,min,F,F"), 0, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("rand"),maxRP,maxRP,F,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("max,max,F,F"), 0, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),minRP,minRP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("min,min,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("rand"),NULL,minRP,F,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("nul,min,F,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        f=_CLNEW RangeFilter(_T("rand"),maxRP,maxRP,T,T);
        result = search->search(q,f);
        assertEqualsMsg(_T("max,max,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);
        f=_CLNEW RangeFilter(_T("rand"),maxRP,NULL,T,F);
        result = search->search(q,f);
        assertEqualsMsg(_T("max,nul,T,T"), 1, result->length());
        _CLLDELETE(result); _CLLDELETE(f);

        search->close();
        _CLLDELETE(search);

        reader->close();
        _CLLDELETE(reader);

        _CLLDELETE(q);
    }
};

void testRangeFilterTrigger(CuTest* tc)
{
    TestRangeFilter trf(tc);
    trf.testRangeFilterId();
    trf.testRangeFilterRand();
}

void testIncludeLowerTrue(CuTest* tc)
{
    WhitespaceAnalyzer a;
    RAMDirectory* index = _CLNEW RAMDirectory();
    IndexWriter* writer = _CLNEW IndexWriter(index,
        &a, true);

    Document doc;
    doc.add(*_CLNEW Field(_T("Category"), _T("a 1"), Field::STORE_YES | Field::INDEX_TOKENIZED));
    writer->addDocument(&doc); doc.clear();

    doc.add(*_CLNEW Field(_T("Category"), _T("a 2"), Field::STORE_YES | Field::INDEX_TOKENIZED));
    writer->addDocument(&doc); doc.clear();

    doc.add(*_CLNEW Field(_T("Category"), _T("a 3"), Field::STORE_YES | Field::INDEX_TOKENIZED));
    writer->addDocument(&doc); doc.clear();

    writer->close();
    _CLLDELETE(writer);

    IndexSearcher* s = _CLNEW IndexSearcher(index);
    Filter* f = _CLNEW RangeFilter(_T("Category"), _T("3"), _T("3"), true, true);

    Term* t = _CLNEW Term(_T("Category"), _T("a"));
    Query* q1 = _CLNEW TermQuery(t);
    _CLLDECDELETE(t);

    t = _CLNEW Term(_T("Category"), _T("3"));
    Query* q2 = _CLNEW TermQuery(t);
    _CLLDECDELETE(t);

    Hits* h = s->search(q1);
    assertTrue(h->length() == 3);
    _CLLDELETE(h);

    h = s->search(q2);
    assertTrue(h->length() == 1);
    _CLLDELETE(h);

    h = s->search(q1, f);
    assertTrue(h->length() == 1);
    _CLLDELETE(h);

    s->close();
    _CLLDELETE(s);
    _CLLDELETE(q1);
    _CLLDELETE(q2);
    _CLLDELETE(f);

    index->close();
    _CLLDECDELETE(index);
}

CuSuite *testRangeFilter(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene RangeFilter Test"));

    SUITE_ADD_TEST(suite, testRangeFilterTrigger);
    SUITE_ADD_TEST(suite, testIncludeLowerTrue);

    return suite;
}


// EOF
