/*------------------------------------------------------------------------------
* Copyright (C) 2010 the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

DEFINE_MUTEX(searchMutex);
DEFINE_CONDITION(searchCondition);

DEFINE_MUTEX(deleteMutex);
DEFINE_CONDITION(deleteCondition);

_LUCENE_THREAD_FUNC(searchDocs, _searcher) {

    WhitespaceAnalyzer an;
    IndexSearcher * searcher = (IndexSearcher *)_searcher;
    Query * query = QueryParser::parse(_T("one"), _T("content"), &an);
    Hits * hits = searcher->search(query);
    
    usleep(9999); //make sure that searchMutex is being waited on...

    CONDITION_NOTIFYALL(searchCondition);
    SCOPED_LOCK_MUTEX(deleteMutex);

    _CLLDELETE(hits);
    _CLLDELETE(query);

    CONDITION_WAIT(deleteMutex, deleteCondition);
    _LUCENE_THREAD_FUNC_RETURN(0);
}

void testEndThreadException(CuTest *tc) {

    const int MAX_DOCS=1500;
	RAMDirectory ram;
	WhitespaceAnalyzer an;
	IndexWriter* writer = _CLNEW IndexWriter(&ram, &an, true);

    // add some documents
    Document doc;
    for (int i = 0; i < MAX_DOCS; i++) {
        TCHAR * tmp = English::IntToEnglish(i);
        doc.add(* new Field(_T("content"), tmp, Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        writer->addDocument(&doc);
        doc.clear();
        _CLDELETE_ARRAY( tmp );
    }

    CuAssertEquals(tc, MAX_DOCS, writer->docCount());
    
    writer->close();
	_CLLDELETE(writer);
    
    // this sequence is OK: delete searcher after search thread finish
    {
        IndexSearcher * searcher = _CLNEW IndexSearcher(&ram);
        _LUCENE_THREADID_TYPE thread = _LUCENE_THREAD_CREATE(&searchDocs, searcher);
        SCOPED_LOCK_MUTEX(searchMutex);

        CONDITION_WAIT(searchMutex, searchCondition);
        usleep(9999); //make sure that deleteMutex is being waited on...
        CONDITION_NOTIFYALL(deleteCondition);

        _LUCENE_THREAD_JOIN(thread);

        searcher->close();
        _CLLDELETE(searcher);
    }

    // this produces memory exception: delete searcher after search finish but before thread finish
    {
        IndexSearcher * searcher = _CLNEW IndexSearcher(&ram);
        _LUCENE_THREADID_TYPE thread = _LUCENE_THREAD_CREATE(&searchDocs, searcher);
        SCOPED_LOCK_MUTEX(searchMutex);

        CONDITION_WAIT(searchMutex, searchCondition);
        searcher->close();
        _CLLDELETE(searcher);
        CONDITION_NOTIFYALL(deleteCondition);

        _LUCENE_THREAD_JOIN(thread);
    }


    ram.close();
}


CuSuite *testIndexSearcher(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene IndexSearcher Test"));

    SUITE_ADD_TEST(suite, testEndThreadException);

    return suite;
  }
// EOF
