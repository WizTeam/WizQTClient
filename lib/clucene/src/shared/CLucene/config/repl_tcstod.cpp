/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
 
#include "CLucene/_SharedHeader.h"
#include "CLucene/util/Misc.h"

#ifndef _ASCII
double lucene_tcstod(const TCHAR *value, TCHAR **end){
    int32_t len = _tcslen(value)+1;
    char* avalue=_CL_NEWARRAY(char,len);
    char* aend=NULL;
    STRCPY_TtoA(avalue,value,len);
    
    double ret = strtod(avalue,&aend);
    *end=(TCHAR*)value+(aend-avalue);
    _CLDELETE_CaARRAY(avalue);

    return ret;
}
#endif
