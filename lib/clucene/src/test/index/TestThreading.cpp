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
#include <stdio.h>


_LUCENE_THREADID_TYPE* atomicSearchThreads = NULL;
bool atomicSearchFailed = false;
#define ATOMIC_SEARCH_RUN_TIME_SEC 3
_LUCENE_THREAD_FUNC(atomicIndexTest, _writer){
  IndexWriter* writer= (IndexWriter*)_writer;
  uint64_t stopTime = Misc::currentTimeMillis() + 1000*ATOMIC_SEARCH_RUN_TIME_SEC;
  int count = 0;
  try {
    while(Misc::currentTimeMillis() < stopTime && !atomicSearchFailed) {
      // Update all 100 docs...
      TCHAR buf[30];
      StringBuffer sb;
      for(int i=0; i<100; i++) {
        Document d;
        _i64tot(rand(), buf, 10);

        sb.clear();
        English::IntToEnglish(i+10*count, &sb);
        d.add(*_CLNEW Field(_T("contents"), sb.getBuffer() , Field::STORE_NO | Field::INDEX_TOKENIZED));

        _i64tot(i,buf,10);
        d.add(*_CLNEW Field(_T("id"), buf, Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        Term* t = _CLNEW Term(_T("id"), buf);
        writer->updateDocument(t, &d);
        _CLDECDELETE(t);
      }

      count++;
    }
  } catch (CLuceneError& e) {
    fprintf(stderr, "err 1: #%d: %s\n", e.number(), e.what());
    atomicSearchFailed = true;
  }

  _LUCENE_THREAD_FUNC_RETURN(0);
}

_LUCENE_THREAD_FUNC(atomicSearchTest, _directory){
  Directory* directory = (Directory*)_directory;

  uint64_t stopTime = Misc::currentTimeMillis() + 1000*ATOMIC_SEARCH_RUN_TIME_SEC;
  int count = 0;
  try {
    while(Misc::currentTimeMillis() < stopTime && !atomicSearchFailed) {
      IndexReader* r = IndexReader::open(directory);

      try {
        if ( 100 != r->numDocs() ){
          fprintf(stderr, "err 2: 100 != %d \n", r->numDocs());
          atomicSearchFailed = true;
        }
      } catch (CLuceneError& e) {
        fprintf(stderr, "err 3: %d:%s\n", e.number(), e.what());
        atomicSearchFailed = true;
        break;
      }
      r->close();
      _CLDELETE(r);

      count++;
    }
  } catch (CLuceneError& e) {
    fprintf(stderr, "err 4: #%d: %s\n", e.number(), e.what());
    atomicSearchFailed = true;
  }

  _LUCENE_THREAD_FUNC_RETURN(0);
}

/*
  Run one indexer and 2 searchers against single index as
  stress test.
 */
void runThreadingTests(CuTest* tc, Directory& directory){

  SimpleAnalyzer ANALYZER;
  IndexWriter writer(&directory, &ANALYZER, true);

  // Establish a base index of 100 docs:
  StringBuffer sb;
  TCHAR buf[10];
  for(int i=0;i<100;i++) {
    Document d;
    _i64tot(i,buf,10);
    d.add(*_CLNEW Field(_T("id"), buf, Field::STORE_YES | Field::INDEX_UNTOKENIZED));

    sb.clear();
    English::IntToEnglish(i, &sb);
    d.add(*_CLNEW Field(_T("contents"), sb.getBuffer(), Field::STORE_NO | Field::INDEX_TOKENIZED));
    writer.addDocument(&d);
  }
  writer.flush();

  //read using multiple threads...
  atomicSearchThreads = _CL_NEWARRAY(_LUCENE_THREADID_TYPE, 4);
  atomicSearchThreads[0] = _LUCENE_THREAD_CREATE(&atomicIndexTest, &writer);
  atomicSearchThreads[1] = _LUCENE_THREAD_CREATE(&atomicIndexTest, &writer);
  atomicSearchThreads[2] = _LUCENE_THREAD_CREATE(&atomicSearchTest, &directory);
  atomicSearchThreads[3] = _LUCENE_THREAD_CREATE(&atomicSearchTest, &directory);

  for ( int i=0;i<4;i++ ){
    _LUCENE_THREAD_JOIN(atomicSearchThreads[i]);
  }
  _CLDELETE_ARRAY(atomicSearchThreads);

  writer.close();

  CuAssert(tc, _T("hit unexpected exception in one of the threads\n"), !atomicSearchFailed);
}

/*
  Run above stress test against RAMDirectory and then
  FSDirectory.
 */
void testRAMThreading(CuTest *tc){
  // First in a RAM directory:
  RAMDirectory directory; //todo: use MockRAMDirectory?
  runThreadingTests(tc,directory);
  directory.close();
}

/*
  Run above stress test against FSDirectory.
 */
void testFSThreading(CuTest *tc){
  //setup some variables
  char tmpfsdirectory[1024];
  strcpy(tmpfsdirectory,cl_tempDir);
  strcat(tmpfsdirectory,"/threading-index");

  // Second in an FSDirectory:
  Directory* directory = FSDirectory::getDirectory(tmpfsdirectory);
  runThreadingTests(tc,*directory);
  directory->close();
  _CLDECDELETE(directory);
}

CuSuite *testatomicupdates(void)
{
  srand ( (unsigned int)Misc::currentTimeMillis() );
  CuSuite *suite = CuSuiteNew(_T("CLucene Atomic Updates Test"));
  SUITE_ADD_TEST(suite, testRAMThreading);
  SUITE_ADD_TEST(suite, testFSThreading);

  return suite;
}
// EOF
