/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef TEST_H
#define TEST_H
#include "CLucene.h"
#include "CLucene/_clucene-config.h"
#include "CLucene/config/repl_tchar.h"
#include "CLucene/config/repl_wchar.h"
#include "CLucene/debug/_condition.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/Misc.h"

#include "CLucene/store/RAMDirectory.h"
#include "CLucene/store/Lock.h"
#include "CLucene/index/TermVector.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#define LUCENE_INT64_MAX_SHOULDBE _ILONGLONG(0x7FFFFFFFFFFFFFFF)
#define LUCENE_INT64_MIN_SHOULDBE (-LUCENE_INT64_MAX_SHOULDBE - _ILONGLONG(1) )

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(search)
CL_NS_USE(document)
CL_NS_USE(queryParser)
CL_NS_USE(analysis)
CL_NS_USE2(analysis, standard)

#include "CuTest.h"

CuSuite *testsnowball(void);
CuSuite *testhighlighter(void);
CuSuite *teststreams(void);
CuSuite *testutf8(void);
CuSuite *testanalysis(void);

extern unittest tests[];

#define CLUCENE_DATA_LOCATION1 "../../src/test/data/"
#define CLUCENE_DATA_LOCATION2 "../src/test/data/"
#define CLUCENE_DATA_LOCATION3 "../../../src/test/data/"
#define CLUCENE_DATA_LOCATIONENV "srcdir"
extern const char* cl_tempDir;

#endif
