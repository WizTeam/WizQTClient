/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
/**
 * Unit tests for sorting code.
 *
 */

Searcher* sort_full;
Searcher* sort_searchX;
Searcher* sort_searchY;
Query* sort_queryX;
Query* sort_queryY;
Query* sort_queryA;
Query* sort_queryF;
Sort* _sort;
SimpleAnalyzer sort_analyser;

typedef StringMap<TCHAR*, float_t> sortScores;
typedef std::pair<TCHAR*,float_t> scorePair;

// document data:
// the tracer field is used to determine which document was hit
// the contents field is used to search and _sort by relevance
// the int field to _sort by int
// the float field to _sort by float
// the string field to _sort by string
const TCHAR* data[11][6] = {
	// tracer       contents            int                 float               string      custom
	{   _T("A"),   _T("x a"),           _T("5"),           _T("4f"),           _T("c"),     _T("A-3")   },
	{   _T("B"),   _T("y a"),           _T("5"),           _T("3.4028235E38"), _T("i"),     _T("B-10")  },
	{   _T("C"),   _T("x a b c"),       _T("2147483647"),  _T("1.0"),          _T("j"),     _T("A-2")   },
	{   _T("D"),   _T("y a b c"),       _T("-1"),          _T("0.0f"),         _T("a"),     _T("C-0")   },
	{   _T("E"),   _T("x a b c d"),     _T("5"),           _T("2f"),           _T("h"),     _T("B-8")   },
	{   _T("F"),   _T("y a b c d"),     _T("2"),           _T("3.14159f"),     _T("g"),     _T("B-1")   },
	{   _T("G"),   _T("x a b c d"),     _T("3"),           _T("-1.0"),         _T("f"),     _T("C-100") },
	{   _T("H"),   _T("y a b c d"),     _T("0"),           _T("1.4E-45"),      _T("e"),     _T("C-88")  },
	{   _T("I"),   _T("x a b c d e f"), _T("-2147483648"), _T("1.0e+0"),       _T("d"),     _T("A-10")  },
	{   _T("J"),   _T("y a b c d e f"), _T("4"),           _T(".5"),           _T("b"),     _T("C-7")   },
	{   _T("Z"),   _T("f"),             NULL,              NULL,              NULL,         NULL    }
	};

Searcher* sort_getIndex (bool even, bool odd){
	RAMDirectory* indexStore = _CLNEW RAMDirectory;
	IndexWriter writer(indexStore, &sort_analyser, true);
	for (int i=0; i<11; ++i) {
		if (((i%2)==0 && even) || ((i%2)==1 && odd)) {
			Document doc;
			doc.add (*_CLNEW Field ( _T("tracer"),   data[i][0], Field::STORE_YES));
			doc.add (*_CLNEW Field ( _T("contents"), data[i][1], Field::INDEX_TOKENIZED));
			if (data[i][2] != NULL)
                doc.add (*_CLNEW Field (_T("int"),   data[i][2], Field::INDEX_UNTOKENIZED));
			if (data[i][3] != NULL)
                doc.add (*_CLNEW Field (_T("float"),    data[i][3], Field::INDEX_UNTOKENIZED));
			if (data[i][4] != NULL)
                doc.add (*_CLNEW Field (_T("string"),   data[i][4], Field::INDEX_UNTOKENIZED));
			if (data[i][5] != NULL)
                doc.add (*_CLNEW Field (_T("custom"),   data[i][5], Field::INDEX_UNTOKENIZED));
			writer.addDocument (&doc);
		}
	}
	writer.close ();
	IndexSearcher* res = _CLNEW IndexSearcher(indexStore);
	_CLDECDELETE(indexStore);
	return res;
}

void testSortSetup(CuTest* /*tc*/) {
    sort_full = sort_getIndex (true, true);
	sort_searchX = sort_getIndex (true, false);
	sort_searchY = sort_getIndex (false, true);

    Term* tmp;

    tmp = _CLNEW Term (_T("contents"), _T("x"));
	sort_queryX = _CLNEW TermQuery (tmp);
    _CLDECDELETE(tmp);

    tmp = _CLNEW Term (_T("contents"), _T("y"));
	sort_queryY = _CLNEW TermQuery (tmp);
    _CLDECDELETE(tmp);

    tmp = _CLNEW Term (_T("contents"), _T("a"));
	sort_queryA = _CLNEW TermQuery (tmp);
    _CLDECDELETE(tmp);

    tmp = _CLNEW Term (_T("contents"), _T("f"));
	sort_queryF = _CLNEW TermQuery (tmp);
    _CLDECDELETE(tmp);

	_sort   = _CLNEW Sort();
}

void testSortCleanup(CuTest* /*tc*/) {
    _CLDELETE(sort_full);
    _CLDELETE(sort_searchX);
    _CLDELETE(sort_searchY);
    _CLDELETE(sort_queryX);
    _CLDELETE(sort_queryY);
    _CLDELETE(sort_queryA);
    _CLDELETE(sort_queryF);
    _CLDELETE(_sort);
}

// make sure the documents returned by the search match the expected list
void sortMatches (CuTest *tc, Searcher* searcher, Query* query, Sort* sort, const TCHAR* expectedResult){
	Hits* result = searcher->search (query, sort);
	StringBuffer buff(10);
	int32_t n = result->length();
	for (int i=0; i<n; ++i) {
		Document& doc = result->doc(i);
		TCHAR** v = doc.getValues(_T("tracer")); //todo: should be const???
		for (int j=0; v[j]!=NULL; ++j) {
			buff.append (v[j]);
		}
        _CLDELETE_CARRAY_ALL(v);
	}
	CuAssertStrEquals (tc, _T("tracer value"), expectedResult, buff.getBuffer());

    _CLDELETE(result);
}
void sortSameValues (CuTest* tc, sortScores* m1, sortScores* m2, bool deleteM1=false, bool deleteM2=true) {
	CuAssertIntEquals (tc, _T("sortScores size not equal"),m1->size(), m2->size());
	sortScores::iterator iter = m1->begin();
	float_t m=pow(10.0,-8);
	while (iter != m1->end()) {
		TCHAR* key = iter->first;

		sortScores::iterator i1 = m1->find(key);
		sortScores::iterator i2 = m2->find(key);

		float_t f1 = i1->second;
		float_t f2 = i2->second;

		float_t diff = f1-f2;
		if ( diff < 0 )
		    diff *= -1;
		if ( diff>m )
			CuAssert(tc,_T("sortSameValue f1!=f2"),false);
		iter++;
	}

	if ( deleteM1 )
		_CLDELETE(m1);
	if ( deleteM2 )
		_CLDELETE(m2);
}
// make sure the documents returned by the search match the expected list pattern
void sortMatchesPattern (CuTest* tc, Searcher* searcher, Query* query, Sort* sort, const TCHAR* pattern){
	Hits* result = searcher->search (query, sort);
	int32_t n = result->length();
	StringBuffer buff;
	for (int i=0; i<n; ++i) {
		Document& doc = result->doc(i);
		TCHAR** v = doc.getValues(_T("tracer"));
		for (int j=0; v[j]!=NULL; ++j) {
			buff.append (v[j]);
		}
		_CLDELETE_CARRAY_ALL(v);
	}

	//todo:need to actually test these matches... don't have regexp...
	CuMessage(tc,_T("\nmatching \"%s\" against pattern \"%s\""),buff.getBuffer(),pattern);
	// System.out.println ("matching \""+buff+"\" against pattern \""+pattern+"\"");
	//assertTrue (Pattern.compile(pattern).matcher(buff.toString()).matches());

	_CLDELETE(result);
}

// runs a variety of sorts useful for multisearchers
void sort_runMultiSorts (CuTest* tc, Searcher* multi) {
	_sort->setSort (SortField::FIELD_DOC());
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("[AB]{2}[CD]{2}[EF]{2}[GH]{2}[IJ]{2}"));

	_sort->setSort (_CLNEW SortField (_T("int"), SortField::INT, false));
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("IDHFGJ[ABE]{3}C"));

	SortField* sorts1[3]= {_CLNEW SortField (_T("int"), SortField::INT, false), SortField::FIELD_DOC(), NULL};
	_sort->setSort ( sorts1 );
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("IDHFGJ[AB]{2}EC"));

	_sort->setSort (_T("int"));
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("IDHFGJ[AB]{2}EC"));

	SortField* sorts2[3] = { _CLNEW SortField (_T("float"), SortField::FLOAT, false), SortField::FIELD_DOC(), NULL};
	_sort->setSort (sorts2);
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("GDHJ[CI]{2}EFAB"));

	_sort->setSort (_T("float"));
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("GDHJ[CI]{2}EFAB"));

	_sort->setSort (_T("string"));
	sortMatches (tc, multi, sort_queryA, _sort, _T("DJAIHGFEBC"));

	_sort->setSort (_T("int"), true);
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("C[AB]{2}EJGFHDI"));

	_sort->setSort (_T("float"), true);
	sortMatchesPattern (tc, multi, sort_queryA, _sort, _T("BAFE[IC]{2}JHDG"));

	_sort->setSort (_T("string"), true);
	sortMatches (tc, multi, sort_queryA, _sort, _T("CBEFGHIAJD"));

	//_sort->setSort (_CLNEW SortField[] { _CLNEW SortField (_T("string"), Locale.US) });
	//sortMatches (tc, multi, sort_queryA, _sort, _T("DJAIHGFEBC"));

	//_sort->setSort (_CLNEW SortField[] { _CLNEW SortField (_T("string"), Locale.US, true) });
	//sortMatches (tc, multi, sort_queryA, _sort, _T("CBEFGHIAJD"));

	const TCHAR* sorts3[3] = {_T("int"),_T("float"),NULL};
	_sort->setSort ( sorts3 );
	sortMatches (tc, multi, sort_queryA, _sort, _T("IDHFGJEABC"));

	const TCHAR* sorts4[3] = {_T("float"),_T("string"),NULL};
	_sort->setSort (sorts4);
	sortMatches (tc, multi, sort_queryA, _sort, _T("GDHJICEFAB"));

	_sort->setSort (_T("int"));
	sortMatches (tc, multi, sort_queryF, _sort, _T("IZJ"));

	_sort->setSort (_T("int"), true);
	sortMatches (tc, multi, sort_queryF, _sort, _T("JZI"));

	_sort->setSort (_T("float"));
	sortMatches (tc, multi, sort_queryF, _sort, _T("ZJI"));

	_sort->setSort (_T("string"));
	sortMatches (tc, multi, sort_queryF, _sort, _T("ZJI"));

	_sort->setSort (_T("string"), true);
	sortMatches (tc, multi, sort_queryF, _sort, _T("IJZ"));
}

 sortScores* sort_getScores (CuTest* tc, Hits* hits, bool deleteHits=true){
	sortScores* scoreMap = _CLNEW sortScores(true);
	int n = hits->length();
	float_t m=pow(10.0,-8);

	for (int i=0; i<n; ++i) {
		Document& doc = hits->doc(i);
		TCHAR** v = doc.getValues( _T("tracer"));

		int vLength=0;
		while(v[vLength]!=NULL)
			vLength++;

		CuAssertIntEquals (tc, _T("tracer values"), vLength, 1);

		if ( scoreMap->find(v[0]) != scoreMap->end () ){
			//this (should) be a multi search... the document will be double, so here we check that
			//the existing value is the same as this value... and then delete and ignore it.
			float_t diff = scoreMap->find(v[0])->second - hits->score(i);
			if ( diff < 0 )
				diff *= -1;
			if ( diff>m )
				CuAssert(tc,_T("sort_getScores(multi or incorrect) f1!=f2"),false);

			_CLDELETE_CARRAY_ALL(v);
		}else{
			scoreMap->insert ( scorePair(v[0], hits->score(i)) );
			_CLDELETE_ARRAY(v);
		}
	}
	if ( deleteHits )
		_CLDELETE(hits);
	return scoreMap;
}



//////////////////////////////////////////////////////////////////////
// The actual tests
//////////////////////////////////////////////////////////////////////
void testBuiltInSorts(CuTest *tc) {
	_CLDELETE(_sort);
    _sort = _CLNEW Sort();
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("ACEGI"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("BDFHJ"));

    _sort->setSort(SortField::FIELD_DOC());
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("ACEGI"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("BDFHJ"));
}

// test sorts where the type of field is specified
void testTypedSort(CuTest *tc){
    SortField* sorts1[3] = { _CLNEW SortField (_T("int"), SortField::INT,false), SortField::FIELD_DOC(), NULL };
	_sort->setSort (sorts1);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("IGAEC"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("DHFJB"));

    SortField* sorts2[3] = { _CLNEW SortField (_T("float"), SortField::FLOAT,false), SortField::FIELD_DOC(), NULL };
	_sort->setSort (sorts2);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("GCIEA"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("DHJFB"));

	SortField* sorts3[3] = { _CLNEW SortField (_T("string"), SortField::STRING,false), SortField::FIELD_DOC(), NULL };
	_sort->setSort (sorts3);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("AIGEC"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("DJHFB"));
}

// test sorts when there's nothing in the index
void testEmptyIndex(CuTest *tc) {
	Searcher* empty = sort_getIndex(false,false);

    _CLDELETE(_sort);
	_sort = _CLNEW Sort();
	sortMatches (tc, empty, sort_queryX, _sort, _T(""));

    _sort->setSort(SortField::FIELD_DOC());
	sortMatches (tc, empty, sort_queryX, _sort, _T(""));

	SortField* sorts1[3] = { _CLNEW SortField (_T("int"), SortField::INT,false), SortField::FIELD_DOC(), NULL };
	_sort->setSort (sorts1);
	sortMatches (tc, empty, sort_queryX, _sort, _T(""));

	SortField* sorts3[3] = { _CLNEW SortField (_T("string"), SortField::STRING,false), SortField::FIELD_DOC(), NULL };
	_sort->setSort (sorts3);
	sortMatches (tc, empty, sort_queryX, _sort, _T(""));

	SortField* sorts2[3] = { _CLNEW SortField (_T("float"), SortField::FLOAT,false), SortField::FIELD_DOC(), NULL };
	_sort->setSort (sorts2);
	sortMatches (tc, empty, sort_queryX, _sort, _T(""));

    _CLDELETE(empty);
}

// test sorts where the type of field is determined dynamically
void testAutoSort(CuTest *tc) {
	_sort->setSort(_T("int"));
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("IGAEC"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("DHFJB"));

	_sort->setSort(_T("float"));
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("GCIEA"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("DHJFB"));

	_sort->setSort(_T("string"));
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("AIGEC"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("DJHFB"));
}

// test sorts in reverse
void testReverseSort(CuTest *tc){
	/*SortField* sorts[3] = { _CLNEW SortField (NULL, SortField::INT,true), SortField::FIELD_DOC, NULL };
	_sort->setSort (sorts);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("IEGCA"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("JFHDB"));*/

	_sort->setSort (_CLNEW SortField (NULL, SortField::DOC, true));
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("IGECA"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("JHFDB"));

	_sort->setSort (_T("int"), true);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("CAEGI"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("BJFHD"));

	_sort->setSort (_T("float"), true);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("AECIG"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("BFJHD"));

	_sort->setSort (_T("string"), true);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("CEGIA"));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("BFHJD"));
}

void testEmptyFieldSort(CuTest *tc) {
	_sort->setSort ( _T("string"));
	sortMatches (tc, sort_full, sort_queryF, _sort, _T("ZJI"));

	_sort->setSort ( _T("string"), true);
	sortMatches (tc, sort_full, sort_queryF, _sort, _T("IJZ"));

	_sort->setSort ( _T("int"));
	sortMatches (tc, sort_full, sort_queryF, _sort, _T("IZJ"));

	_sort->setSort ( _T("int"), true);
	sortMatches (tc, sort_full, sort_queryF, _sort, _T("JZI"));

	_sort->setSort ( _T("float"));
	sortMatches (tc, sort_full, sort_queryF, _sort, _T("ZJI"));

	_sort->setSort ( _T("float"), true);
	sortMatches (tc, sort_full, sort_queryF, _sort, _T("IJZ"));
}


// test sorts using a series of fields
void testSortCombos(CuTest *tc) {
	const TCHAR* sorts1[3]= {_T("int"),_T("float"), NULL};
	_sort->setSort ( sorts1 );
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("IGEAC"));

	SortField* sorts2[3] = { _CLNEW SortField (_T("int"), SortField::AUTO, true),
		_CLNEW SortField (NULL, SortField::DOC, true), NULL };
	_sort->setSort ( sorts2);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("CEAGI"));

	const TCHAR* sorts3[3]= {_T("float"),_T("string"), NULL};
	_sort->setSort (sorts3);
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("GICEA"));
}

// test a custom _sort function
/*void testCustomSorts(CuTest *tc) {
	_sort->setSort (_CLNEW SortField (_T("custom"), SampleComparable.getComparatorSource()));
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("CAIEG"));

	_sort->setSort (_CLNEW SortField (_T("custom"), SampleComparable.getComparatorSource(), true));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("HJDBF"));

	SortComparator custom = SampleComparable.getComparator();
	_sort->setSort (_CLNEW SortField (_T("custom"), custom));
	sortMatches (tc, sort_full, sort_queryX, _sort, _T("CAIEG"));

	_sort->setSort (_CLNEW SortField (_T("custom"), custom, true));
	sortMatches (tc, sort_full, sort_queryY, _sort, _T("HJDBF"));
}*/

// test a variety of sorts using more than one searcher
void testMultiSort(CuTest *tc) {
	Searchable* searchables[3] ={ sort_searchX, sort_searchY, NULL };
	MultiSearcher searcher(searchables);

	sort_runMultiSorts (tc, &searcher);
	searcher.close();
}


// test that the relevancy scores are the same even if
// hits are sorted
void testNormalizedScores(CuTest *tc) {

	// capture relevancy scores
	sortScores* scoresX = sort_getScores (tc, sort_full->search (sort_queryX));
	sortScores* scoresY = sort_getScores (tc, sort_full->search (sort_queryY));
	sortScores* scoresA = sort_getScores (tc, sort_full->search (sort_queryA));

	// we'll test searching locally, remote and multi
	// note: the multi test depends on each separate index containing
	// the same documents as our local index, so the computed normalization
	// will be the same.  so we make a multi searcher over two equal document
	// sets - not realistic, but necessary for testing.
	//MultiSearcher remote(_CLNEW Searchable[] { getRemote() });
	Searchable* searchables[3] = { sort_full, sort_full,NULL };
	MultiSearcher multi( searchables );

	// change sorting and make sure relevancy stays the same

	_CLDELETE(_sort);
	_sort = _CLNEW Sort();
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	_sort->setSort(SortField::FIELD_DOC());
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	_sort->setSort (_T("int"));
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	_sort->setSort (_T("float"));
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	_sort->setSort (_T("string"));
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	const TCHAR* sorts1[3] = { _T("int"),_T("float"),NULL};
	_sort->setSort ( sorts1 );
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	SortField* sorts2[3] = {  _CLNEW SortField (_T("int"), SortField::AUTO, true), _CLNEW SortField (NULL, SortField::DOC, true), NULL };
	_sort->setSort ( sorts2 );
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	const TCHAR* sorts3[3] = { _T("float"),_T("string"),NULL};
	_sort->setSort (sorts3);
	sortSameValues (tc, scoresX, sort_getScores(tc, sort_full->search(sort_queryX,_sort)));
	sortSameValues (tc, scoresX, sort_getScores(tc, multi.search(sort_queryX,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, sort_full->search(sort_queryY,_sort)));
	sortSameValues (tc, scoresY, sort_getScores(tc, multi.search(sort_queryY,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, sort_full->search(sort_queryA,_sort)));
	sortSameValues (tc, scoresA, sort_getScores(tc, multi.search(sort_queryA,_sort)));

	_CLDELETE(scoresX);
	_CLDELETE(scoresY);
	_CLDELETE(scoresA);
}

CuSuite *testsort(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Sort Test"));
    SUITE_ADD_TEST(suite, testSortSetup);

    SUITE_ADD_TEST(suite, testBuiltInSorts);
    SUITE_ADD_TEST(suite, testTypedSort);
    SUITE_ADD_TEST(suite, testEmptyIndex);
    SUITE_ADD_TEST(suite, testAutoSort);
	SUITE_ADD_TEST(suite, testEmptyFieldSort);
	SUITE_ADD_TEST(suite, testSortCombos);
	//SUITE_ADD_TEST(suite, testCustomSorts);
	SUITE_ADD_TEST(suite, testMultiSort);
	SUITE_ADD_TEST(suite, testNormalizedScores);
	SUITE_ADD_TEST(suite, testReverseSort);

    SUITE_ADD_TEST(suite, testSortCleanup);
    return suite;
}
// EOF
