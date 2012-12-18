/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "TestCLString.h"

#ifdef COMPILER_MSVC
#ifdef _DEBUG
	#define CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif
#endif

#include <stdlib.h>

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

using namespace std;
using namespace lucene::util;

const char* cl_tempDir;
char clucene_data_location[1024];

int main( int argc, char** argv ){
	//Dumper Debug
	#ifdef COMPILER_MSVC
	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );//| _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_CHECK_CRT_DF );
	#endif
	#endif

	Benchmarker bench;
	TestCLString clstring;
	bool ret_result = false;

	cl_tempDir = NULL;
	if ( Misc::dir_Exists("/tmp") )
		cl_tempDir = "/tmp";
	if ( getenv("TEMP") != NULL )
		cl_tempDir = getenv("TEMP");
	else if ( getenv("TMP") != NULL )
		cl_tempDir = getenv("TMP");
	
	char* tmp = _CL_NEWARRAY(char,strlen(cl_tempDir)+9);
	strcpy(tmp,cl_tempDir);
	strcat(tmp,"/clucene");
	_mkdir(tmp);
	if ( Misc::dir_Exists(tmp) )
		cl_tempDir=tmp;


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


	bench.Add(&clstring);
	ret_result = bench.run();



exit_point:
	_lucene_shutdown(); //clears all static memory
    //print lucenebase debug
   
	return ret_result ? 0 : 1;

	//Debuggin techniques:
	//For msvc, use this for breaking on memory leaks: 
	//	_crtBreakAlloc
	//for linux, use valgrind
}
