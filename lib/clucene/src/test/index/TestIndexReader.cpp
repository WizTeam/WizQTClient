/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include <iostream>
#include "CLucene/index/_SegmentHeader.h"
#include "CLucene/index/_MultiSegmentReader.h"
#include "CLucene/index/MultiReader.h"

typedef IndexReader* (*TestIRModifyIndex)(CuTest* tc, IndexReader* reader, int modify);
DEFINE_MUTEX(createReaderMutex)


void createDocument(Document& doc, int n, int numFields) {
  doc.clear();
  StringBuffer sb;
  sb.append( _T("a"));
  sb.appendInt(n);
  doc.add(* _CLNEW Field( _T("field1"), sb.getBuffer(), Field::STORE_YES | Field::INDEX_TOKENIZED));
  sb.append(_T(" b"));
  sb.appendInt(n);
  for (int i = 1; i < numFields; i++) {
    TCHAR buf[10];
    _sntprintf(buf,10,_T("field%d"), i+1);
    doc.add(* _CLNEW Field(buf, sb.getBuffer(), Field::STORE_YES | Field::INDEX_TOKENIZED));
  }
}

void createIndex(CuTest* tc, Directory* dir, bool multiSegment) {
  WhitespaceAnalyzer whitespaceAnalyzer;
  IndexWriter w(dir, &whitespaceAnalyzer, true);

  w.setMergePolicy(_CLNEW LogDocMergePolicy());
  Document doc;
  for (int i = 0; i < 100; i++) {
    createDocument(doc, i, 4);
    w.addDocument(&doc);
    if (multiSegment && (i % 10) == 0) {
      w.flush();
    }
  }

  if (!multiSegment) {
    w.optimize();
  }

  w.close();

  IndexReader* r = IndexReader::open(dir);
  if (multiSegment) {
    CuAssert(tc,_T("check is multi"), strcmp(r->getObjectName(),"MultiSegmentReader")==0);
  } else {
    CuAssert(tc,_T("check is segment"), strcmp(r->getObjectName(),"SegmentReader")==0);
  }
  r->close();
  _CLDELETE(r);
}

struct ReaderCouple{
  IndexReader* newReader;
  IndexReader* refreshedReader;
};

void assertReaderClosed(CuTest* tc, IndexReader* reader, bool checkSubReaders, bool checkNormsClosed) {
  //TODO: CuAssertIntEquals(tc, _T("Check refcount is zero"), 0, reader->getRefCount());

/* can't test internal...  if (checkNormsClosed && reader->instanceOf(SegmentReader::getClassName())) {
    CuAssertTrue(tc, ((SegmentReader*) reader)->normsClosed());
  }*/

  if (checkSubReaders) {
 /* can't test internal...
    if (reader->instanceOf(MultiSegmentReader::getClassName())) {
      const CL_NS(util)::ArrayBase<IndexReader*>& subReaders = *((MultiSegmentReader*) reader)->getSubReaders();
      for (int i = 0; i < subReaders.length; i++) {
        assertReaderClosed(tc, subReaders[i], checkSubReaders, checkNormsClosed);
      }
    }*/

    if (reader != NULL && reader->instanceOf(MultiReader::getClassName())) {
      const CL_NS(util)::ArrayBase<IndexReader*>& subReaders = *((MultiReader*) reader)->getSubReaders();
      for (size_t i = 0; i < subReaders.length; i++) {
        assertReaderClosed(tc, subReaders[i], checkSubReaders, checkNormsClosed);
      }
    }
  }
}

void assertReaderOpen(CuTest* /*tc*/, IndexReader* reader) {
  Document doc;
  reader->document(0, doc);//hack to call ensureOpen...

/* can't test internal...  if (reader->instanceOf(MultiSegmentReader::getClassName())) {
    const CL_NS(util)::ArrayBase<IndexReader*>& subReaders = *((MultiSegmentReader*) reader)->getSubReaders();
    for (int i = 0; i < subReaders.length; i++) {
      assertReaderOpen(tc, subReaders[i]);
    }
  }*/
}

Directory* defaultModifyIndexTestDir1 = NULL;
IndexReader* defaultModifyIndexTest(CuTest* /*tc*/, IndexReader* reader, int i){
  WhitespaceAnalyzer whitespaceAnalyzer;
  switch (i) {
    case 0: {
      IndexWriter w(defaultModifyIndexTestDir1, &whitespaceAnalyzer, false);
      Term* t1 = _CLNEW Term(_T("field2"), _T("a11"));
      w.deleteDocuments(t1);
      Term* t2 = _CLNEW Term(_T("field2"), _T("b30"));
      w.deleteDocuments(t2);
      _CLDECDELETE(t1);
      _CLDECDELETE(t2);
      w.close();
      break;
    }
    case 1: {
      IndexReader* reader = IndexReader::open(defaultModifyIndexTestDir1);
      reader->setNorm(4, _T("field1"), (uint8_t)123);
      reader->setNorm(44, _T("field2"), (uint8_t)222);
      reader->setNorm(44, _T("field4"), (uint8_t)22);
      reader->close();
      _CLDELETE(reader);
      break;
    }
    case 2: {
      IndexWriter w(defaultModifyIndexTestDir1, &whitespaceAnalyzer, false);
      w.optimize();
      w.close();
      break;
    }
    case 3: {
      IndexWriter w(defaultModifyIndexTestDir1, &whitespaceAnalyzer, false);
      Document doc;
      createDocument(doc, 101, 4);
      w.addDocument(&doc);
      w.optimize();
      createDocument(doc,102, 4);
      w.addDocument(&doc);
      createDocument(doc,103, 4);
      w.addDocument(&doc);
      w.close();
      break;
    }
    case 4: {
      IndexReader* reader = IndexReader::open(defaultModifyIndexTestDir1);
      reader->setNorm(5, _T("field1"), (uint8_t)123);
      reader->setNorm(55, _T("field2"), (uint8_t)222);
      reader->close();
      _CLDELETE(reader);
      break;
    }

  }
  return IndexReader::open(defaultModifyIndexTestDir1);
}


Directory* defaultModifyIndexTestDir2 = NULL;
IndexReader* defaultModifyIndexTestMulti(CuTest* tc, IndexReader* reader, int i){
  IndexReader* x = defaultModifyIndexTest(tc,reader,i); //call main test
  _CLDELETE(x);


  Directory* tmp = defaultModifyIndexTestDir1;
  defaultModifyIndexTestDir1 = defaultModifyIndexTestDir2;
  x = defaultModifyIndexTest(tc,reader,i); //call main test
  _CLDELETE(x);
  defaultModifyIndexTestDir1 = tmp;

  ValueArray<IndexReader*> readers(2);
  readers[0] = IndexReader::open(defaultModifyIndexTestDir1);
  readers[1] = IndexReader::open(defaultModifyIndexTestDir2);
  return _CLNEW MultiReader(&readers, true);
}

ReaderCouple refreshReader(CuTest* tc, IndexReader* reader, TestIRModifyIndex test, int modify, bool hasChanges) {
  SCOPED_LOCK_MUTEX(createReaderMutex)

  IndexReader* r = NULL;
  if (test != NULL) {
    r = (*test)(tc, reader, modify);
  }

  IndexReader* refreshed = reader->reopen();
  if (hasChanges) {
    CuAssert(tc, _T("No new IndexReader instance created during refresh."), refreshed != reader );
  } else {
    CuAssert(tc,  _T("New IndexReader instance created during refresh even though index had no changes."), refreshed == reader);
  }

  ReaderCouple ret = {r, refreshed};
  return ret;
}
ReaderCouple refreshReader(CuTest* tc, IndexReader* reader, bool hasChanges) {
  return refreshReader(tc,reader, NULL, -1, hasChanges);
}
void performDefaultIRTests(CuTest *tc, IndexReader* index1, IndexReader* index2, IndexReader* index2B, TestIRModifyIndex test){
  TestAssertIndexReaderEquals(tc, index1, index2);

  // verify that reopen() does not return a new reader instance
  // in case the index has no changes
  ReaderCouple couple = refreshReader(tc, index2, false);
  CuAssertTrue(tc, couple.refreshedReader == index2);

  couple = refreshReader(tc, index2, test, 0, true);
  index1 = couple.newReader;
  IndexReader* index2_refreshed = couple.refreshedReader;
  index2->close();
  assertReaderClosed(tc, index2, true, true);
  if ( index2_refreshed != index2 ){
    _CLDELETE(index2);
  }

  // test if refreshed reader and newly opened reader return equal results
  TestAssertIndexReaderEquals(tc, index1, index2_refreshed);

  index1->close();
  _CLDELETE(index1);
  index2_refreshed->close();
  assertReaderClosed(tc, index2_refreshed, true, true);
  _CLDELETE(index2_refreshed);

  index2 = index2B;

  for (int i = 1; i < 4; i++) {
    couple = refreshReader(tc, index2, test, i, true);
    // refresh IndexReader
    index2->close();
    if ( couple.refreshedReader != index2 ){
      _CLDELETE(index2);
    }

    index2 = couple.refreshedReader;
    index1 = couple.newReader;
    TestAssertIndexReaderEquals(tc, index1, index2);
    index1->close();
    assertReaderClosed(tc, index1, true, true);
    _CLDELETE(index1);
  }

  index2->close();
  assertReaderClosed(tc, index2, true, true);
  _CLDELETE(index2);
}



void testIndexReaderReopen(CuTest *tc){
  RAMDirectory dir;
  createIndex(tc, &dir, false);
  IndexReader* index1 = IndexReader::open(&dir);
  IndexReader* index2 = IndexReader::open(&dir);
  IndexReader* index2B = IndexReader::open(&dir);

  defaultModifyIndexTestDir1 = &dir;
  performDefaultIRTests(tc, index1, index2, index2B, defaultModifyIndexTest);
  defaultModifyIndexTestDir1 = NULL;

  index1->close();
  _CLDELETE(index1);
  //_CLDELETE(index2);this one gets deleted...
  //_CLDELETE(index2B);
}


void testMultiReaderReopen(CuTest *tc){
  RAMDirectory dir1;
  createIndex(tc, &dir1, true);
  RAMDirectory dir2 ;
  createIndex(tc, &dir2, true);

  ValueArray<IndexReader*> readers1(2);
  readers1[0] = IndexReader::open(&dir1);
  readers1[1] = IndexReader::open(&dir2);
  IndexReader* index1 = _CLNEW MultiReader(&readers1, true);

  ValueArray<IndexReader*> readers2(2);
  readers2[0] = IndexReader::open(&dir1);
  readers2[1] = IndexReader::open(&dir2);
  IndexReader* index2 = _CLNEW MultiReader(&readers2, true);

  ValueArray<IndexReader*> readers2b(2);
  readers2b[0] = IndexReader::open(&dir1);
  readers2b[1] = IndexReader::open(&dir2);
  IndexReader* index2b = _CLNEW MultiReader(&readers2b, true);

  defaultModifyIndexTestDir1 = &dir1;
  defaultModifyIndexTestDir2 = &dir2;
  performDefaultIRTests(tc, index1, index2, index2b, defaultModifyIndexTestMulti);
  defaultModifyIndexTestDir1 = NULL;
  defaultModifyIndexTestDir2 = NULL;

  _CLDELETE(index1);
  //_CLDELETE(index2);this one gets deleted...
  //_CLDELETE(index2B);
}

CuSuite *testindexreader(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene IndexReader Test"));
  SUITE_ADD_TEST(suite, testIndexReaderReopen);
  SUITE_ADD_TEST(suite, testMultiReaderReopen);

  return suite;
}
// EOF
