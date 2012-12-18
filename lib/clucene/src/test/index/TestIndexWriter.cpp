/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include <CLucene/search/MatchAllDocsQuery.h>
#include <stdio.h>

//checks if a merged index finds phrases correctly
void testIWmergePhraseSegments(CuTest *tc){
	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir, CL_MAX_PATH, "%s/%s",cl_tempDir, "test.indexwriter");
	SimpleAnalyzer a;
  Directory* dir = FSDirectory::getDirectory(fsdir);

	IndexWriter ndx2(dir,&a,true);
	ndx2.setUseCompoundFile(false);
	Document doc0;
	doc0.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value0 value1"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx2.addDocument(&doc0);
	ndx2.optimize();
	ndx2.close();

	IndexWriter ndx(fsdir,&a,false);
	ndx.setUseCompoundFile(false);
	Document doc1;
	doc1.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value1 value0"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx.addDocument(&doc1);
	ndx.optimize();
	ndx.close();

	//test the index querying
	IndexSearcher searcher(fsdir);
	Query* query0 = QueryParser::parse(
		_T("\"value0 value1\""),
		_T("field0"),
		&a
	);
	Hits* hits0 = searcher.search(query0);
	CLUCENE_ASSERT(hits0->length() > 0);
	Query* query1 = QueryParser::parse(
		_T("\"value1 value0\""),
		_T("field0"),
		&a
	);
	Hits* hits1 = searcher.search(query1);
	CLUCENE_ASSERT(hits1->length() > 0);
	_CLDELETE(query0);
	_CLDELETE(query1);
	_CLDELETE(hits0);
	_CLDELETE(hits1);
	_CLDECDELETE(dir);
}

//checks that adding more than the min_merge value goes ok...
//checks for a mem leak that used to occur
void testIWmergeSegments1(CuTest *tc){
	RAMDirectory ram;
	SimpleAnalyzer a;

    IndexWriter ndx2(&ram,&a,true);
	ndx2.close();                   //test immediate closing bug reported

	IndexWriter ndx(&ram,&a,true);  //set create to false

	ndx.setUseCompoundFile(false);
	ndx.setMergeFactor(2);
	TCHAR fld[1000];
	for ( int i=0;i<1000;i++ ){
    English::IntToEnglish(i,fld,1000);

		Document doc;

		doc.add ( *_CLNEW Field(_T("field0"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		doc.add ( *_CLNEW Field(_T("field1"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		doc.add ( *_CLNEW Field(_T("field2"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		doc.add ( *_CLNEW Field(_T("field3"),fld,Field::STORE_YES | Field::INDEX_TOKENIZED) );
		ndx.addDocument(&doc);
	}
	//ndx.optimize(); //optimize so we can read terminfosreader with segmentreader
	ndx.close();

	//test the ram loading
	RAMDirectory ram2(&ram);
	IndexReader* reader2 = IndexReader::open(&ram2);
	Term* term = _CLNEW Term(_T("field0"),fld);
	TermEnum* en = reader2->terms(term);
	CLUCENE_ASSERT(en->next());
	_CLDELETE(en);
	_CLDECDELETE(term);
	_CLDELETE(reader2);
}

//checks if appending to an index works correctly
void testIWmergeSegments2(CuTest *tc){
	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir, CL_MAX_PATH, "%s/%s",cl_tempDir, "test.indexwriter");
	SimpleAnalyzer a;
  Directory* dir = FSDirectory::getDirectory(fsdir);

	IndexWriter ndx2(dir,&a,true);
	ndx2.setUseCompoundFile(false);
	Document doc0;
	doc0.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value0"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx2.addDocument(&doc0);
	ndx2.optimize();
	ndx2.close();

	IndexWriter ndx(fsdir,&a,false);
	ndx.setUseCompoundFile(false);
	Document doc1;
	doc1.add(
		*_CLNEW Field(
			_T("field0"),
			_T("value1"),
			Field::STORE_YES | Field::INDEX_TOKENIZED
		)
	);
	ndx.addDocument(&doc1);
	ndx.optimize();
	ndx.close();

	//test the ram querying
	IndexSearcher searcher(fsdir);
	Term* term0 = _CLNEW Term(_T("field0"),_T("value1"));
	Query* query0 = QueryParser::parse(_T("value0"),_T("field0"),&a);
	Hits* hits0 = searcher.search(query0);
	CLUCENE_ASSERT(hits0->length() > 0);
	Term* term1 = _CLNEW Term(_T("field0"),_T("value0"));
	Query* query1 = QueryParser::parse(_T("value1"),_T("field0"),&a);
	Hits* hits1 = searcher.search(query1);
	CLUCENE_ASSERT(hits1->length() > 0);
	_CLDELETE(query0);
	_CLDELETE(query1);
	_CLDELETE(hits0);
  _CLDELETE(hits1);
	_CLDECDELETE(term0);
	_CLDECDELETE(term1);
    dir->close();
    _CLDECDELETE(dir);
}

void testAddIndexes(CuTest *tc){
	char reuters_origdirectory[1024];
  strcpy(reuters_origdirectory, clucene_data_location);
  strcat(reuters_origdirectory, "/reuters-21578-index");

  {
    RAMDirectory dir;
    WhitespaceAnalyzer a;
    IndexWriter w(&dir, &a, true);
    ValueArray<Directory*> dirs(2);
    dirs[0] = FSDirectory::getDirectory(reuters_origdirectory);
    dirs[1] = FSDirectory::getDirectory(reuters_origdirectory);
    w.addIndexesNoOptimize(dirs);
    w.flush();
    CLUCENE_ASSERT(w.docCount()==62); //31 docs in reuters...

    // TODO: Currently there is a double ref-counting mechanism in place for Directory objects,
    //      so we need to dec them both
    dirs[1]->close();_CLDECDELETE(dirs[1]);
    dirs[0]->close();_CLDECDELETE(dirs[0]);
  }
  {
    RAMDirectory dir;
    WhitespaceAnalyzer a;
    IndexWriter w(&dir, &a, true);
    ValueArray<Directory*> dirs(2);
    dirs[0] = FSDirectory::getDirectory(reuters_origdirectory);
    dirs[1] = FSDirectory::getDirectory(reuters_origdirectory);
    w.addIndexes(dirs);
    w.flush();
    CLUCENE_ASSERT(w.docCount()==62); //31 docs in reuters...

    // TODO: Currently there is a double ref-counting mechanism in place for Directory objects,
    //      so we need to dec them both
    dirs[1]->close();_CLDECDELETE(dirs[1]);
    dirs[0]->close();_CLDECDELETE(dirs[0]);
  }
}

void testHashingBug(CuTest* /*tc*/){
  //Manuel Freiholz's indexing bug

  CL_NS(document)::Document doc;
  CL_NS(document)::Field* field;
  CL_NS(analysis::standard)::StandardAnalyzer analyzer;
  CL_NS(store)::RAMDirectory dir;
  CL_NS(index)::IndexWriter writer(&dir, &analyzer, true, true );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VERSION"), _T("1"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_PID"), _T("5"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_DATE"), _T("20090722"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_SEARCHDATA"), _T("all kind of data"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_TOKENIZED );
  doc.add( (*field) );

  writer.addDocument( &doc ); // ADDING FIRST DOCUMENT. -> this works!

  doc.clear();

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VERSION"), _T("1"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_PID"), _T("5"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_LINEID"), _T("20"), CL_NS(document)::Field::STORE_YES | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VT_ORDER"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_H"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_HF"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_D"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_OD"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_P1"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) );

  field = _CLNEW CL_NS(document)::Field( _T("CNS_VN_H1"), _T("456033000"), CL_NS(document)::Field::STORE_NO | CL_NS(document)::Field::INDEX_UNTOKENIZED );
  doc.add( (*field) ); // the problematic field!

  writer.addDocument( &doc ); // ADDING SECOND DOCUMENT - will never return from this function
  writer.optimize();          // stucks in line 222-223
  writer.close();
  _CL_LDECREF(&dir);
}

class IWlargeScaleCorrectness_tester {
public:
	void invoke(Directory& storage, CuTest *tc);
};

void IWlargeScaleCorrectness_tester::invoke(
	Directory& storage,
	CuTest *tc
){
	SimpleAnalyzer a;

	IndexWriter* ndx = _CLNEW IndexWriter(&storage,&a,true);

	ndx->setUseCompoundFile(false);

	const long documents = 200;
	const long step = 23;
	const long inverted_step = 113;
	const long repetitions = 5;

	CLUCENE_ASSERT(0 == (step * inverted_step + 1) % documents);

	long value0;
	long value1 = 0;

	long block_size = 1;
	long reopen = 1;

	for (value0 = 0; value0 < documents * repetitions; value0++) {
		if (reopen == value0) {
			ndx->optimize();
			ndx->close();
			_CLDELETE(ndx);
			ndx = _CLNEW IndexWriter(&storage,&a,false);
			ndx->setUseCompoundFile(false);
			reopen += block_size;
			block_size++;
		}

		TCHAR* value0_string = NumberTools::longToString(value0 % documents);
		TCHAR* value1_string = NumberTools::longToString(value1);

		Document doc;
		
		doc.add (
			*_CLNEW Field(
				_T("field0"),
				value0_string,
				Field::STORE_YES | Field::INDEX_UNTOKENIZED
			)
		);
		doc.add (
			*_CLNEW Field(
				_T("field1"),
				value1_string,
				Field::STORE_YES | Field::INDEX_UNTOKENIZED
			)
		);
		ndx->addDocument(&doc);

		_CLDELETE_ARRAY(value0_string);
		_CLDELETE_ARRAY(value1_string);
		value1 = (value1 + step) % documents;
	}

	ndx->optimize();
	ndx->close();

	IndexSearcher searcher(&storage);
	Query* query0 = _CLNEW MatchAllDocsQuery;
	Sort by_value1(
		_CLNEW SortField(
			_T("field1"),
			SortField::STRING,
			true
		)
	);
	Hits* hits0 = searcher.search(query0, &by_value1);
	long last = 0;
	for (long i = 0; i < hits0->length(); i++) {
		Document& retrieved = hits0->doc(i);
		TCHAR const* value = retrieved.get(_T("field0"));
		long current = NumberTools::stringToLong(value);
		long delta = (current + documents - last) % documents;
		if (0 == (i % repetitions)) {
			CLUCENE_ASSERT(inverted_step == delta);
		} else {
			CLUCENE_ASSERT(0 == delta);
		}
		last = current;
	}
	_CLDELETE(query0);
	_CLDELETE(hits0);
	_CLDELETE(ndx);
}

void testIWlargeScaleCorrectness(CuTest *tc){
	char fsdir[CL_MAX_PATH];
	_snprintf(fsdir,CL_MAX_PATH,"%s/%s",cl_tempDir, "test.search");
	RAMDirectory ram;
	FSDirectory* disk = FSDirectory::getDirectory(fsdir);
	IWlargeScaleCorrectness_tester().invoke(ram, tc);
	IWlargeScaleCorrectness_tester().invoke(*disk, tc);
	disk->close();
	_CLDECDELETE(disk);
}

void testExceptionFromTokenStream(CuTest *tc) {

    class TokenFilterWithException : public TokenFilter
    {
    private:
        int count;

    public:
        TokenFilterWithException(TokenStream * in) : 
          TokenFilter(in, true), count(0) {};

          Token* next(Token * pToken) {
              if (count++ == 5) {
                  _CLTHROWA(CL_ERR_IO, "TokenFilterWithException testing IO exception");         
              }
              return input->next(pToken);
          };
    };

    class AnalyzerWithException : public Analyzer
    {
        TokenStream* lastStream;
    public:
        AnalyzerWithException() { lastStream = NULL; }
        virtual ~AnalyzerWithException() { _CLDELETE( lastStream ); } 
        TokenStream* tokenStream(const TCHAR * fieldName, Reader * reader) {
            return _CLNEW TokenFilterWithException(_CLNEW WhitespaceTokenizer(reader));
        };
        
        TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader)
        {
            _CLDELETE( lastStream );
            lastStream = _CLNEW TokenFilterWithException(_CLNEW WhitespaceTokenizer(reader));
            return lastStream;
        }
    };

    RAMDirectory * dir = _CLNEW RAMDirectory();
    AnalyzerWithException a;
    IndexWriter * writer = _CLNEW IndexWriter(dir, &a, true);

    Document* doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("aa bb cc dd ee ff gg hh ii"),
        Field::STORE_NO | Field::INDEX_TOKENIZED));
    try {
        writer->addDocument(doc);
        CuFail(tc, _T("did not hit expected exception"));
    } catch (CLuceneError&) {
    }
    _CLLDELETE(doc);

    // Make sure we can add another normal document
    doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("aa bb cc dd"), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);

    // Make sure we can add another normal document
    doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("aa bb cc dd"), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);

    writer->close();
    _CLLDELETE(writer);

    IndexReader* reader = IndexReader::open(dir);
    Term* t = _CLNEW Term(_T("content"), _T("aa"));
    assertEquals(reader->docFreq(t), 3);
    
    // Make sure the doc that hit the exception was marked
    // as deleted:
    TermDocs* tdocs = reader->termDocs(t);
    int count = 0;
    while(tdocs->next()) {
      count++;
    }
    _CLLDELETE(tdocs);
    assertEquals(2, count);
    
    t->set(_T("content"), _T("gg"));
    assertEquals(reader->docFreq(t), 0);
    _CLDECDELETE(t);

    reader->close();
    _CLLDELETE(reader);

    dir->close();
    _CLDECDELETE(dir);
}

/**
* Make sure we skip wicked long terms.
*/
void testWickedLongTerm(CuTest *tc) {
    RAMDirectory* dir = _CLNEW RAMDirectory();
    StandardAnalyzer a;
    IndexWriter* writer = _CLNEW IndexWriter(dir, &a, true);

    TCHAR bigTerm[16383];
    for (int i=0; i<16383; i++)
        bigTerm[i]=_T('x');
    bigTerm[16382] = 0;

    Document* doc = _CLNEW Document();

    // Max length term is 16383, so this contents produces
    // a too-long term:
    TCHAR* contents = _CL_NEWARRAY(TCHAR, 17000);
    _tcscpy(contents, _T("abc xyz x"));
    _tcscat(contents, bigTerm);
    _tcscat(contents, _T(" another term"));
    doc->add(* _CLNEW Field(_T("content"), contents, Field::STORE_NO | Field::INDEX_TOKENIZED));
    _CLDELETE_CARRAY(contents);
    writer->addDocument(doc);
    _CLLDELETE(doc);

    // Make sure we can add another normal document
    doc = _CLNEW Document();
    doc->add(* _CLNEW Field(_T("content"), _T("abc bbb ccc"), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer->addDocument(doc);
    _CLLDELETE(doc);
    writer->close();
    _CLDELETE(writer);

    IndexReader* reader = IndexReader::open(dir);

    // Make sure all terms < max size were indexed
    Term* t = _CLNEW Term(_T("content"), _T("abc"), true);
    assertEquals(2, reader->docFreq(t));
    t->set(_T("content"), _T("bbb"), true);
    assertEquals(1, reader->docFreq(t));
    t->set(_T("content"), _T("term"), true);
    assertEquals(1, reader->docFreq(t));
    t->set(_T("content"), _T("another"), true);
    assertEquals(1, reader->docFreq(t));

    // Make sure position is still incremented when
    // massive term is skipped:
    t->set(_T("content"), _T("another"), true);
    TermPositions* tps = reader->termPositions(t);
    assertTrue(tps->next());
    assertEquals(1, tps->freq());
    assertEquals(3, tps->nextPosition());
    _CLLDELETE(tps);

    // Make sure the doc that has the massive term is in
    // the index:
    assertEqualsMsg(_T("document with wicked long term should is not in the index!"), 1, reader->numDocs());

    reader->close();
    _CLLDELETE(reader);

    // Make sure we can add a document with exactly the
    // maximum length term, and search on that term:
    doc = _CLNEW Document();
    doc->add(*_CLNEW Field(_T("content"), bigTerm, Field::STORE_NO | Field::INDEX_TOKENIZED));
    StandardAnalyzer sa;
    sa.setMaxTokenLength(100000);
    writer = _CLNEW IndexWriter(dir, &sa, true);
    writer->addDocument(doc);
    _CLLDELETE(doc);
    writer->close();
    reader = IndexReader::open(dir);
    t->set(_T("content"), bigTerm);
    assertEquals(1, reader->docFreq(t));
    reader->close();

    _CLDECDELETE(t);

    _CLLDELETE(writer);
    _CLLDELETE(reader);

    dir->close();
    _CLDECDELETE(dir);
}


void testDeleteDocument(CuTest* tc) {
    const int size = 205;
    RAMDirectory* dir = _CLNEW RAMDirectory();
    StandardAnalyzer a;
    IndexWriter* writer = _CLNEW IndexWriter(dir, &a, true);

    // build an index that is big enough that a deletion files is written
    // in the DGaps format
    for (int i = 0; i < size; i++) {
        Document* doc = _CLNEW Document();
        TCHAR* contents = _CL_NEWARRAY(TCHAR, (size / 10) + 1);
        _i64tot(i, contents, 10);
        doc->add(* _CLNEW Field(_T("content"), contents, Field::STORE_NO | Field::INDEX_TOKENIZED));
    	_CLDELETE_CARRAY(contents);
        writer->addDocument(doc);
        _CLDELETE_ARRAY( contents );
        _CLLDELETE(doc);
    }

    // assure that the index has only one segment
    writer->optimize();
    // close and flush index
    writer->close();
    _CLLDELETE( writer );

    // reopen the index and delete the document next to last
    writer = _CLNEW IndexWriter(dir, &a, false);
    TCHAR* contents = _CL_NEWARRAY(TCHAR, (size / 10) + 1);
    _i64tot(size - 2, contents, 10);
    Term* t = _CLNEW Term(_T("content"), contents);
    _CLDELETE_LARRAY( contents );
    writer->deleteDocuments(t);
    writer->close();

    // now the index has a deletion file in the DGaps format

    _CLLDELETE(writer);
    _CLDECDELETE(t);

    // open this index with a searcher to read the deletions file again 
    IndexReader* reader = IndexReader::open(dir);
    IndexSearcher* searcher = _CLNEW IndexSearcher(reader);
    searcher->close();
    reader->close();
    _CLLDELETE(searcher);
    _CLLDELETE(reader);

    dir->close();
    _CLLDELETE( dir );
}

void testMergeIndex(CuTest* tc) {

    // A crash depends on the following:
    // - The first document needs two differently named fields that set TERMVECTOR_YES.
    // - The IndexWriter needs to reopen an existing index.
    // - The reopened IndexWriter needs to call optimize() to force a merge
    //   on term vectors. This merging causes the crash.
    // Submitted by McCann

    RAMDirectory* dir = _CLNEW RAMDirectory();

    // open a new lucene index
    SimpleAnalyzer a;
    IndexWriter* writer = _CLNEW IndexWriter( dir, false, &a, true );
    writer->setUseCompoundFile( false );

    // add two fields to document
    Document* doc = _CLNEW Document();
    doc->add ( *_CLNEW Field(_T("field0"), _T("value0"), Field::STORE_NO | Field::TERMVECTOR_YES | Field::INDEX_TOKENIZED) );
    doc->add ( *_CLNEW Field(_T("field1"), _T("value1"), Field::STORE_NO | Field::TERMVECTOR_YES | Field::INDEX_TOKENIZED) );
    writer->addDocument(doc);
    _CLLDELETE(doc);

    // close and flush index
    writer->close();
    _CLLDELETE( writer );

    // open the previous lucene index
    writer = _CLNEW IndexWriter( dir, false, &a, false );
    writer->setUseCompoundFile( false );

    // add a field to document
    // note: the settings on this field don't seem to affect the crash
    doc = _CLNEW Document();
    doc->add ( *_CLNEW Field(_T("field"), _T("value"), Field::STORE_NO | Field::TERMVECTOR_YES | Field::INDEX_TOKENIZED) );
    writer->addDocument(doc);
    _CLLDELETE(doc);

    // optimize index to force a merge
    writer->optimize();
    // close and flush index
    writer->close();
    _CLLDELETE( writer );

    // Close directory
    dir->close();
  _CLLDELETE( dir );
}

CuSuite *testindexwriter(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene IndexWriter Test"));
    SUITE_ADD_TEST(suite, testHashingBug);
    SUITE_ADD_TEST(suite, testAddIndexes);
    SUITE_ADD_TEST(suite, testIWmergeSegments1);
    SUITE_ADD_TEST(suite, testIWmergeSegments2);
    SUITE_ADD_TEST(suite, testIWmergePhraseSegments);
    SUITE_ADD_TEST(suite, testIWlargeScaleCorrectness);
    
    // TODO: This test fails due to differences between CLucene's StandardTokenizer and JLucene's; this test
    // should work when the tokenizer will be brought up-to-date, 
    //SUITE_ADD_TEST(suite, testWickedLongTerm);
    
    SUITE_ADD_TEST(suite, testExceptionFromTokenStream);
    SUITE_ADD_TEST(suite, testDeleteDocument);
    SUITE_ADD_TEST(suite, testMergeIndex);

    return suite;
}
// EOF
