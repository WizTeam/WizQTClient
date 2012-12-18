/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

	void testBefore(CuTest *tc) {
	// create an index
		char fsdir[CL_MAX_PATH];
		_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "dfindex");
		
		FSDirectory* indexStore = FSDirectory::getDirectory( fsdir);
		Analyzer* a = _CLNEW SimpleAnalyzer();
		IndexWriter* writer = _CLNEW IndexWriter(indexStore, a, true);
     	int64_t now = Misc::currentTimeMillis()/1000;

     	Document doc;
     	// add time that is in the past
		TCHAR* tn = DateField::timeToString(now - 1000);
     	doc.add(*_CLNEW Field(_T("datefield"), tn,Field::STORE_YES | Field::INDEX_UNTOKENIZED));
		_CLDELETE_CARRAY(tn);
     	doc.add(*_CLNEW Field(_T("body"), _T("Today is a very sunny day in New York City"),Field::STORE_YES | Field::INDEX_TOKENIZED));
      	writer->addDocument(&doc);
    	writer->close();
		_CLDELETE( writer );

		IndexReader* reader = IndexReader::open(indexStore);
    	IndexSearcher* searcher = _CLNEW IndexSearcher(reader);
	
    	// filter that should preserve matches
		DateFilter* df1 = DateFilter::Before(_T("datefield"), now);
	
    	// filter that should discard matches
    	DateFilter* df2 = DateFilter::Before(_T("datefield"), now - 999999);
	
    	// search something that doesn't exist with DateFilter
		Term* term = _CLNEW Term(_T("body"), _T("NoMatchForThis"));
    	Query* query1 = _CLNEW TermQuery(term);
		_CLDECDELETE(term);

    	// search for something that does exists
		term=_CLNEW Term(_T("body"), _T("sunny"));
    	Query* query2 = _CLNEW TermQuery(term);
		_CLDECDELETE(term);
	
    	Hits* result = NULL;
	
    	// ensure that queries return expected results without DateFilter first
    	result = searcher->search(query1);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);
	
    	result = searcher->search(query2);
   		CLUCENE_ASSERT(1 == result->length());
		_CLDELETE(result);
	
    	// run queries with DateFilter
    	result = searcher->search(query1, df1);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

    	result = searcher->search(query1, df2);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

     	result = searcher->search(query2, df1);
     	CLUCENE_ASSERT(1 == result->length());
		_CLDELETE(result);

    	result = searcher->search(query2, df2);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

		reader->close();
		searcher->close();
		_CLDELETE(a)
		_CLDELETE(reader);
		_CLDELETE(searcher);
		_CLDELETE(query1);
		_CLDELETE(query2);
		_CLDELETE(df1);
		_CLDELETE(df2);
		
		indexStore->close();
		_CLDECDELETE(indexStore);
	}

	void testAfter(CuTest *tc) {
		// create an index
		RAMDirectory* indexStore = _CLNEW RAMDirectory;
		Analyzer* a = _CLNEW SimpleAnalyzer();
		IndexWriter* writer = _CLNEW IndexWriter(indexStore, a, true);
		int64_t now = Misc::currentTimeMillis()/1000;

		// add time that is in the future
		TCHAR* tf = DateField::timeToString(now + 888888);

     	Document* doc = _CLNEW Document;
		doc->add(*_CLNEW Field(_T("datefield"), tf,Field::STORE_YES | Field::INDEX_UNTOKENIZED));
		_CLDELETE_CARRAY(tf);

     	doc->add(*_CLNEW Field(_T("body"), _T("Today is a very sunny day in New York City"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		writer->addDocument(doc);
		_CLDELETE(doc);

   		writer->close();
		_CLDELETE( writer );

		//read the index
		IndexReader* reader = IndexReader::open(indexStore);
    	IndexSearcher* searcher = _CLNEW IndexSearcher(reader);

		// filter that should preserve matches
    	DateFilter* df1 = DateFilter::After(_T("datefield"), now);
	
    	// filter that should discard matches
    	DateFilter* df2 = DateFilter::After(_T("datefield"), now + 999999);
	
    	// search something that doesn't exist with DateFilter
		Term* term = _CLNEW Term(_T("body"), _T("NoMatchForThis"));
    	Query* query1 = _CLNEW TermQuery(term);
		_CLDECDELETE(term);
	
    	// search for something that does exists
		term=_CLNEW Term(_T("body"), _T("sunny"));
    	Query* query2 = _CLNEW TermQuery(term);
		_CLDECDELETE(term);
	
    	Hits* result = NULL;

		// ensure that queries return expected results without DateFilter first
    	result = searcher->search(query1);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

    	result = searcher->search(query2);
    	CLUCENE_ASSERT(1 == result->length());
		_CLDELETE(result);

		// run queries with DateFilter
    	result = searcher->search(query1, df1);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

		result = searcher->search(query1, df2);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

     	result = searcher->search(query2, df1);
     	CLUCENE_ASSERT(1 == result->length());
		_CLDELETE(result);
		
    	result = searcher->search(query2, df2);
    	CLUCENE_ASSERT(0 == result->length());
		_CLDELETE(result);

		reader->close();
		searcher->close();

		_CLDELETE(query1);
		_CLDELETE(query2);
		_CLDELETE(df1);
		_CLDELETE(df2);
		_CLDELETE(reader);
		_CLDELETE(searcher);
		_CLDELETE(a);
		indexStore->close();
		_CLDECDELETE(indexStore);
	}

	void testDateFilterDestructor(CuTest *tc){
		char loc[1024];
		strcpy(loc, clucene_data_location);
		strcat(loc, "/reuters-21578-index");

		CuAssert(tc,_T("Index does not exist"),Misc::dir_Exists(loc));
		IndexReader* reader = IndexReader::open(loc);
		int64_t now = Misc::currentTimeMillis()/1000;
    	DateFilter* df1 = DateFilter::After(_T("datefield"), now);
		BitSet* bs = df1->bits(reader);
		_CLDELETE(bs);
		_CLDELETE(df1);

		reader->close();
		_CLDELETE(reader);
	}


CuSuite *testdatefilter(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene DateFilter Test"));

    SUITE_ADD_TEST(suite, testDateFilterDestructor);
    SUITE_ADD_TEST(suite, testBefore);
    SUITE_ADD_TEST(suite, testAfter);

    return suite; 
}
// EOF
