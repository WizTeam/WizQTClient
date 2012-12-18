/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "CLucene/_SharedHeader.h"

int64_t lucene_tcstoi64(const TCHAR* str, TCHAR**end, int radix){
	#define LUCENE_TCSTOI64_RADIX(x,r) ((x>=_T('0') && x<=_T('9'))?x-_T('0'):((x>=_T('a') && x<=_T('z'))?x-_T('a')+10:((x>=_T('A') && x<=_T('Z'))?x-_T('A')+10:1000)))

	if (radix < 2 || radix > 36) 
		return 0;

	/* Skip white space.  */
	while (_istspace (*str))
		++str;

	int sign=1;
	if ( str[0] == _T('+') )
		str++;
	else if ( str[0] == _T('-') ){
		sign = -1;
		str++;
	}
		
  *end=(TCHAR*)str;
  long r = -1;
  while ( (r=LUCENE_TCSTOI64_RADIX(*end[0],radix)) >=0 && r<radix )
      (*end)++;

  TCHAR* p = (*end)-1;
  int64_t ret = 0;
  int pos = 0;
  for ( ;p>=str;p-- ){
      int i=LUCENE_TCSTOI64_RADIX(p[0],radix);
      if ( pos == 0 )
          ret=i;
      else
          ret += (int64_t)pow((float_t)radix,(float_t)pos) * i; //todo: might be quicker with a different pow overload

      pos++;
  }
  return sign*ret;
}
