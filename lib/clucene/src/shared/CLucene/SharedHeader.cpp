/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "CLucene/_SharedHeader.h"


#ifdef _CL_HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif


const TCHAR* _LUCENE_BLANK_STRING=_T("");
const char* _LUCENE_BLANK_ASTRING="";


#if defined(_ASCII)
  #if defined(_LUCENE_PRAGMA_WARNINGS)
	 #pragma message ("==================Using ascii mode!!!==================")
	#else
	 #warning "==================Using ascii mode!!!=================="
	#endif
#endif

wchar_t* lucene_wcsdup( const wchar_t* str ){
	size_t len = wcslen(str);
	wchar_t* ret = (wchar_t*)malloc( (len + 1) * sizeof(wchar_t) );
	wcscpy(ret, str);
	return ret;
}
