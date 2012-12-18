/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef lucene_stdheader_h
#define lucene_stdheader_h

/**
* This header contains public distributed code that needs to be included *before*
* any clucene-core code is included. It also uses the clucene-shared header
* which contains platform specific code configured by cmake.
*/

//configurations for library
#include "CLucene/CLConfig.h"

//shared header
#include "CLucene/SharedHeader.h"

//error handling macros/functions
#include "CLucene/debug/error.h"

//todo: would be good to deprecate this... it's ugly
#define StringArrayWithDeletor       CL_NS(util)::CLVector<TCHAR*, CL_NS(util)::Deletor::tcArray >
#define StringArray                  std::vector<TCHAR*>
#define StringArrayWithDeletor       CL_NS(util)::CLVector<TCHAR*, CL_NS(util)::Deletor::tcArray >
#define StringArrayConst             std::vector<const TCHAR*>
//#define StringArrayConstWithDeletor  CL_NS(util)::CLVector<const TCHAR*, CL_NS(util)::Deletor::tcArray >

#define AStringArray                 std::vector<char*>
#define AStringArrayWithDeletor      CL_NS(util)::CLVector<char*, CL_NS(util)::Deletor::acArray >
#define AStringArrayConst            std::vector<const char*>
//#define AStringArrayConstWithDeletor CL_NS(util)::CLVector<const char*, CL_NS(util)::Deletor::acArray >

//call this at the end of running to clean up memory. 
extern CLUCENE_EXPORT void _lucene_shutdown();

#endif // lucene_apiheader_h
