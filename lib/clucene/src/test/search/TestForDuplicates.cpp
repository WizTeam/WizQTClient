/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

  static void print_tHits( CuTest *tc, Hits* hits ) {
    CuMessageA(tc,"%d total results\n\n", hits->length());
    for (size_t i = 0 ; i < hits->length(); i++) {
      if ( i < 10 || (i > 94 && i < 105) ) {
        const Document& d = hits->doc(i);
	      CuMessage(tc, _T("%d %s\n"), i, d.get(_T("id")) );
      }
    }
  }
  void testSearchTestForDuplicatesRaw(CuTest *tc){
		const int MAX_DOCS=1500;
		const char *strBody[10] = {"test", "value", "why not", "computer", "clucene",
			"sun", "program", "main", "database", "code"};
		RAMDirectory ram;

		//---
		WhitespaceAnalyzer an;
		IndexWriter* writer = _CLNEW IndexWriter(&ram, &an, true);
		Document *doc = 0;

		//---
		TCHAR strDb[1024];
		//printf("Indexing, please wait...\n");
		for (int32_t i = 0; i < MAX_DOCS; i++) {
			//****
			//printf("%d/%d=%s\n", i, MAX_DOCS,strBody[i%10]);
			doc = _CLNEW Document();

			//---
			_sntprintf(strDb, 1024, _T("%d"), i);
			doc->add( *_CLNEW Field(_T("id"), strDb,Field::STORE_YES | Field::INDEX_UNTOKENIZED) );

			STRCPY_AtoT(strDb, strBody[i%10], 1022);

			doc->add(*_CLNEW Field(_T("body"), strDb,Field::STORE_NO | Field::INDEX_TOKENIZED) );
			//---
			writer->addDocument(doc);
			_CLDELETE(doc);
			//****
		}
		//printf("\nDone.\n");

		//---
		writer->close();
		_CLDELETE(writer);



		IndexSearcher searcher(&ram);
		//---
		int32_t dupl = 0;
		Query* query = QueryParser::parse(_T("test"), _T("body"), &an);
		Hits* result = searcher.search(query);

		CLUCENE_ASSERT(result->length()==((int)MAX_DOCS/10));

		//printf("Building result map...\n");
		std::map<int32_t, int32_t> resMap;
		int32_t id;
		for (size_t j = 0; j < result->length(); j++) {
			doc = &result->doc(j);

			id = _ttoi(doc->get(_T("id")));
			if ( resMap.find(id) ==resMap.end() ) {
				resMap.insert( std::pair<int32_t,int32_t>(id, 1));
				//printf("Inserted $d\n",id);
			} else {
				TCHAR tmp[2048];
				_sntprintf(tmp,2048,_T("Duplicated result found - Id: %d\n"), id);
				CuAssert(tc,tmp,false);
				dupl++;
			}
		}
		//printf("Total duplicated found: %d\n", dupl);

		//---
		_CLDELETE(result);
		_CLDELETE(query);
		searcher.close();
		ram.close();
	}

   void testSearchTestForDuplicates(CuTest *tc) {
      RAMDirectory directory;
      SimpleAnalyzer analyzer;
      IndexWriter* writer = _CLNEW IndexWriter(&directory, &analyzer, true);
      const int32_t MAX_DOCS = 255;

      for (int32_t j = 0; j < MAX_DOCS; j++) {
		    Document* d = _CLNEW Document();
		    d->add(*_CLNEW Field(_T("priority"), _T("high"),Field::STORE_YES | Field::INDEX_TOKENIZED));
		    TCHAR buf[80];
		    _i64tot(j,buf,10);

		    d->add(*_CLNEW Field(_T("id"), buf,Field::STORE_YES | Field::INDEX_TOKENIZED));
		    writer->addDocument(d);

		    _CLDELETE(d);
	    }
	    writer->close();
	    _CLDELETE(writer);

      // try a search without OR
      Searcher* searcher = _CLNEW IndexSearcher( &directory );
      QueryParser* parser = _CLNEW QueryParser(_T("priority"), &analyzer);
      Hits* hits = NULL;

      Query* query = parser->parse(_T("high"));
      TCHAR* tmp = query->toString(_T("priority"));
      CuMessage(tc, _T("Query: %s\n"), tmp );
      _CLDELETE_CARRAY(tmp);

      hits = searcher->search(query);
      print_tHits(tc, hits);
      _CLDELETE(hits);
      _CLDELETE(query);
      _CLDELETE(parser);

      searcher->close();
      _CLDELETE(searcher);



      // try a new search with OR
      searcher = _CLNEW IndexSearcher( &directory );
      parser = _CLNEW QueryParser(_T("priority"), &analyzer);
      hits = NULL;

      query = parser->parse(_T("high OR medium"));
      tmp = query->toString(_T("priority"));
      CuMessage(tc, _T("Query: %s\n"), tmp );
      _CLDELETE_CARRAY(tmp);

      hits = searcher->search(query);
      print_tHits(tc, hits);
      _CLDELETE(hits);
      _CLDELETE(query);
      _CLDELETE(parser);

      searcher->close();
      _CLDELETE(searcher);

      directory.close();
   }


  CuSuite *testduplicates(void)
  {
	  CuSuite *suite = CuSuiteNew(_T("CLucene Duplicates Test"));

    SUITE_ADD_TEST(suite, testSearchTestForDuplicates);
    SUITE_ADD_TEST(suite, testSearchTestForDuplicatesRaw);

    return suite;
  }
// EOF
