/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
 
#include "CLucene/_SharedHeader.h"

int lucene_tcscasecmp(const TCHAR * sa, const TCHAR * sb){
    TCHAR ca,cb;
    if (sa == sb)
    	return 0;
    
    do{
        ca = _totlower( (*(sa++)) );
        cb = _totlower( (*(sb++)) );
    } while ( ca != L'\0' && (ca == cb) );
    
    return (int)(ca - cb);
}
