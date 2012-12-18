/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

IndexSearcher* tv_searcher = NULL;
RAMDirectory* tv_directory = NULL;

void testTermPositionVectors(CuTest *tc) {
    CLUCENE_ASSERT(tv_searcher!=NULL);

    Term* term = _CLNEW Term(_T("field"), _T("fifty"));
    TermQuery query(term);
    _CLDECDELETE(term);
    try {
      Hits* hits = tv_searcher->search(&query);
      CuAssert (tc,_T("hits.length != 100"), 100 == hits->length());

      for (size_t i = 0; i < hits->length(); i++)
      {
		    ArrayBase<TermFreqVector*>* vector = tv_searcher->getReader()->getTermFreqVectors(hits->id(i));
		    CLUCENE_ASSERT(vector != NULL);
		    CLUCENE_ASSERT(vector->length== 1);
		    vector->deleteValues();
		    _CLLDELETE(vector);
      }

      _CLDELETE(hits);
    } catch (CLuceneError& e) {
        if ( e.number() == CL_ERR_IO )
            CuAssert(tc, _T("IO Error"),false);

        throw e;
    }
}
void testTermVectors(CuTest *tc) {
    CLUCENE_ASSERT(tv_searcher!=NULL);

    Term* term = _CLNEW Term(_T("field"), _T("seventy"));
    TermQuery query(term);
    _CLDECDELETE(term);

    try {
      Hits* hits = tv_searcher->search(&query);
      CuAssertIntEquals(tc,_T("hits!=100"), 100, hits->length());

      for (size_t i = 0; i < hits->length(); i++)
      {
        ArrayBase<TermFreqVector*>* vector = tv_searcher->getReader()->getTermFreqVectors(hits->id(i));
        CLUCENE_ASSERT(vector != NULL);
        CLUCENE_ASSERT(vector->length == 1);
		    vector->deleteValues();
		    _CLLDELETE(vector);
      }

	  //test mem leaks with vectors
      CL_NS(search)::Explanation expl;
	    tv_searcher->explain(&query, hits->id(50), &expl);
      TCHAR* tmp = expl.toString();
      _CLDELETE_CARRAY(tmp);

      _CLDELETE(hits);

    } catch (CLuceneError& e) {
        if ( e.number() == CL_ERR_IO )
            CuAssert(tc,_T("IO Exception"),false);
        else
            throw e;
    }
}

void testTVSetup(CuTest* /*tc*/) {
    SimpleAnalyzer a;
    tv_directory = _CLNEW RAMDirectory();
    IndexWriter writer(tv_directory, &a, true);
    writer.setUseCompoundFile(false);

    TCHAR buf[200];
    for (int32_t i = 0; i < 1000; i++) {
      Document doc;
      English::IntToEnglish(i,buf,200);

	  int mod3 = i % 3;
      int mod2 = i % 2;
	  int termVector = 0;
      if (mod2 == 0 && mod3 == 0)
		  termVector = Field::TERMVECTOR_WITH_POSITIONS_OFFSETS;
      else if (mod2 == 0)
		  termVector = Field::TERMVECTOR_WITH_POSITIONS;
      else if (mod3 == 0)
		  termVector = Field::TERMVECTOR_WITH_OFFSETS;
      else
		  termVector = Field::TERMVECTOR_YES;

	  doc.add(*new Field(_T("field"), buf, Field::STORE_YES | Field::INDEX_TOKENIZED | termVector ));
      writer.addDocument(&doc);
    }
    writer.close();
    tv_searcher = _CLNEW IndexSearcher(tv_directory);
}
void testTVCleanup(CuTest* /*tc*/) {
    _CLDELETE(tv_searcher);
    tv_directory->close();
    _CLDELETE(tv_directory);
}

void setupDoc(Document& doc, const TCHAR* text)
{
	doc.add(*new Field(_T("field"), text, Field::STORE_YES |
		Field::INDEX_TOKENIZED | Field::TERMVECTOR_YES));
}
struct __TCharCompare
{
  bool operator()(const TCHAR* s1, const TCHAR* s2) const
  {
    return _tcscmp(s1, s2) < 0;
  }
};

void testKnownSetOfDocuments(CuTest *tc) {
    const TCHAR* test1 = _T("eating chocolate in a computer lab"); //6 terms
    const TCHAR* test2 = _T("computer in a computer lab"); //5 terms
    const TCHAR* test3 = _T("a chocolate lab grows old"); //5 terms
    const TCHAR* test4 = _T("eating chocolate with a chocolate lab in an old chocolate colored computer lab"); //13 terms

    typedef std::map<const TCHAR*, int32_t, __TCharCompare> test4MapType;
    test4MapType test4Map;
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("chocolate"), 3) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("lab"), 2) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("eating"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("computer"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("with"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("a"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("colored"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("in"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("an"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("computer"), 1) );
    test4Map.insert( std::pair<const TCHAR*,int32_t>(_T("old"), 1) );

    Document testDoc1;
    setupDoc(testDoc1, test1);
    Document testDoc2;
    setupDoc(testDoc2, test2);
    Document testDoc3;
    setupDoc(testDoc3, test3);
    Document testDoc4;
    setupDoc(testDoc4, test4);

    RAMDirectory dir;

    try {
      SimpleAnalyzer a;
      IndexWriter writer(&dir, &a, true);

      writer.addDocument(&testDoc1);
      writer.addDocument(&testDoc2);
      writer.addDocument(&testDoc3);
      writer.addDocument(&testDoc4);
      writer.close();

      IndexSearcher knownSearcher(&dir);
      TermEnum* termEnum = knownSearcher.getReader()->terms();
      TermDocs* termDocs = knownSearcher.getReader()->termDocs();

      CL_NS(search)::Similarity* sim = knownSearcher.getSimilarity();
      while (termEnum->next() == true)
      {
        Term* term = termEnum->term(true);
        //System.out.println("Term: " + term);
        termDocs->seek(term);

        while (termDocs->next())
        {
          int32_t docId = termDocs->doc();
          int32_t freq = termDocs->freq();
          //System.out.println("Doc Id: " + docId + " freq " + freq);
          TermFreqVector* vector = knownSearcher.getReader()->getTermFreqVector(docId, _T("field"));
          float_t tf = sim->tf(freq);
          float_t idf = sim->idf(term, &knownSearcher);
          //float_t qNorm = sim.queryNorm()
          idf += tf; //remove warning

          const ArrayBase<const TCHAR*>* terms = vector->getTerms();
          CLUCENE_ASSERT(vector != NULL);
          int termsCount=terms != NULL ? terms->length : 0;

          //This is fine since we don't have stop words
          float_t lNorm = sim->lengthNorm(_T("field"), termsCount);
          lNorm ++;//remove warning

          //float_t coord = sim.coord()
          //System.out.println("TF: " + tf + " IDF: " + idf + " LenNorm: " + lNorm);
          const ArrayBase<const TCHAR*>* vTerms = vector->getTerms();
          const ArrayBase<int32_t>* freqs = vector->getTermFrequencies();
          size_t i=0;
          while ( vTerms && i < vTerms->length )
          {
            if ( _tcscmp(term->text(), vTerms->values[i]) == 0 )
            {
              CLUCENE_ASSERT((*freqs)[i] == freq);
            }
            i++;
          }

          _CLDELETE(vector);
        }
        _CLDECDELETE(term);
        //System.out.println("--------");
      }
      _CLDELETE(termEnum);
      _CLDELETE(termDocs);


      Term* tqTerm = _CLNEW Term(_T("field"), _T("chocolate"));
      TermQuery query(tqTerm);
      _CLDECDELETE(tqTerm);

      Hits* hits = knownSearcher.search(&query);
      //doc 3 should be the first hit b/c it is the shortest match
      CLUCENE_ASSERT(hits->length() == 3);
      float_t score = hits->score(0);
      score++;

      CLUCENE_ASSERT(2==hits->id(0) );
      CLUCENE_ASSERT(3==hits->id(1) );
      CLUCENE_ASSERT(0==hits->id(2) );

      TermFreqVector* vector = knownSearcher.getReader()->getTermFreqVector(hits->id(1), _T("field"));
      CLUCENE_ASSERT(vector != NULL);
      //_tprintf(_T("Vector: %s\n"),vector);
      const ArrayBase<const TCHAR*>* terms = vector->getTerms();
      const ArrayBase<int32_t>* freqs = vector->getTermFrequencies();
      CLUCENE_ASSERT(terms != NULL);

      int termsLength = terms->length;
      CLUCENE_ASSERT(termsLength == 10);

      for (int32_t i = 0; i < termsLength; i++) {
        const TCHAR* term = terms->values[i];
        //_tprintf(_T("Term: %s, test4map.size()=%d\n"),term, test4Map.size());
        int32_t freq = (*freqs)[i];
        CLUCENE_ASSERT( _tcsstr(test4,term) != NULL );
        test4MapType::const_iterator itr = test4Map.find(term);
        CLUCENE_ASSERT( itr != test4Map.end() );
        int32_t freqInt = itr->second;
        CLUCENE_ASSERT(freqInt == freq);
      }
      _CLDELETE(vector);
      _CLDELETE(hits);
      knownSearcher.close();

    } catch (CLuceneError& e) {
      CuAssert(tc, e.twhat(),false);
    }
}

CuSuite *testtermvector(void)
{
    tv_searcher = NULL;
	CuSuite *suite = CuSuiteNew(_T("CLucene Term Vector Test"));
    SUITE_ADD_TEST(suite, testTVSetup);
    SUITE_ADD_TEST(suite, testKnownSetOfDocuments);
    SUITE_ADD_TEST(suite, testTermVectors);
    SUITE_ADD_TEST(suite, testTermPositionVectors);
    SUITE_ADD_TEST(suite, testTVCleanup);

    return suite;
}
// EOF
