/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include <assert.h>
#include "test.h"
#include <stdio.h>

#include "CLucene/search/MultiPhraseQuery.h"

	SimpleAnalyzer a;
	StandardAnalyzer aStd;
	WhitespaceAnalyzer aWS;
	IndexSearcher* s=NULL;

	void _TestSearchesRun(CuTest *tc, Analyzer* analyzer, Searcher* search, const TCHAR* qry){
		Query* q = NULL;
		Hits* h = NULL;
		try{
			q = QueryParser::parse(qry , _T("contents"), analyzer);
			if ( q != NULL ){
			    h = search->search( q );

			    if ( h->length() > 0 ){
			    //check for explanation memory leaks...
          CL_NS(search)::Explanation expl1;
					search->explain(q, h->id(0), &expl1);
					TCHAR* tmp = expl1.toString();
					_CLDELETE_CARRAY(tmp);
					if ( h->length() > 1 ){ //do a second one just in case
						CL_NS(search)::Explanation expl2;
						search->explain(q, h->id(1), &expl2);
						tmp = expl2.toString();
						_CLDELETE_CARRAY(tmp);
					}
				}
			}
    }catch(CLuceneError& err){
      CuFail(tc,_T("Error: %s\n"), err.twhat());
    }catch(...){
      CuFail(tc,_T("Error: unknown\n"));
    }
		_CLDELETE(h);
		_CLDELETE(q);
	}

	void testSrchOpenIndex(CuTest *tc ){
		char loc[1024];
		strcpy(loc, clucene_data_location);
		strcat(loc, "/reuters-21578-index");

		CuAssert(tc,_T("Index does not exist"), Misc::dir_Exists(loc));
		s=_CLNEW IndexSearcher(loc);
  }
	void testSrchCloseIndex(CuTest* /*tc*/ ){
		if ( s!=NULL ){
			s->close();
			_CLDELETE(s);
		}
  }

	void testSrchPunctuation(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);

		//test punctuation
		_TestSearchesRun(tc, &a,s, _T("a&b") );
		_TestSearchesRun(tc, &a,s, _T("a&&b") );
		_TestSearchesRun(tc, &a,s, _T(".NET") );
	}

	void testSrchSlop(CuTest *tc ){
#ifdef NO_FUZZY_QUERY
		CuNotImpl(tc,_T("Fuzzy"));
#else
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//test slop
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"~2") );
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"~2 flork") );
		_TestSearchesRun(tc, &a,s, _T("\"term\"~2") );
		_TestSearchesRun(tc, &a,s, _T("\" \"~2 germ") );
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"~2^2") );
#endif
	}

	void testSrchNumbers(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);

		// The numbers go away because SimpleAnalzyer ignores them
		_TestSearchesRun(tc, &a,s, _T("3") );
		_TestSearchesRun(tc, &a,s, _T("term 1.0 1 2") );
		_TestSearchesRun(tc, &a,s, _T("term term1 term2") );

		_TestSearchesRun(tc, &aStd,s, _T("3") );
		_TestSearchesRun(tc, &aStd,s, _T("term 1.0 1 2") );
		_TestSearchesRun(tc, &aStd,s, _T("term term1 term2") );
	}

	void testSrchWildcard(CuTest *tc ){
#ifdef NO_WILDCARD_QUERY
		CuNotImpl(tc,_T("Wildcard"));
#else
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//testWildcard
		_TestSearchesRun(tc, &a,s, _T("term*") );
		_TestSearchesRun(tc, &a,s, _T("term*^2") );
		_TestSearchesRun(tc, &a,s, _T("term~") );
		_TestSearchesRun(tc, &a,s, _T("term^2~") );
		_TestSearchesRun(tc, &a,s, _T("term~^2") );
		_TestSearchesRun(tc, &a,s, _T("term*germ") );
		_TestSearchesRun(tc, &a,s, _T("term*germ^3") );

		//test problem reported by Gary Mangum
		BooleanQuery* bq = _CLNEW BooleanQuery();
		Term* upper = _CLNEW Term(_T("contents"),_T("0105"));
		Term* lower = _CLNEW Term(_T("contents"),_T("0105"));
		RangeQuery* rq=_CLNEW RangeQuery(lower,upper,true);
		bq->add(rq,true,true,false);
		_CLDECDELETE(upper);
		_CLDECDELETE(lower);

		Term* prefix = _CLNEW Term(_T("contents"),_T("reuters21578"));
		PrefixQuery* pq = _CLNEW PrefixQuery(prefix);
		_CLDECDELETE(prefix);
		bq->add(pq,true,true,false);

		Hits* h = NULL;
		try{
			h = s->search( bq );
		}_CLFINALLY(
		_CLDELETE(h);
		_CLDELETE(bq);
		);
#endif
	}

	void testSrchEscapes(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//testEscaped
		_TestSearchesRun(tc, &aWS,s, _T("\\[brackets") );
		_TestSearchesRun(tc, &a,s, _T("\\[brackets") );
		_TestSearchesRun(tc, &aWS,s, _T("\\\\") );
		_TestSearchesRun(tc, &aWS,s, _T("\\+blah") );
		_TestSearchesRun(tc, &aWS,s, _T("\\(blah") );
	}

	void testSrchRange(CuTest *tc ){
#ifdef NO_RANGE_QUERY
		CuNotImpl(tc,_T("Range"));
#else
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
		//testRange
		_TestSearchesRun(tc, &a,s, _T("[ j m]") );
		_TestSearchesRun(tc, &a,s, _T("[ j m ]") );
		_TestSearchesRun(tc, &a,s, _T("{ j m}") );
		_TestSearchesRun(tc, &a,s, _T("{ j m }") );
		_TestSearchesRun(tc, &a,s, _T("{a TO b}") );
		_TestSearchesRun(tc, &a,s, _T("{ j m }^2.0") );
		_TestSearchesRun(tc, &a,s, _T("[ j m] OR bar") );
		_TestSearchesRun(tc, &a,s, _T("[ j m] AND bar") );
		_TestSearchesRun(tc, &a,s, _T("( bar blar { j m}) ") );
		_TestSearchesRun(tc, &a,s, _T("gack ( bar blar { j m}) ") );
#endif
	}

	void testSrchSimple(CuTest *tc ){
		CuAssert(tc,_T("Searcher was not open"),s!=NULL);
    	//simple tests
		_TestSearchesRun(tc, &a,s, _T("a AND b") );

		_TestSearchesRun(tc, &a,s, _T("term term term") );

#ifdef _UCS2
		TCHAR tmp1[100];
		lucene_utf8towcs(tmp1,"t\xc3\xbcrm term term",100);
		_TestSearchesRun(tc, &a,s, tmp1 );

		lucene_utf8towcs(tmp1,"\xc3\xbcmlaut",100);
		_TestSearchesRun(tc, &a,s, tmp1 );
#endif

		_TestSearchesRun(tc, &a,s, _T("(a AND b)") );
		_TestSearchesRun(tc, &a,s, _T("c OR (a AND b)") );
		_TestSearchesRun(tc, &a,s, _T("a AND NOT b") );
		_TestSearchesRun(tc, &a,s, _T("a AND -b") );
		_TestSearchesRun(tc, &a,s, _T("a AND !b") );
		_TestSearchesRun(tc, &a,s, _T("a && b") );
		_TestSearchesRun(tc, &a,s, _T("a && ! b") );

		_TestSearchesRun(tc, &a,s, _T("a OR b") );
		_TestSearchesRun(tc, &a,s, _T("a || b") );
		_TestSearchesRun(tc, &a,s, _T("a OR !b") );
		_TestSearchesRun(tc, &a,s, _T("a OR ! b") );
		_TestSearchesRun(tc, &a,s, _T("a OR -b") );

		_TestSearchesRun(tc, &a,s, _T("+term -term term") );
		_TestSearchesRun(tc, &a,s, _T("foo:term AND field:anotherTerm") );
		_TestSearchesRun(tc, &a,s, _T("term AND \"phrase phrase\"") );
		_TestSearchesRun(tc, &a,s, _T("search AND \"meaningful direction\"") );
		_TestSearchesRun(tc, &a,s, _T("\"hello there\"") );

		_TestSearchesRun(tc, &a,s,  _T("a AND b") );
		_TestSearchesRun(tc, &a,s,  _T("hello") );
		_TestSearchesRun(tc, &a,s,  _T("\"hello there\"") );

		_TestSearchesRun(tc, &a,s, _T("germ term^2.0") );
		_TestSearchesRun(tc, &a,s, _T("term^2.0") );
		_TestSearchesRun(tc, &a,s, _T("term^2") );
		_TestSearchesRun(tc, &a,s, _T("term^2.3") );
		_TestSearchesRun(tc, &a,s, _T("\"germ term\"^2.0") );
		_TestSearchesRun(tc, &a,s, _T("\"term germ\"^2") );

		_TestSearchesRun(tc, &a,s, _T("(foo OR bar) AND (baz OR boo)") );
		_TestSearchesRun(tc, &a,s, _T("((a OR b) AND NOT c) OR d") );
		_TestSearchesRun(tc, &a,s, _T("+(apple \"steve jobs\") -(foo bar baz)") );

		_TestSearchesRun(tc, &a,s, _T("+title:(dog OR cat) -author:\"bob dole\"") );


		_TestSearchesRun(tc, &a,s, _T(".*") );
		_TestSearchesRun(tc, &a,s, _T("<*") );
		_TestSearchesRun(tc, &a,s, _T("/*") );
		_TestSearchesRun(tc, &a,s, _T(";*") );
	}

void SearchTest(CuTest *tc, bool bram) {
	uint64_t start = Misc::currentTimeMillis();

	SimpleAnalyzer analyzer;

	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");
	Directory* ram = (bram?(Directory*)_CLNEW RAMDirectory():(Directory*)FSDirectory::getDirectory(fsdir) );

	IndexWriter writer( ram, &analyzer, true);
	writer.setUseCompoundFile(false);
  writer.setMaxBufferedDocs(3);

	const TCHAR* docs[] = { _T("a b c d e asdf"),
		_T("a b c d e a b c d e asdg"),
		_T("a b c d e f g h i j"),
		_T("a c e"),
		_T("e c a"),
		_T("a c e a c e asef"),
		_T("a c e a b c")
	};

	for (int j = 0; j < 7; j++) {
		Document* d = _CLNEW Document();
		//no need to delete fields... document takes ownership
		d->add(*_CLNEW Field(_T("contents"),docs[j],Field::STORE_YES | Field::INDEX_TOKENIZED));

		writer.addDocument(d);
		_CLDELETE(d);
	}
	writer.close();

	if (!bram){
		ram->close();
		_CLDECDELETE(ram);
		ram = (Directory*)FSDirectory::getDirectory(fsdir);
	}

	IndexReader* reader = IndexReader::open(ram);
	IndexSearcher searcher(reader);

	const TCHAR* queries[] = {
    _T("a AND NOT b"),
    _T("+a -b"),
		_T("\"a b\""),
		_T("\"a b c\""),
		_T("a AND b"),
		_T("a c"),
		_T("\"a c\""),
		_T("\"a c e\"")
	};
	int shouldbe[] = {3,3,4,4,4,7,3,3};
	Hits* hits = NULL;
	QueryParser parser(_T("contents"), &analyzer);

	for (int k = 0; k < 8; k++) {
		Query* query = parser.parse(queries[k]);

		TCHAR* qryInfo = query->toString(_T("contents"));
		hits = searcher.search(query);
		CLUCENE_ASSERT( hits->length() == shouldbe[k] );
		_CLDELETE_CARRAY(qryInfo);
		_CLDELETE(hits);
		_CLDELETE(query);
	}

  //test MultiPositionQuery...
  {
    MultiPhraseQuery* query = _CLNEW MultiPhraseQuery();
    RefCountArray<Term*> terms(3);
    Term* termE = _CLNEW Term(_T("contents"), _T("e"));
    terms[0] = _CLNEW Term(_T("contents"), _T("asdf"));
    terms[1] = _CLNEW Term(_T("contents"), _T("asdg"));
    terms[2] = _CLNEW Term(_T("contents"), _T("asef"));

    query->add(termE);
		_CLDECDELETE(termE);
    
    query->add(&terms);
    terms.deleteValues();

		TCHAR* qryInfo = query->toString(_T("contents"));
		hits = searcher.search(query);
		CLUCENE_ASSERT( hits->length() == 3 );
		_CLDELETE_CARRAY(qryInfo);
		_CLDELETE(hits);
		_CLDELETE(query);
  }
  
	searcher.close();
    reader->close();
	_CLDELETE( reader );

	ram->close();
	_CLDECDELETE(ram);

	CuMessageA (tc,"took %d milliseconds\n", (int32_t)(Misc::currentTimeMillis()-start));
}

void testNormEncoding(CuTest *tc) {
	//just a quick test of the default similarity
	CLUCENE_ASSERT(CL_NS(search)::Similarity::getDefault()->queryNorm(1)==1.0);

    float_t f = CL_NS(search)::Similarity::getDefault()->queryNorm(9);
    f -= (1.0/3.0);
    if ( f < 0 )
        f *= -1;
	CLUCENE_ASSERT(f < 0.1);

    //test that div by zero is handled
    float_t tmp = CL_NS(search)::Similarity::getDefault()->lengthNorm(_T("test"),0);
    tmp = CL_NS(search)::Similarity::getDefault()->queryNorm(0);

	//test that norm encoding is working properly
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(-1)==0 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(0)==0 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(1)==124 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(1)==124 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(7516192768.0 )==255);


	CLUCENE_ASSERT( CL_NS(search)::Similarity::decodeNorm(124)==1 );
	CLUCENE_ASSERT( CL_NS(search)::Similarity::decodeNorm(255)==7516192768.0 );

    //well know value:
    CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(0.5f) == 120 );

    //can decode self
    CLUCENE_ASSERT( CL_NS(search)::Similarity::encodeNorm(CL_NS(search)::Similarity::decodeNorm(57)) == 57 );
}

void testSrchManyHits(CuTest* /*tc*/) {
  SimpleAnalyzer analyzer;
	RAMDirectory ram;
	IndexWriter writer( &ram, &analyzer, true);

	const TCHAR* docs[] = { _T("a b c d e"),
		_T("a b c d e a b c d e"),
		_T("a b c d e f g h i j"),
		_T("a c e"),
		_T("e c a"),
		_T("a c e a c e"),
		_T("a c e a b c")
	};

	for (int j = 0; j < 140; j++) {
		Document* d = _CLNEW Document();
		//no need to delete fields... document takes ownership
		int x = j%7;
		d->add(*_CLNEW Field(_T("contents"),docs[x],Field::STORE_YES | Field::INDEX_TOKENIZED));

		writer.addDocument(d);
		_CLDELETE(d);
	}
	writer.close();

	IndexSearcher searcher(&ram);

	BooleanQuery query;
	Term* t = _CLNEW Term(_T("contents"), _T("a"));
	query.add(_CLNEW TermQuery(t),true,false, false);
	_CLDECDELETE(t);
	Hits* hits = searcher.search(&query);
	for ( size_t x=0;x<hits->length();x++ ){
	      hits->doc(x);
	}
	_CLDELETE(hits);
	searcher.close();
}

void testSrchMulti(CuTest *tc) {
  SimpleAnalyzer analyzer;
	RAMDirectory ram0;
	IndexWriter writer0( &ram0, &analyzer, true);

	const TCHAR* docs0[] = {
		_T("a")
	};

	Document* d = _CLNEW Document();
	//no need to delete fields... document takes ownership
	d->add(*_CLNEW Field(_T("contents"),docs0[0],Field::STORE_YES | Field::INDEX_TOKENIZED));

	writer0.addDocument(d);
	_CLDELETE(d);
	writer0.close();

	RAMDirectory ram1;
	IndexWriter writer1( &ram1, &analyzer, true);

	const TCHAR* docs1[] = {
		_T("e")
	};

	d = _CLNEW Document();
	//no need to delete fields... document takes ownership
	d->add(*_CLNEW Field(_T("contents"),docs1[0],Field::STORE_YES | Field::INDEX_TOKENIZED));

	writer1.addDocument(d);
	_CLDELETE(d);
	writer1.close();

	IndexSearcher searcher0(&ram0);
	IndexSearcher searcher1(&ram1);

	Searchable* searchers[3];

	searchers[0] = &searcher0;
	searchers[1] = &searcher1;
	searchers[2] = NULL;

	MultiSearcher searcher(searchers);

  Term* termA = _CLNEW Term(_T("contents"), _T("a"));
  Term* termC = _CLNEW Term(_T("contents"), _T("c"));
	RangeQuery query(termA, termC, true);
  _CLDECDELETE(termA);
  _CLDECDELETE(termC);

	Query* rewritten = searcher.rewrite(&query);
	Hits* hits = searcher.search(rewritten);
	for ( size_t x=0;x<hits->length();x++ ){
	  hits->doc(x);
	}
  CLUCENE_ASSERT(hits->length() == 1);
	if (&query != rewritten) {
		_CLDELETE(rewritten);
	}
	_CLDELETE(hits);
	searcher.close();
}

void ramSearchTest(CuTest *tc) { SearchTest(tc, true); }
void fsSearchTest(CuTest *tc) { SearchTest(tc, false); }

CuSuite *testsearch(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Search Test"));
  SUITE_ADD_TEST(suite, ramSearchTest);
	SUITE_ADD_TEST(suite, fsSearchTest);

	SUITE_ADD_TEST(suite, testNormEncoding);
	SUITE_ADD_TEST(suite, testSrchManyHits);
	SUITE_ADD_TEST(suite, testSrchMulti);
	SUITE_ADD_TEST(suite, testSrchOpenIndex);
	SUITE_ADD_TEST(suite, testSrchPunctuation);
	SUITE_ADD_TEST(suite, testSrchSlop);
	SUITE_ADD_TEST(suite, testSrchNumbers);
	SUITE_ADD_TEST(suite, testSrchWildcard);
	SUITE_ADD_TEST(suite, testSrchEscapes);
	SUITE_ADD_TEST(suite, testSrchRange);
	SUITE_ADD_TEST(suite, testSrchSimple);
	SUITE_ADD_TEST(suite, testSrchCloseIndex);

    return suite;
}
// EOF
