/*------------------------------------------------------------------------------
* Copyright (C) 2010 Borivoj Kostka and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/RAMDirectory.h"
#include "CLucene/index/MergeScheduler.h"
#include "../store/MockRAMDirectory.h"
#include "IndexWriter4Test.h"

CL_NS_USE(store)
CL_NS_USE(index)
CL_NS_USE(document)
CL_NS_USE2(analysis,standard)

static IndexWriter4Test * newWriter(Directory * dir, WhitespaceAnalyzer * analyzer, bool create) {

    IndexWriter4Test * writer = _CLNEW IndexWriter4Test(dir, analyzer, create);
    writer->setMergePolicy(_CLNEW LogDocMergePolicy());
    return writer;
}

static void addDocs(IndexWriter4Test * writer, int numDocs) {

    for (int i = 0; i < numDocs; i++) {
        Document doc;
        doc.add(* _CLNEW Field(_T("content"), _T("aaa"), Field::STORE_NO | Field::INDEX_TOKENIZED));
        writer->addDocument(&doc);
    }
}

static void addDocs2(IndexWriter4Test * writer, int numDocs) {

    for (int i = 0; i < numDocs; i++) {
        Document doc;
        doc.add(* _CLNEW Field(_T("content"), _T("bbb"), Field::STORE_NO | Field::INDEX_TOKENIZED));
        writer->addDocument(&doc);
    }
}

static void verifyNumDocs(CuTest *tc, Directory * dir, int numDocs) {

    IndexReader * reader = IndexReader::open(dir);
    assertEquals(numDocs, reader->maxDoc());
    assertEquals(numDocs, reader->numDocs());
    reader->close();
    _CLLDELETE(reader);
}

static void verifyTermDocs(CuTest *tc, Directory * dir, Term * term, int numDocs) {

    IndexReader * reader = IndexReader::open(dir);
    TermDocs * termDocs = reader->termDocs(term);
    int count = 0;
    while (termDocs->next())
        count++;
    assertEquals(numDocs, count);
    termDocs->close();
    _CLLDELETE(termDocs);
    reader->close();
    _CLLDELETE(reader);
}

void setUpDirs(CuTest *tc, Directory * dir, Directory * aux) {

    IndexWriter4Test * writer = NULL;
    WhitespaceAnalyzer analyzer;

    writer = newWriter(dir, &analyzer, true);
    writer->setMaxBufferedDocs(1000);
    // add 1000 documents in 1 segment
    addDocs(writer, 1000);
    assertEquals(1000, writer->docCount());
    assertEquals(1, writer->getSegmentCount());
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(aux, &analyzer, true);
    writer->setUseCompoundFile(false); // use one without a compound file
    writer->setMaxBufferedDocs(100);
    writer->setMergeFactor(10);
    // add 30 documents in 3 segments
    for (int i = 0; i < 3; i++) {
        addDocs(writer, 10);
        writer->close();
        _CLLDELETE(writer);
        writer = newWriter(aux, &analyzer, false);
        writer->setUseCompoundFile(false); // use one without a compound file
        writer->setMaxBufferedDocs(100);
        writer->setMergeFactor(10);
    }
    assertEquals(30, writer->docCount());
    assertEquals(3, writer->getSegmentCount());
    writer->close();
    _CLLDELETE(writer);
}

void testSimpleCase(CuTest *tc) {

    // main directory
    Directory * dir = _CLNEW RAMDirectory();
    // two auxiliary directories
    Directory * aux = _CLNEW RAMDirectory();
    Directory * aux2 = _CLNEW RAMDirectory();

    IndexWriter4Test * writer = NULL;

    WhitespaceAnalyzer analyzer;

    writer = newWriter(dir, &analyzer, true);

    // add 100 documents
    addDocs(writer, 100);
    assertEquals(100, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(aux, &analyzer, true);
    writer->setUseCompoundFile(false); // use one without a compound file
    // add 40 documents in separate files
    addDocs(writer, 40);
    assertEquals(40, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(aux2, &analyzer, true);
    // add 40 documents in compound files
    addDocs2(writer, 50);
    assertEquals(50, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    // test doc count before segments are merged
    writer = newWriter(dir, &analyzer, false);
    assertEquals(100, writer->docCount());
    {
        ValueArray<Directory*> dirs(2);
        dirs[0] = aux;
        dirs[1] = aux2;
        writer->addIndexesNoOptimize( dirs );
    }
    assertEquals(190, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    // make sure the old index is correct
    verifyNumDocs(tc, aux, 40);

    // make sure the new index is correct
    verifyNumDocs(tc, dir, 190);

    // now add another set in.
    Directory * aux3 = _CLNEW RAMDirectory();
    writer = newWriter(aux3, &analyzer, true);
    // add 40 documents
    addDocs(writer, 40);
    assertEquals(40, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    // test doc count before segments are merged/index is optimized
    writer = newWriter(dir, &analyzer, false);
    assertEquals(190, writer->docCount());
    {
        ValueArray<Directory*> dirs(1);
        dirs[0] = aux3;
        writer->addIndexesNoOptimize( dirs );
    }
    assertEquals(230, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    // make sure the new index is correct
    verifyNumDocs(tc, dir, 230);

    Term t1(_T("content"), _T("aaa"));
    Term t2(_T("content"), _T("bbb"));

    verifyTermDocs(tc, dir, &t1, 180);
    verifyTermDocs(tc, dir, &t2, 50);

    // now optimize it.
    writer = newWriter(dir, &analyzer, false);
    writer->optimize();
    writer->close();
    _CLLDELETE(writer);

    // make sure the new index is correct
    verifyNumDocs(tc, dir, 230);

    verifyTermDocs(tc, dir, &t1, 180);
    verifyTermDocs(tc, dir, &t2, 50);

    // now add a single document
    Directory * aux4 = _CLNEW RAMDirectory();
    writer = newWriter(aux4, &analyzer, true);
    addDocs2(writer, 1);
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(dir, &analyzer, false);
    assertEquals(230, writer->docCount());

    {
        ValueArray<Directory*> dirs(1);
        dirs[0] = aux4;
        writer->addIndexesNoOptimize( dirs );
    }

    assertEquals(231, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    verifyNumDocs(tc, dir, 231);

    verifyTermDocs(tc, dir, &t2, 51);

    dir->close();
    _CLLDELETE(dir);
    aux->close();
    _CLLDELETE(aux);
    aux2->close();
    _CLLDELETE(aux2);
    aux3->close();
    _CLLDELETE(aux3);
    aux4->close();
    _CLLDELETE(aux4);
}

// case 0: add self or exceed maxMergeDocs, expect exception
void testAddSelf(CuTest * tc)  {

    // main directory
    Directory * dir = _CLNEW RAMDirectory();
    // auxiliary directory
    Directory * aux = _CLNEW RAMDirectory();

    IndexWriter4Test * writer = NULL;
    WhitespaceAnalyzer analyzer;

    writer = newWriter(dir, &analyzer, true);
    // add 100 documents
    addDocs(writer, 100);
    assertEquals(100, writer->docCount());
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(aux, &analyzer, true);
    writer->setUseCompoundFile(false); // use one without a compound file
    writer->setMaxBufferedDocs(1000);
    // add 140 documents in separate files
    addDocs(writer, 40);
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(aux, &analyzer, true);
    writer->setUseCompoundFile(false); // use one without a compound file
    writer->setMaxBufferedDocs(1000);
    addDocs(writer, 100);
    writer->close();
    _CLLDELETE(writer);

    writer = newWriter(dir, &analyzer, false);
    try {
      // cannot add self
      ValueArray<Directory*> dirs(2);
      dirs[0] = aux;
      dirs[1] = dir;
      writer->addIndexesNoOptimize( dirs );
      assertTrue(false);
    }
    catch (CLuceneError&) {
      assertEquals(100, writer->docCount());
    }
    writer->close();
    _CLLDELETE(writer);

    // make sure the index is correct
    verifyNumDocs(tc, dir, 100);

    dir->close();
    _CLLDELETE(dir);

    aux->close();
    _CLLDELETE(aux);
}

  // in all the remaining tests, make the doc count of the oldest segment
  // in dir large so that it is never merged in addIndexesNoOptimize()
  // case 1: no tail segments
void testNoTailSegments(CuTest * tc) {

    // main directory
    Directory *dir = _CLNEW RAMDirectory();
    // auxiliary directory
    Directory *aux = _CLNEW RAMDirectory();

    WhitespaceAnalyzer  analyzer;

    setUpDirs(tc, dir, aux);

    IndexWriter4Test * writer = newWriter(dir, &analyzer, false);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(4);
    addDocs(writer, 10);

    ValueArray<Directory*> dirs(1);
    dirs[0] = aux;
    writer->addIndexesNoOptimize(dirs);

    assertEquals(1040, writer->docCount());
    assertEquals(2, writer->getSegmentCount());
    assertEquals(1000, writer->getDocCount(0));
    writer->close();
    _CLLDELETE(writer);

    // make sure the index is correct
    verifyNumDocs(tc, dir, 1040);

    dir->close();
    _CLLDELETE(dir);

    aux->close();
    _CLLDELETE(aux);
}

// case 2: tail segments, invariants hold, no copy
void testNoCopySegments(CuTest * tc)  {

    // main directory
    Directory * dir = _CLNEW RAMDirectory();
    // auxiliary directory
    Directory * aux = _CLNEW RAMDirectory();

    WhitespaceAnalyzer  an;

    setUpDirs(tc, dir, aux);

    IndexWriter4Test * writer = newWriter(dir, &an, false);
    writer->setMaxBufferedDocs(9);
    writer->setMergeFactor(4);
    addDocs(writer, 2);

    ValueArray<Directory*> dirs(1);
    dirs[0] = aux;
    writer->addIndexesNoOptimize(dirs);

    assertEquals(1032, writer->docCount());
    assertEquals(2, writer->getSegmentCount());
    assertEquals(1000, writer->getDocCount(0));
    writer->close();
    _CLLDELETE(writer);

    // make sure the index is correct
    verifyNumDocs(tc, dir, 1032);

    dir->close();
    _CLLDELETE(dir);

    aux->close();
    _CLLDELETE(aux);
}

// case 3: tail segments, invariants hold, copy, invariants hold
void testNoMergeAfterCopy(CuTest * tc) {

    // main directory
    Directory * dir = _CLNEW RAMDirectory();
    // auxiliary directory
    Directory * aux = _CLNEW RAMDirectory();

    WhitespaceAnalyzer  an;

    setUpDirs(tc, dir, aux);

    IndexWriter4Test * writer = newWriter(dir, &an, false);
    writer->setMaxBufferedDocs(10);
    writer->setMergeFactor(4);

    ValueArray<Directory*> dirs(2);
    dirs[0] = aux;
    dirs[1] = aux;
    writer->addIndexesNoOptimize(dirs);

    assertEquals(1060, writer->docCount());
    assertEquals(1000, writer->getDocCount(0));
    writer->close();
    _CLLDELETE(writer);

    // make sure the index is correct
    verifyNumDocs(tc, dir, 1060);

    dir->close();
    _CLLDELETE(dir);

    aux->close();
    _CLLDELETE(aux);
}

// case 4: tail segments, invariants hold, copy, invariants not hold
void testMergeAfterCopy(CuTest * tc) {

    // main directory
    Directory * dir = _CLNEW RAMDirectory();
    // auxiliary directory
    Directory * aux = _CLNEW RAMDirectory();

    WhitespaceAnalyzer  an;

    setUpDirs(tc, dir, aux);

    IndexReader * reader = IndexReader::open(aux);
    for (int i = 0; i < 20; i++) {
      reader->deleteDocument(i);
    }
    assertEquals(10, reader->numDocs());
    reader->close();
    _CLLDELETE(reader);

    IndexWriter4Test * writer = newWriter(dir, &an, false);
    writer->setMaxBufferedDocs(4);
    writer->setMergeFactor(4);

    ValueArray<Directory*> dirs(2);
    dirs[0] = aux;
    dirs[1] = aux;
    writer->addIndexesNoOptimize(dirs);

    assertEquals(1020, writer->docCount());
    assertEquals(1000, writer->getDocCount(0));
    writer->close();
    _CLLDELETE(writer);

    // make sure the index is correct
    verifyNumDocs(tc, dir, 1020);

    dir->close();
    _CLLDELETE(dir);

    aux->close();
    _CLLDELETE(aux);
}

// case 5: tail segments, invariants not hold
void testMoreMerges(CuTest * tc)  {

    // main directory
    Directory * dir = _CLNEW RAMDirectory();
    // auxiliary directory
    Directory * aux = _CLNEW RAMDirectory();
    Directory * aux2 = _CLNEW RAMDirectory();

    WhitespaceAnalyzer  an;

    setUpDirs(tc, dir, aux);

    IndexWriter4Test * writer = newWriter(aux2, &an, true);
    writer->setMaxBufferedDocs(100);
    writer->setMergeFactor(10);

    ValueArray<Directory*> dirs(1);
    dirs[0] = aux;
    writer->addIndexesNoOptimize(dirs);

    assertEquals(30, writer->docCount());
    assertEquals(3, writer->getSegmentCount()); 
    writer->close();
    _CLLDELETE(writer);

    IndexReader * reader = IndexReader::open(aux);
    for (int i = 0; i < 27; i++) {
      reader->deleteDocument(i);
    }
    assertEquals(3, reader->numDocs());
    reader->close();
    _CLLDELETE(reader);

    reader = IndexReader::open(aux2);
    for (int i = 0; i < 8; i++) {
      reader->deleteDocument(i);
    }
    assertEquals(22, reader->numDocs());
    reader->close();
    _CLLDELETE( reader );

    writer = newWriter(dir, &an, false);
    writer->setMaxBufferedDocs(6);
    writer->setMergeFactor(4);

    ValueArray<Directory*> dirs2(2);
    dirs2[0] = aux;
    dirs2[1] = aux2;
    writer->addIndexesNoOptimize(dirs2);

    assertEquals(1025, writer->docCount());
    assertEquals(1000, writer->getDocCount(0));
    writer->close();
    _CLLDELETE( writer );

    // make sure the index is correct
    verifyNumDocs(tc, dir, 1025);

    dir->close();
    _CLLDELETE(dir);

    aux->close();
    _CLLDELETE(aux);

    aux2->close();
    _CLLDELETE(aux2);
}

// LUCENE-1270
void testHangOnClose(CuTest * tc) {

    WhitespaceAnalyzer an;
    MockRAMDirectory * dir = _CLNEW MockRAMDirectory();
    dir->setRandomIOExceptionRate(0.0, 0);
    IndexWriter4Test * writer = _CLNEW IndexWriter4Test(dir, false, &an, true);
    writer->setMergePolicy(_CLNEW LogByteSizeMergePolicy());
    writer->setMaxBufferedDocs(5);
    writer->setUseCompoundFile(false);
    writer->setMergeFactor(100);

    Document doc;
    doc.add(* _CLNEW Field(_T("content"), _T("aaa bbb ccc ddd eee fff ggg hhh iii"), 
                        Field::STORE_YES | Field::INDEX_TOKENIZED | Field::TERMVECTOR_WITH_POSITIONS_OFFSETS));
    for(int i=0;i<60;i++)
      writer->addDocument(&doc);
    writer->setMaxBufferedDocs(200);
    Document doc2;
    doc2.add(* _CLNEW Field(_T("content"), _T("aaa bbb ccc ddd eee fff ggg hhh iii"), 
                        Field::STORE_YES | Field::INDEX_NO));
    doc2.add(* _CLNEW Field(_T("content"), _T("aaa bbb ccc ddd eee fff ggg hhh iii"), 
        Field::STORE_YES | Field::INDEX_NO));
    doc2.add(* _CLNEW Field(_T("content"), _T("aaa bbb ccc ddd eee fff ggg hhh iii"), 
        Field::STORE_YES | Field::INDEX_NO));
    doc2.add(* _CLNEW Field(_T("content"), _T("aaa bbb ccc ddd eee fff ggg hhh iii"), 
        Field::STORE_YES | Field::INDEX_NO));
    for(int i=0;i<10;i++)
      writer->addDocument(&doc2);
    writer->close();
    _CLLDELETE(writer);

    MockRAMDirectory * dir2 = _CLNEW MockRAMDirectory();
    dir2->setRandomIOExceptionRate(0.0, 0);
    writer = _CLNEW IndexWriter4Test(dir2, false, &an, true);

    LogByteSizeMergePolicy * lmp = _CLNEW LogByteSizeMergePolicy();
    lmp->setMinMergeMB(0.0001);
    writer->setMergePolicy(lmp);
    writer->setMergeFactor(4);
    writer->setUseCompoundFile(false);
    writer->setMergeScheduler(_CLNEW SerialMergeScheduler());


    ValueArray<Directory*> dirs(1);
    dirs[0] = dir;
    writer->addIndexesNoOptimize(dirs);

    writer->close();
    _CLLDELETE(writer);

    dir->close();
    _CLLDELETE(dir);

    dir2->close();
    _CLLDELETE(dir2);

}

CuSuite *testAddIndexesNoOptimize(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene AddIndexesNoOptimize Test"));
    SUITE_ADD_TEST(suite, testSimpleCase);
    SUITE_ADD_TEST(suite, testAddSelf);
    SUITE_ADD_TEST(suite, testNoTailSegments);
    SUITE_ADD_TEST(suite, testNoCopySegments);
    SUITE_ADD_TEST(suite, testNoMergeAfterCopy);
    SUITE_ADD_TEST(suite, testMergeAfterCopy);
    SUITE_ADD_TEST(suite, testMoreMerges);
    SUITE_ADD_TEST(suite, testHangOnClose);

    return suite;
}
