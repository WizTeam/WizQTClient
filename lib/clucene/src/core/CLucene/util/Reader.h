/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_Reader_
#define _lucene_util_Reader_

#include "CLucene/util/CLStreams.h"
CL_NS_DEF(util)

#error Reader has been refactored. It is recommended that you use strigi streams
#error for all input into CLucene. If, however, you dont want to use that dependency, 
#error then you'll have to refactor your current code. The jstreams namespace
#error was completely removed

CL_NS_END
#endif
