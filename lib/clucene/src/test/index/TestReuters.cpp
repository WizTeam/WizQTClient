/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/util/dirent.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/LuceneThreads.h"
#include "CLucene/search/Explanation.h"
CL_NS_USE(search)

#ifdef _CL_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <algorithm>
using namespace std;

CL_NS_USE(util)

	//an extremelly simple analyser. this eliminates differences
	//caused by differences in character classifications functions
	class ReutersTokenizer:public CharTokenizer {
	public:
		// Construct a new LetterTokenizer.
		ReutersTokenizer(CL_NS(util)::Reader* in):
		  CharTokenizer(in) {}

	    ~ReutersTokenizer(){}
	protected:
		bool isTokenChar(const TCHAR c) const{
			if ( c == ' ' || c == '\t' ||
    		 c == '-' || c == '.' ||
    		 c == '\n' || c == '\r' ||
    		 c == ',' || c == '<' ||
    		 c == '>' || c<=9){
    			return false;
			}else
    			return true;
		}
		TCHAR normalize(const TCHAR c) const{
			return c;
		}
	};

	class ReutersAnalyzer: public Analyzer {
     public:
		 TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader){
			return _CLNEW ReutersTokenizer(reader);
		 }
         TokenStream* reusableTokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader)
         {
             Tokenizer* tokenizer = static_cast<Tokenizer*>(getPreviousTokenStream());
             if (tokenizer == NULL) {
                 tokenizer = _CLNEW ReutersTokenizer(reader);
                 setPreviousTokenStream(tokenizer);
             } else
                 tokenizer->reset(reader);
             return tokenizer;
         }
         virtual ~ReutersAnalyzer(){}
  };

  bool stringLowercaseCompare( const string &left, const string &right ){
   for( string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end() && rit != right.end(); ++lit, ++rit )
      if( tolower( *lit ) < tolower( *rit ) )
         return true;
      else if( tolower( *lit ) > tolower( *rit ) )
         return false;
   if( left.size() < right.size() )
      return true;
   return false;
  }




char reuters_fsdirectory[CL_MAX_PATH];
bool reuters_ready = false;

char reuters_srcdirectory[1024];
char reuters_origdirectory[1024];

//indexes the reuters-21578 data.
void testReuters(CuTest *tc) {
  strcpy(reuters_srcdirectory, clucene_data_location);
  strcat(reuters_srcdirectory, "/reuters-21578");
  CuAssert(tc,_T("Data does not exist"),Misc::dir_Exists(reuters_srcdirectory));

  strcpy(reuters_origdirectory, clucene_data_location);
  strcat(reuters_origdirectory, "/reuters-21578-index");
  CuAssert(tc,_T("Index does not exist"),Misc::dir_Exists(reuters_origdirectory));

  FSDirectory* fsdir = FSDirectory::getDirectory(reuters_fsdirectory);
  ReutersAnalyzer a;

  IndexWriter writer(fsdir,&a,true);
  writer.setUseCompoundFile(false);
  //NOTE: when comparing against indexes created <~= 1.9, there was always 1 more field
  //added then the actual limit...
  writer.setMaxFieldLength(10001);

  vector<string> files;
  CuAssertTrue(tc, Misc::listFiles(reuters_srcdirectory, files,false));
  sort(files.begin(), files.end(), stringLowercaseCompare);

  char tmppath[CL_MAX_DIR];
  strncpy(tmppath,reuters_srcdirectory,CL_MAX_DIR);
  strcat(tmppath,"/");
  char* tmppathP = tmppath + strlen(tmppath);
  TCHAR tpath[CL_MAX_PATH];
  struct cl_stat_t buf;

  vector<string>::iterator fl = files.begin();
  while ( fl != files.end() ){
    strcpy(tmppathP,fl->c_str());
    STRCPY_AtoT(tpath,fl->c_str(),CL_MAX_PATH);
    fileStat(tmppath,&buf);
    if ( buf.st_mode & S_IFREG){
      Document* doc = _CLNEW Document;
      doc->add(*_CLNEW Field(_T("path"),tpath,Field::INDEX_UNTOKENIZED | Field::STORE_YES));
      doc->add(*_CLNEW Field(_T("contents"), _CLNEW FileReader(tmppath, "ASCII"),Field::INDEX_TOKENIZED));

      writer.addDocument( doc );
      _CLDELETE(doc);

    }
    fl++;
  }

  writer.close();
  fsdir->close();
  _CLDECDELETE(fsdir);

  //note: for those comparing 0.9.16 to later, the optimize() has been removed so
  //we can do certain tests with the multi-* classes (reader,etc)
  //performance will naturally be worse

  reuters_ready = true;
}

void testBySection(CuTest* tc){
  IndexReader* reader1 = IndexReader::open(reuters_origdirectory);
  IndexReader* reader2 = IndexReader::open(reuters_fsdirectory);

  TestAssertIndexReaderEquals(tc,reader1,reader2);
  reader1->close();
  reader2->close();
  _CLDELETE(reader1);
  _CLDELETE(reader2);
}

#define threadsCount 10

void threadSearch(IndexSearcher* searcher, const TCHAR* qry, StandardAnalyzer* threadAnalyzer){
  Query* q = NULL;
  Hits* h = NULL;
  try{
    q = QueryParser::parse(qry , _T("contents"), threadAnalyzer);
    if ( q != NULL ){
      h = searcher->search( q );

      if ( h->length() > 0 ){
        //check for explanation memory leaks...
        CL_NS(search)::Explanation expl1;
        searcher->explain(q, h->id(0), &expl1);
        TCHAR* tmp = expl1.toString();
        _CLDELETE_CARRAY(tmp);
        if ( h->length() > 1 ){ //do a second one just in case
          CL_NS(search)::Explanation expl2;
          searcher->explain(q, h->id(1), &expl2);
          tmp = expl2.toString();
          _CLDELETE_CARRAY(tmp);
        }
      }
    }
  }_CLFINALLY(
              _CLDELETE(h);
              _CLDELETE(q);
  );
}
_LUCENE_THREAD_FUNC(threadedSearcherTest, arg){
  IndexSearcher* searcher = (IndexSearcher*)(((void**)arg)[0]);
  StandardAnalyzer* threadAnalyzer = (StandardAnalyzer*)(((void**)arg)[1]);

  for ( int i=0;i<100;i++ ){
    threadSearch(searcher, _T("test"), threadAnalyzer );
    threadSearch(searcher, _T("reuters"), threadAnalyzer );
    threadSearch(searcher, _T("data"), threadAnalyzer );
  }
  _LUCENE_THREAD_FUNC_RETURN(0);
}

void testThreaded(CuTest* tc){
  CLUCENE_ASSERT(reuters_ready);
  IndexSearcher searcher(reuters_origdirectory);

  //read using multiple threads...
  _LUCENE_THREADID_TYPE threads[threadsCount];

  int i;
  StandardAnalyzer threadAnalyzer;
  void* args[2];
  args[0] = &searcher;
  args[1] = &threadAnalyzer;

  for ( i=0;i<threadsCount;i++ ){
    threads[i] = _LUCENE_THREAD_CREATE(&threadedSearcherTest, args);
  }

  CL_NS(util)::Misc::sleep(3000);

  for ( i=0;i<threadsCount;i++ )
    _LUCENE_THREAD_JOIN(threads[i]);

  searcher.close();
}

CuSuite *testreuters(void)
{
  CuSuite *suite = CuSuiteNew(_T("CLucene Reuters Test"));

  //setup some variables
  strcpy(reuters_fsdirectory,cl_tempDir);
  strcat(reuters_fsdirectory,"/reuters-index");

  SUITE_ADD_TEST(suite, testReuters);
  SUITE_ADD_TEST(suite, testBySection);
  SUITE_ADD_TEST(suite, testThreaded);
  return suite;
}
// EOF
