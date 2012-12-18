/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#ifndef _lucene_examples_benchmark_stdafx_
#define _lucene_examples_benchmark_stdafx_

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
//#include <tchar.h>
#include <string.h>

#include "CLucene.h"
#include "CLucene/_clucene-config.h"
#include "CLucene/util/Misc.h"
#include "CLucene/store/RAMDirectory.h"

#define CLUCENE_DATA_LOCATION1 "../../src/test/data/"
#define CLUCENE_DATA_LOCATION2 "../src/test/data/"
#define CLUCENE_DATA_LOCATION3 "../../../src/test/data/"
#define CLUCENE_DATA_LOCATIONENV "srcdir"

extern const char* cl_tempDir;
extern char clucene_data_location[1024];

class Benchmarker;
#include "Timer.h"
#include "Unit.h"
#include "Benchmarker.h"

#endif
