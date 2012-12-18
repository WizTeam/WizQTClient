/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

#ifndef NO_WILDCARD_QUERY

void _testWildcard(CuTest* tc, IndexSearcher* searcher, const TCHAR* qt, int expectedLen){
	Term* term = _CLNEW Term(_T("body"), qt);
	Query* query = _CLNEW WildcardQuery(term);

	//test the wildcardquery
	Hits* result = searcher->search(query);
	CLUCENE_ASSERT(expectedLen == result->length());
	_CLDELETE(result);
	_CLDELETE(query);


	//now test wildcardfilter
	Filter* filter = _CLNEW WildcardFilter(term);
	BitSet* bits = filter->bits(searcher->getReader());
	CLUCENE_ASSERT(expectedLen == bits->count());
	_CLDELETE(filter);
	_CLDELETE(bits);

	_CLDECDELETE(term);
}




	void testAsterisk(CuTest *tc){
		RAMDirectory indexStore;
		SimpleAnalyzer an;
		IndexWriter* writer = _CLNEW IndexWriter(&indexStore, &an, true);
		Document doc1;
		Document doc2;
		doc1.add(*_CLNEW Field(_T("body"), _T("metal"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		doc2.add(*_CLNEW Field(_T("body"), _T("metals"),Field::STORE_YES | Field::INDEX_TOKENIZED));

		writer->addDocument(&doc1);
		writer->addDocument(&doc2);
    	writer->close();
		_CLDELETE(writer);
        //////////////////////////////////////////////////
		
		IndexReader* reader = IndexReader::open(&indexStore);
		IndexSearcher* searcher = _CLNEW IndexSearcher(reader);

		_testWildcard(tc, searcher, _T("metal*"), 2);
		_testWildcard(tc, searcher, _T("m*tal"), 1);
		_testWildcard(tc, searcher, _T("m*tal*"), 2);
			

		Term* term = _CLNEW Term(_T("body"), _T("metal"));
		Query* query1 = _CLNEW TermQuery(term);
		_CLDECDELETE(term);

    	Hits* result = searcher->search(query1);
    	CLUCENE_ASSERT(1 == result->length());
		_CLDELETE(result);
		_CLDELETE(query1);

	
		indexStore.close();
		searcher->close();
		reader->close();
		_CLDELETE(reader);
		_CLDELETE(searcher);
	}

	void testQuestionmark(CuTest *tc){
		RAMDirectory indexStore;
		SimpleAnalyzer an;
		IndexWriter* writer = _CLNEW IndexWriter(&indexStore, &an, true);
		Document doc1;
		Document doc2;
		Document doc3;
		Document doc4;
		doc1.add(*_CLNEW Field(_T("body"), _T("metal"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		doc2.add(*_CLNEW Field(_T("body"), _T("metals"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		doc3.add(*_CLNEW Field(_T("body"), _T("mXtals"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		doc4.add(*_CLNEW Field(_T("body"), _T("mXtXls"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		writer->addDocument(&doc1);
		writer->addDocument(&doc2);
		writer->addDocument(&doc3);
		writer->addDocument(&doc4);
		writer->close();
		_CLDELETE(writer);
        //////////////////////////////////////////////////////
    	
		IndexReader* reader = IndexReader::open(&indexStore);
		IndexSearcher* searcher = _CLNEW IndexSearcher(reader);
		

		_testWildcard(tc, searcher, _T("m?tal"), 1);
		_testWildcard(tc, searcher, _T("metal?"), 1);
		_testWildcard(tc, searcher, _T("metal??"), 0);
		_testWildcard(tc, searcher, _T("meta??"), 1); //metals
		_testWildcard(tc, searcher, _T("metals?"), 0);
		_testWildcard(tc, searcher, _T("m?t?ls"), 3);

		indexStore.close();
		reader->close();
		searcher->close();
		_CLDELETE(reader);
		_CLDELETE(searcher);
	}
#else
	void _NO_WILDCARD_QUERY(CuTest *tc){
		CuNotImpl(tc,_T("Wildcard"));
	}
#endif


CuSuite *testwildcard(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Wildcard Test"));

	#ifndef NO_WILDCARD_QUERY
		SUITE_ADD_TEST(suite, testQuestionmark);
		SUITE_ADD_TEST(suite, testAsterisk);
	#else
		SUITE_ADD_TEST(suite, _NO_WILDCARD_QUERY);
    #endif

    return suite; 
}
// EOF
