/*------------------------------------------------------------------------------
* Copyright (C) 2010 Borivoj Kostka and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include <string>
#include <iostream>

#include "CLucene.h"
#include "CLucene/_clucene-config.h"

#include "CLucene/analysis/Analyzers.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/IndexWriter.h"
#include "CLucene/search/IndexSearcher.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/FSDirectory.h"
#include "CLucene/store/RAMDirectory.h"
#include "MockRAMDirectory.h"

/**
 * JUnit testcase to test RAMDirectory. RAMDirectory itself is used in many testcases,
 * but not one of them uses an different constructor other than the default constructor.
 * 
 * @author Bernhard Messer
 * 
 * @version $Id: RAMDirectory.java 150537 2004-09-28 22:45:26 +0200 (Di, 28 Sep 2004) cutting $
 */

static int docsToAdd = 500;
static char indexDir[CL_MAX_PATH] = "";

struct ThreadData
{
    MockRAMDirectory * dir;
    IndexWriter * writer;
    int           num;
    CuTest *      tc;
};

// setup the index
void testRAMDirectorySetUp (CuTest *tc) {

    if (strlen(cl_tempDir) + 13 > CL_MAX_PATH)
        CuFail(tc, _T("Not enough space in indexDir buffer"));

    sprintf(indexDir, "%s/RAMDirIndex", cl_tempDir);
    WhitespaceAnalyzer analyzer;
    IndexWriter * writer  = new IndexWriter(indexDir, &analyzer, true);

    // add some documents
    TCHAR * text;
    for (int i = 0; i < docsToAdd; i++) {
      Document doc;
      text = English::IntToEnglish(i);
      doc.add(* new Field(_T("content"), text, Field::STORE_YES | Field::INDEX_UNTOKENIZED));
      writer->addDocument(&doc);
      _CLDELETE_ARRAY(text);
    }

    CuAssertEquals(tc, docsToAdd, writer->docCount(), _T("document count"));
    writer->close();
    _CLDELETE( writer );
}

// BK> all test functions are the same except RAMDirectory constructor, so shared code moved here
void checkDir(CuTest *tc, MockRAMDirectory * ramDir) {

    // Check size
    CuAssertTrue(tc, ramDir->sizeInBytes == ramDir->getRecomputedSizeInBytes(), _T("RAMDir size"));

    // open reader to test document count
    IndexReader * reader = IndexReader::open(ramDir);
    CuAssertEquals(tc, docsToAdd, reader->numDocs(), _T("document count"));

    // open search to check if all doc's are there
    IndexSearcher * searcher = _CLNEW IndexSearcher(reader);

    // search for all documents
    Document doc;
    for (int i = 0; i < docsToAdd; i++) {
        searcher->doc(i, doc);
        CuAssertTrue(tc, doc.getField(_T("content")) != NULL, _T("content is NULL"));
    }

    // cleanup
    reader->close();
    searcher->close();
    _CLLDELETE(reader);
    _CLLDELETE(searcher);
}

void testRAMDirectory (CuTest *tc) {

    Directory * dir = FSDirectory::getDirectory(indexDir);
    MockRAMDirectory * ramDir = _CLNEW MockRAMDirectory(dir);

    // close the underlaying directory
    dir->close();
    _CLDELETE( dir );

    checkDir(tc, ramDir);

    ramDir->close();
    _CLLDELETE(ramDir);
}

void testRAMDirectoryString (CuTest *tc) {

    MockRAMDirectory * ramDir = _CLNEW MockRAMDirectory(indexDir);

    checkDir(tc, ramDir);

    ramDir->close();
    _CLLDELETE(ramDir);
}

static int numThreads = 50;
static int docsPerThread = 40;

_LUCENE_THREAD_FUNC(indexDocs, _data) {

    ThreadData * data = (ThreadData *)_data;
    int cnt = 0;
    TCHAR * text;
    for (int j=1; j<docsPerThread; j++) {
        Document doc;
        text = English::IntToEnglish(data->num*docsPerThread+j);
        doc.add(*new Field(_T("sizeContent"), text, Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        data->writer->addDocument(&doc);
        _CLDELETE_ARRAY(text);
        {
            SCOPED_LOCK_MUTEX(data->dir->THIS_LOCK);
            CuAssertTrue(data->tc, data->dir->sizeInBytes == data->dir->getRecomputedSizeInBytes());
        }
    }
    _LUCENE_THREAD_FUNC_RETURN( 0 );
}
  
void testRAMDirectorySize(CuTest * tc)  {
      
    MockRAMDirectory * ramDir = _CLNEW MockRAMDirectory(indexDir);
    WhitespaceAnalyzer analyzer;
    IndexWriter * writer = _CLNEW IndexWriter(ramDir, &analyzer, false);
    writer->optimize();
    
    CuAssertTrue(tc, ramDir->sizeInBytes == ramDir->getRecomputedSizeInBytes(), _T("RAMDir size"));

    _LUCENE_THREADID_TYPE* threads = _CL_NEWARRAY(_LUCENE_THREADID_TYPE, numThreads);
    ThreadData * tdata = _CL_NEWARRAY(ThreadData, numThreads);
    
    for (int i=0; i<numThreads; i++) {
      tdata[i].num = i;
      tdata[i].dir = ramDir;
      tdata[i].tc = tc;
      tdata[i].writer = writer;
      threads[i] = _LUCENE_THREAD_CREATE(&indexDocs, &tdata[i]);
    }

    for (int i=0; i<numThreads; i++){
        _LUCENE_THREAD_JOIN(threads[i]);
    }

    _CLDELETE_ARRAY(threads);
    _CLDELETE_ARRAY(tdata);

    writer->optimize();
    CuAssertTrue(tc, ramDir->sizeInBytes == ramDir->getRecomputedSizeInBytes(), _T("RAMDir size"));
    
    CuAssertEquals(tc, docsToAdd + (numThreads * (docsPerThread-1)), writer->docCount(), _T("document count"));

    writer->close();
    _CLLDELETE(writer);

    ramDir->close();
    _CLLDELETE(ramDir);
}

#if 0
  public void testSerializable() throws IOException {
    Directory dir = new RAMDirectory();
    ByteArrayOutputStream bos = new ByteArrayOutputStream(1024);
    assertEquals("initially empty", 0, bos.size());
    ObjectOutput out = new ObjectOutputStream(bos);
    int headerSize = bos.size();
    out.writeObject(dir);
    out.close();
    assertTrue("contains more then just header", headerSize < bos.size());
  } 
#endif

void testRAMDirectoryTearDown(CuTest * tc) {
    // cleanup 
    if (*indexDir) {
        // delete index dir + all files in it (portable way!)
    }

}
  
CuSuite *testRAMDirectory(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene RAMDirectory Test"));

    SUITE_ADD_TEST(suite, testRAMDirectorySetUp);

    SUITE_ADD_TEST(suite, testRAMDirectory);
    SUITE_ADD_TEST(suite, testRAMDirectoryString);
    SUITE_ADD_TEST(suite, testRAMDirectorySize);
 
    SUITE_ADD_TEST(suite, testRAMDirectoryTearDown);

    return suite;
}

