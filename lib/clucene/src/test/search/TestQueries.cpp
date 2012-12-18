/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/search/MultiPhraseQuery.h"
#include "QueryUtils.h"

/// Java PrefixQuery test, 2009-06-02
void testPrefixQuery(CuTest *tc){
	WhitespaceAnalyzer analyzer;
	RAMDirectory directory;
	const TCHAR* categories[] = {_T("/Computers"), _T("/Computers/Mac"), _T("/Computers/Windows")};

	IndexWriter writer( &directory, &analyzer, true);
	for (int i = 0; i < 3; i++) {
		Document *doc = _CLNEW Document();
		doc->add(*_CLNEW Field(_T("category"), categories[i], Field::STORE_YES | Field::INDEX_UNTOKENIZED));
		writer.addDocument(doc);
		_CLDELETE(doc);
	}
	writer.close();

	Term* t = _CLNEW Term(_T("category"), _T("/Computers"));
	PrefixQuery *query = _CLNEW PrefixQuery(t);
	IndexSearcher searcher(&directory);
	Hits *hits = searcher.search(query);
	CLUCENE_ASSERT(3 == hits->length()); // All documents in /Computers category and below
	_CLDELETE(query);
	_CLDELETE(t);
	_CLDELETE(hits);

	t = _CLNEW Term(_T("category"), _T("/Computers/Mac"));
	query = _CLNEW PrefixQuery(t);
	hits = searcher.search(query);
	CLUCENE_ASSERT(1 == hits->length()); // One in /Computers/Mac
	_CLDELETE(query);
	_CLDELETE(t);
	_CLDELETE(hits);
}

#ifndef NO_FUZZY_QUERY

/// Java FuzzyQuery test, 2009-06-02
class TestFuzzyQuery {
private:
	CuTest *tc;

	void addDoc(const TCHAR* text, IndexWriter* writer) {
		Document* doc = _CLNEW Document();
		doc->add(*_CLNEW Field(_T("field"), text, Field::STORE_YES | Field::INDEX_TOKENIZED));
		writer->addDocument(doc);
		_CLLDELETE(doc);
	}

	Hits* searchQuery(IndexSearcher* searcher, const TCHAR* field, const TCHAR* text,
		float_t minSimilarity=FuzzyQuery::defaultMinSimilarity, size_t prefixLen=0){

		Term* t = _CLNEW Term(field, text);
		FuzzyQuery* query = _CLNEW FuzzyQuery(t, minSimilarity, prefixLen);
		Hits* hits = searcher->search(query);
		_CLLDELETE(query);
		_CLLDECDELETE(t);
		return hits;
	}

	size_t getHitsLength(IndexSearcher* searcher, const TCHAR* field, const TCHAR* text,
		float_t minSimilarity=FuzzyQuery::defaultMinSimilarity, size_t prefixLen=0){

		Hits* hits = searchQuery(searcher, field, text, minSimilarity, prefixLen);
		size_t ret = hits->length();
		_CLLDELETE(hits);
		return ret;
	}
public:
	TestFuzzyQuery(CuTest *_tc):tc(_tc){
	}
	~TestFuzzyQuery(){
	}

	void testFuzziness() {
		RAMDirectory directory;
		WhitespaceAnalyzer a;
		IndexWriter writer(&directory, &a, true);
		addDoc(_T("aaaaa"), &writer);
		addDoc(_T("aaaab"), &writer);
		addDoc(_T("aaabb"), &writer);
		addDoc(_T("aabbb"), &writer);
		addDoc(_T("abbbb"), &writer);
		addDoc(_T("bbbbb"), &writer);
		addDoc(_T("ddddd"), &writer);
		writer.optimize();
		writer.close();
		IndexSearcher searcher(&directory);

		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa")) == 3);

		// same with prefix
		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa"),FuzzyQuery::defaultMinSimilarity,1) == 3);
		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa"),FuzzyQuery::defaultMinSimilarity,2) == 3);
		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa"),FuzzyQuery::defaultMinSimilarity,3) == 3);
		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa"),FuzzyQuery::defaultMinSimilarity,4) == 2);
		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa"),FuzzyQuery::defaultMinSimilarity,5) == 1);
		CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaaaa"),FuzzyQuery::defaultMinSimilarity,6) == 1);

		// not similar enough:
		CuAssertTrue(tc, getHitsLength(&searcher, _T("field"), _T("xxxxx")) == 0);
		CuAssertTrue(tc, getHitsLength(&searcher, _T("field"), _T("aaccc")) == 0); // edit distance to "aaaaa" = 3

		// query identical to a word in the index:
		Hits* hits = searchQuery(&searcher, _T("field"), _T("aaaaa"));
		CLUCENE_ASSERT( hits->length() == 3);
		CuAssertStrEquals(tc, NULL, _T("aaaaa"), hits->doc(0).get(_T("field")));
		// default allows for up to two edits:
		CuAssertStrEquals(tc, NULL, _T("aaaab"), hits->doc(1).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaabb"), hits->doc(2).get(_T("field")));
		_CLLDELETE(hits);

		// query similar to a word in the index:
		hits = searchQuery(&searcher, _T("field"), _T("aaaac"));
		CLUCENE_ASSERT( hits->length() == 3);
		CuAssertStrEquals(tc, NULL, _T("aaaaa"), hits->doc(0).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaaab"), hits->doc(1).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaabb"), hits->doc(2).get(_T("field")));
		_CLLDELETE(hits);

		// now with prefix
		hits = searchQuery(&searcher, _T("field"), _T("aaaac"), FuzzyQuery::defaultMinSimilarity, 1);
		CLUCENE_ASSERT( hits->length() == 3);
		CuAssertStrEquals(tc, NULL, _T("aaaaa"), hits->doc(0).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaaab"), hits->doc(1).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaabb"), hits->doc(2).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("aaaac"), FuzzyQuery::defaultMinSimilarity, 2);
		CLUCENE_ASSERT( hits->length() == 3);
		CuAssertStrEquals(tc, NULL, _T("aaaaa"), hits->doc(0).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaaab"), hits->doc(1).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaabb"), hits->doc(2).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("aaaac"), FuzzyQuery::defaultMinSimilarity, 3);
		CLUCENE_ASSERT( hits->length() == 3);
		CuAssertStrEquals(tc, NULL, _T("aaaaa"), hits->doc(0).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaaab"), hits->doc(1).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaabb"), hits->doc(2).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("aaaac"), FuzzyQuery::defaultMinSimilarity, 4);
		CLUCENE_ASSERT( hits->length() == 2);
		CuAssertStrEquals(tc, NULL, _T("aaaaa"), hits->doc(0).get(_T("field")));
		CuAssertStrEquals(tc, NULL, _T("aaaab"), hits->doc(1).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("aaaac"), FuzzyQuery::defaultMinSimilarity, 5);
		CLUCENE_ASSERT( hits->length() == 0);
		_CLLDELETE(hits);


		hits = searchQuery(&searcher, _T("field"), _T("ddddX"));
		CLUCENE_ASSERT( hits->length() == 1);
		CuAssertStrEquals(tc, NULL, _T("ddddd"), hits->doc(0).get(_T("field")));
		_CLLDELETE(hits);

		// now with prefix
		hits = searchQuery(&searcher, _T("field"), _T("ddddX"), FuzzyQuery::defaultMinSimilarity, 1);
		CLUCENE_ASSERT( hits->length() == 1);
		CuAssertStrEquals(tc, NULL, _T("ddddd"), hits->doc(0).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("ddddX"), FuzzyQuery::defaultMinSimilarity, 2);
		CLUCENE_ASSERT( hits->length() == 1);
		CuAssertStrEquals(tc, NULL, _T("ddddd"), hits->doc(0).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("ddddX"), FuzzyQuery::defaultMinSimilarity, 3);
		CLUCENE_ASSERT( hits->length() == 1);
		CuAssertStrEquals(tc, NULL, _T("ddddd"), hits->doc(0).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("ddddX"), FuzzyQuery::defaultMinSimilarity, 4);
		CLUCENE_ASSERT( hits->length() == 1);
		CuAssertStrEquals(tc, NULL, _T("ddddd"), hits->doc(0).get(_T("field")));
		_CLLDELETE(hits);

		hits = searchQuery(&searcher, _T("field"), _T("ddddX"), FuzzyQuery::defaultMinSimilarity, 5);
		CLUCENE_ASSERT( hits->length() == 0);
		_CLLDELETE(hits);

		// different field = no match:
		hits = searchQuery(&searcher, _T("anotherfield"), _T("ddddX"));
		CLUCENE_ASSERT( hits->length() == 0);
		_CLLDELETE(hits);

		searcher.close();
		directory.close();
	}

    void testFuzzinessLong() {
        RAMDirectory directory;
        WhitespaceAnalyzer a;
        IndexWriter writer(&directory, &a, true);
        addDoc(_T("aaaaaaa"), &writer);
        addDoc(_T("segment"), &writer);
        writer.optimize();
        writer.close();
        IndexSearcher searcher(&directory);
        
        // not similar enough:
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("xxxxx")) == 0);

        // edit distance to "aaaaaaa" = 3, this matches because the string is longer than
        // in testDefaultFuzziness so a bigger difference is allowed:
        Hits* hits = searchQuery(&searcher, _T("field"), _T("aaaaccc"));
        CLUCENE_ASSERT( hits->length() == 1);
        CuAssertStrEquals(tc, NULL, _T("aaaaaaa"), hits->doc(0).get(_T("field")));
        _CLLDELETE(hits);

        // now with prefix
        hits = searchQuery(&searcher, _T("field"), _T("aaaaccc"), FuzzyQuery::defaultMinSimilarity, 1);
        CLUCENE_ASSERT( hits->length() == 1);
        CuAssertStrEquals(tc, NULL, _T("aaaaaaa"), hits->doc(0).get(_T("field")));
        _CLLDELETE(hits);
        hits = searchQuery(&searcher, _T("field"), _T("aaaaccc"), FuzzyQuery::defaultMinSimilarity, 4);
        CLUCENE_ASSERT( hits->length() == 1);
        CuAssertStrEquals(tc, NULL, _T("aaaaaaa"), hits->doc(0).get(_T("field")));
        _CLLDELETE(hits);
        hits = searchQuery(&searcher, _T("field"), _T("aaaaccc"), FuzzyQuery::defaultMinSimilarity, 5);
        CLUCENE_ASSERT( hits->length() == 0);
        _CLLDELETE(hits);

        // no match, more than half of the characters is wrong:
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaacccc")) == 0);

        // now with prefix
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("aaacccc"), FuzzyQuery::defaultMinSimilarity, 2) == 0);

        // "student" and "stellent" are indeed similar to "segment" by default:
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("student")) == 1);
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("stellent")) == 1);

        // now with prefix
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("student"), FuzzyQuery::defaultMinSimilarity, 1) == 1);
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("stellent"), FuzzyQuery::defaultMinSimilarity, 1) == 1);
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("student"), FuzzyQuery::defaultMinSimilarity, 2) == 0);
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("stellent"), FuzzyQuery::defaultMinSimilarity, 2) == 0);

        // "student" doesn't match anymore thanks to increased minimum similarity:
        CLUCENE_ASSERT( getHitsLength(&searcher, _T("field"), _T("student"), 0.6f, 0) == 0);

        try {
            new FuzzyQuery(_CLNEW Term(_T("field"), _T("student")), 1.1f);
            CuFail(tc, _T("Expected IllegalArgumentException"));
        } catch (CLuceneError& /*e*/) {
            // expecting exception
        }
        try {
            new FuzzyQuery(_CLNEW Term(_T("field"), _T("student")), -0.1f);
            CuFail(tc, _T("Expected IllegalArgumentException"));
        } catch (CLuceneError& /*e*/) {
            // expecting exception
        }

        searcher.close();
        directory.close();
    }
};

void testFuzzyQuery(CuTest *tc){
	
	/// Run Java Lucene tests
	TestFuzzyQuery tester(tc);
	tester.testFuzziness();

	/// Legacy CLucene tests
	RAMDirectory ram;

	//---
	WhitespaceAnalyzer an;
	IndexWriter* writer = _CLNEW IndexWriter(&ram, &an, true);

	//---  
	Document *doc = 0;
	//****
	doc = _CLNEW Document();
	doc->add(*_CLNEW Field(_T("body"),_T("test"),Field::STORE_NO | Field::INDEX_TOKENIZED));
	writer->addDocument(doc);
	_CLDELETE(doc);
	//****
	doc = _CLNEW Document();
	doc->add(*_CLNEW Field(_T("body"),_T("home"),Field::STORE_NO | Field::INDEX_TOKENIZED));
	writer->addDocument(doc);
	_CLDELETE(doc);
	//****
	doc = _CLNEW Document();
	doc->add(*_CLNEW Field(_T("body"), _T("pc linux"),Field::STORE_NO | Field::INDEX_TOKENIZED));
	writer->addDocument(doc);
	_CLDELETE(doc);
	//****
	doc = _CLNEW Document();
	doc->add(*_CLNEW Field(_T("body"), _T("tested"),Field::STORE_NO | Field::INDEX_TOKENIZED));
	writer->addDocument(doc);
	_CLDELETE(doc);
	//****
	doc = _CLNEW Document();
	doc->add(*_CLNEW Field(_T("body"), _T("source"),Field::STORE_NO | Field::INDEX_TOKENIZED));
	writer->addDocument(doc);
	_CLDELETE(doc);

	//---
	writer->close();
	_CLDELETE(writer);

	//---
	IndexSearcher searcher (&ram);

	//---
	Term* term = _CLNEW Term(_T("body"), _T("test~"));
	Query* query = _CLNEW FuzzyQuery(term);
	Hits* result = searcher.search(query);

	CLUCENE_ASSERT(result && result->length() > 0);

	//---
	_CLDELETE(result);
	_CLDELETE(query);
	_CLDECDELETE(term);
	searcher.close();
	ram.close();
}
#else
	void _NO_FUZZY_QUERY(CuTest *tc){
		CuNotImpl(tc,_T("Fuzzy"));
	}
#endif

void testMultiPhraseQuery( CuTest * tc )
{
    MultiPhraseQuery * pQuery = _CLNEW MultiPhraseQuery();

    Term * t1 = _CLNEW Term( _T( "field" ), _T( "t1" ));
    Term * t2 = _CLNEW Term( _T( "field" ), _T( "t2" ));
    Term * t3 = _CLNEW Term( _T( "field" ), _T( "t3" ));
    Term * t4 = _CLNEW Term( _T( "field" ), _T( "t4" ));
    
    CL_NS(util)::ValueArray<CL_NS(index)::Term*> terms( 3 );
    terms[ 0 ] = t2;
    terms[ 1 ] = t3;
    terms[ 2 ] = t4;

    pQuery->add( t1 );
    pQuery->add( &terms );

    Query * pClone = pQuery->clone();
    
    QueryUtils::checkEqual( tc, pQuery, pClone );

    _CLLDECDELETE( t1 );
    _CLLDECDELETE( t2 );
    _CLLDECDELETE( t3 );
    _CLLDECDELETE( t4 );
    _CLLDELETE( pQuery );
    _CLLDELETE( pClone );
}

CuSuite *testqueries(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Queries Test"));

	SUITE_ADD_TEST(suite, testPrefixQuery);
	SUITE_ADD_TEST(suite, testMultiPhraseQuery);
	#ifndef NO_FUZZY_QUERY
		SUITE_ADD_TEST(suite, testFuzzyQuery);
	#else
		SUITE_ADD_TEST(suite, _NO_FUZZY_QUERY);
	#endif


	return suite; 
}
//EOF

