/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
//msvc test for memory leaks:
#ifdef _MSC_VER
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <stdlib.h>
		#include <crtdbg.h>
	#endif
#endif

#include "test.h"
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#ifdef _CL_HAVE_DIRECT_H
	#include <direct.h>
#endif
#ifdef _CL_HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef _CL_HAVE_IO_H
	#include <io.h>
#endif

std::string cl_tempDirS;
const char* cl_tempDir;
bool cl_quiet;
char clucene_data_location[1024];

int main(int argc, char *argv[])
{
	#ifdef _MSC_VER
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF
		_crtBreakAlloc=-1;
	#endif
	#endif

	#ifdef DMALLOC
		if ( getenv("DMALLOC_OPTIONS") == NULL ){
			dmalloc_debug_setup("low,log=dmalloc.log.txt");
		}else{
			//apparently cygwin has to have this code....
			dmalloc_debug_setup(getenv("DMALLOC_OPTIONS"));
		}
	#endif


	int ret_result = 0;
	int i=0;
	int exclude = 0;
	int list_provided = 0;
	CuSuiteList *alltests = NULL;
	CuString *output = CuStringNew();
	bool silent = false;
	bool verbose = false;
	bool times = true;
	uint64_t startTime=0;

	if ( Misc::dir_Exists("/tmp") )
		cl_tempDirS = "/tmp";
	if ( getenv("TEMP") != NULL )
		cl_tempDirS = getenv("TEMP");
	else if ( getenv("TMP") != NULL )
		cl_tempDirS = getenv("TMP");

  if ( Misc::dir_Exists( (cl_tempDirS + "/clucene").c_str() ) )
		cl_tempDirS += "/clucene";
  cl_tempDir = cl_tempDirS.c_str();

	clucene_data_location[0]=0;
	if ( CL_NS(util)::Misc::dir_Exists(CLUCENE_DATA_LOCATION1 "/reuters-21578-index/segments") )
		strcpy(clucene_data_location, CLUCENE_DATA_LOCATION1);
	else if ( CL_NS(util)::Misc::dir_Exists(CLUCENE_DATA_LOCATION2 "/reuters-21578-index/segments") )
		strcpy(clucene_data_location, CLUCENE_DATA_LOCATION2);
	else if ( CL_NS(util)::Misc::dir_Exists(CLUCENE_DATA_LOCATION3 "/reuters-21578-index/segments") )
		strcpy(clucene_data_location, CLUCENE_DATA_LOCATION3);
	else if ( getenv(CLUCENE_DATA_LOCATIONENV) != NULL ){
		strcpy(clucene_data_location,getenv(CLUCENE_DATA_LOCATIONENV));
		strcat(clucene_data_location,"/data/reuters-21578-index/segments");
		if ( CL_NS(util)::Misc::dir_Exists( clucene_data_location ) ){
			strcpy(clucene_data_location, getenv(CLUCENE_DATA_LOCATIONENV));
			strcat(clucene_data_location, "/data");
		}else
			clucene_data_location[0]=0;
	}

	/* first check that we are running the test for the correct position */
	//todo: make this configurable
	if ( !*clucene_data_location ){
		fprintf(stderr,"%s must be run from a subdirectory of the application's root directory\n",argv[0]);
		fprintf(stderr,"ensure that the test data exists in %s or %s or %s\n",CLUCENE_DATA_LOCATION1, CLUCENE_DATA_LOCATION2, CLUCENE_DATA_LOCATION3);
		if ( getenv(CLUCENE_DATA_LOCATIONENV) != NULL )
			fprintf(stderr,"%s/data was also checked because of the " CLUCENE_DATA_LOCATIONENV " environment variable", getenv(CLUCENE_DATA_LOCATIONENV));
		ret_result = 1;
		goto exit_point;
	}

    CuInit(argc, argv);

    /* see if we're in exclude mode, see if list of testcases provided */
    for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i],"-h") || !strcmp(argv[i],"--help") || !strcmp(argv[i],"/?")){
			printf("%s [-l list] [-q quiet] [-v verbose] [-t show times] [-p printf messages immediatelly] [-x exclude] [tests...]\n",argv[0]);
			goto exit_point;
		}
        if (!strcmp(argv[i], "-v")) {
			   verbose = true;
            continue;
        }
        if (!strcmp(argv[i], "-p")) { //used in CuInit
            continue;
        }
		  if (!strcmp(argv[i], "-q")) {
			   silent = true;
            continue;
        }
        if (!strcmp(argv[i], "-x")) {
            exclude = 1;
            continue;
        }
        if (!strcmp(argv[i], "-t")) {
            times = true;
            continue;
        }
        if (!strcmp(argv[i], "-l")) {
            for (i = 0; tests[i].func != NULL; i++) {
                printf("%s\n", tests[i].testname);
            }
            ret_result = 0;
			goto exit_point;
        }
        if (argv[i][0] == '-') {
            fprintf(stderr, "invalid option: `%s'\n", argv[i]);
            ret_result = 1;
			goto exit_point;
        }
        list_provided = 1;
    }

    if (!list_provided) {
        /* add everything */
        alltests = CuSuiteListNew(_T("All CLucene Tests"));
        for (i = 0; tests[i].func != NULL; i++) {
            CuSuiteListAdd(alltests, tests[i].func());
        }
    }
    else if (exclude) {
        /* add everything but the tests listed */
        alltests = CuSuiteListNew(_T("Partial CLucene Tests"));
        for (i = 0; tests[i].func != NULL; i++) {
            int this_test_excluded = 0;
            int j;

            for (j = 1; j < argc && !this_test_excluded; j++) {
                if (!strcmp(argv[j], tests[i].testname)) {
                    this_test_excluded = 1;
                }
            }
            if (!this_test_excluded) {
                CuSuiteListAdd(alltests, tests[i].func());
            }
        }
    }
    else {
        /* add only the tests listed */
        alltests = CuSuiteListNew(_T("Partial CLucene Tests"));
        for (i = 1; i < argc; i++) {
            int j;
            int found = 0;

            if (argv[i][0] == '-') {
                continue;
            }
            for (j = 0; tests[j].func != NULL; j++) {
                if (!strcmp(argv[i], tests[j].testname)) {
                    CuSuiteListAdd(alltests, tests[j].func());
                    found = 1;
                }
            }
            if (!found) {
                fprintf(stderr, "invalid test name: `%s'\n", argv[i]);
                ret_result = 1;
				goto exit_point;
            }
        }
    }

	startTime = Misc::currentTimeMillis();

	printf("Key: .= pass N=not implemented F=fail\n");
	if ( silent )
		CuSuiteListRun(alltests);
	else
		CuSuiteListRunWithSummary(alltests,verbose,times);
    i = CuSuiteListDetails(alltests, output);
    _tprintf(_T("%s\n"), output->buffer);

	if ( times )
		printf("Tests run in %dms\n\n", (int32_t)(CL_NS(util)::Misc::currentTimeMillis()-startTime));

exit_point:
	if ( alltests != NULL )
		CuSuiteListDelete(alltests);
	CuStringFree(output);

	_lucene_shutdown(); //clears all static memory

	if ( ret_result != 0 )
		return ret_result;
	else
		return i > 0 ? 1 : 0;

	//Debuggin techniques:
	//For msvc, use this for breaking on memory leaks:
	//	_crtBreakAlloc
	//for linux, use valgrind
}


void TestAssertIndexReaderEquals(CuTest *tc,  IndexReader* index1, IndexReader* index2){
  const Document::FieldsType* fields1, *fields2;
  Document::FieldsType::const_iterator it1, it2;

  CuAssertPtrNotNull(tc, _T("check index1!=null"), index1);
  CuAssertPtrNotNull(tc, _T("check index1!=null"), index2);

  //misc
  CuAssertIntEquals(tc,_T("IndexReaders have different values for numDocs"), index1->numDocs(), index2->numDocs());
  CuAssertIntEquals(tc,_T("IndexReaders have different values for maxDoc"), index1->maxDoc(), index2->maxDoc());
  CuAssertIntEquals(tc,_T("Only one IndexReader has deletions"), index1->hasDeletions(), index2->hasDeletions());
  CuAssertIntEquals(tc,_T("Only one IndexReader is optimized"), index1->isOptimized(), index2->isOptimized());

  //test field names
  StringArrayWithDeletor fn1;
  StringArrayWithDeletor fn2;
  index1->getFieldNames(IndexReader::ALL, fn1);
  index2->getFieldNames(IndexReader::ALL, fn2);

  //make sure field length is the same
  int fn1count = fn1.size();
  int fn2count = fn2.size();
  CuAssertIntEquals(tc, _T("reader fieldnames count not equal"), fn1count, fn2count );
  for (int n=0;n<fn1count;n++ ){
    //field names aren't always in the same order, so find it.
    int fn2n = 0;
    bool foundField = false;
    while ( fn2[fn2n] != NULL ){
        if ( _tcscmp(fn1[n],fn2[fn2n])==0 ){
            foundField = true;
            break;
        }
        fn2n++;
    }
    CLUCENE_ASSERT( foundField==true );

    //test field norms
    uint8_t* norms1 = index1->norms(fn1[n]);
    uint8_t* norms2 = index2->norms(fn1[n]);
    if ( norms1 != NULL ){
      CLUCENE_ASSERT(norms2 != NULL);
      for ( int i=0;i<index1->maxDoc();i++ ){
        int diff = norms1[i]-norms2[i];
        if ( diff < 0 )
            diff *= -1;
        if ( diff > 16 ){
                      TCHAR tmp[1024];
                      _sntprintf(tmp,1024,_T("Norms are off by more than the threshold! %d, should be %d"), (int32_t)norms2[i], (int32_t)norms1[i]);
          CuAssert(tc,tmp,false);
        }
      }
    }else
      CLUCENE_ASSERT(norms2 == NULL);
    ////////////////////
  }
  fn1.clear(); //save memory
  fn2.clear(); //save memory


  // check deletions
  for (int i = 0; i < index1->maxDoc(); i++) {
    CuAssertIntEquals( tc, _T("only deleted in one index."), index1->isDeleted(i), index2->isDeleted(i));
  }


  // check stored fields
  Document doc1;
  Document doc2;
  for (int i = 0; i < index1->maxDoc(); i++) {
    if (!index1->isDeleted(i)) {
      doc1.clear(); doc2.clear();
      index1->document(i, doc1);
      index2->document(i, doc2);
      fields1 = doc1.getFields();
      fields2 = doc2.getFields();
      CuAssertIntEquals(tc, _T("Different numbers of fields for doc "), fields1->size(), fields2->size());
      it1 = fields1->begin();
      it2 = fields2->begin();
      while ( it1 != fields1->end() ) {
        Field* curField1 = *it1;
        Field* curField2 = *it2;
        CuAssertStrEquals( tc, _T("Different fields names for doc "), curField1->name(), curField2->name());
        CuAssertStrEquals( tc, _T("Different field values for doc "), curField1->stringValue(), curField2->stringValue());
        it1++;
        it2++;
      }
    }
  }


  // check dictionary and posting lists
  TermEnum* enum1 = index1->terms();
  TermEnum* enum2 = index2->terms();
  TermPositions* tp1 = index1->termPositions();
  TermPositions* tp2 = index2->termPositions();
  while(enum1->next()) {

    CuAssertTrue(tc,enum2->next());
    CuAssertStrEquals(tc, _T("Different term field in dictionary."), enum1->term(false)->field(), enum2->term(false)->field() );
    CuAssertStrEquals(tc, _T("Different term field in dictionary."), enum1->term(false)->text(), enum2->term(false)->text() );
    CuAssert(tc, _T("Different term in dictionary."), enum1->term(false)->equals(enum2->term(false)) );

    tp1->seek(enum1->term(false));
    tp2->seek(enum1->term(false));
    while(tp1->next()) {
      CuAssertTrue(tc, tp2->next());
      CuAssertIntEquals(tc,_T("Different doc id in postinglist of term"), tp1->doc(), tp2->doc());
      CuAssertIntEquals(tc,_T("Different term frequence in postinglist of term"), tp1->freq(), tp2->freq());
      for (int i = 0; i < tp1->freq(); i++) {
        CuAssertIntEquals(tc,_T("Different positions in postinglist of term"), tp1->nextPosition(), tp2->nextPosition());
      }
    }
  }
  _CLDELETE(enum1);
  _CLDELETE(enum2);
  _CLDELETE(tp1);
  _CLDELETE(tp2);
}

