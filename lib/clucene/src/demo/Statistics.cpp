/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include <iostream>
#include <stdio.h>

#include "CLucene/StdHeader.h"
#include "CLucene/_clucene-config.h"

#include "CLucene.h"
#include "CLucene/config/repl_tchar.h"
#include "CLucene/config/repl_wchar.h"
#include "CLucene/util/Misc.h"

using namespace std;
using namespace lucene::analysis;
using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::search;

void getStats(const char* directory){

	IndexReader* r = IndexReader::open(directory);
	printf("Statistics for %s\n", directory);
	printf("==================================\n");

	printf("Max Docs: %d\n", r->maxDoc() );
	printf("Num Docs: %d\n", r->numDocs() );
	
    int64_t ver = r->getCurrentVersion(directory);
    _tprintf(_T("Current Version: %f\n"), (float_t)ver );
	
	TermEnum* te = r->terms();
	int32_t nterms;
	for (nterms = 0; te->next() == true; nterms++) {
            /* empty */
    } 
	printf("Term count: %d\n\n", nterms );
	_CLLDELETE(te);

	r->close();
	_CLLDELETE(r);
}
