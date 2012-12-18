/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_SharedHeader.h"

TCHAR* lucene_i64tot(
    int64_t value, /* [I] Value to be converted */
    TCHAR* str,      /* [O] Destination for the converted value */
    int radix)      /* [I] Number base for conversion */
{
    uint64_t val;
    int negative;
    TCHAR buffer[65];
    TCHAR* pos;
    int digit;

	if (value < 0 && radix == 10) {
		negative = 1;
		val = -value;
	} else {
		negative = 0;
		val = value;
	} /* if */

	pos = &buffer[64];
	*pos = '\0';

	do {
		digit = (int)(val % radix);
		val = val / radix;
		if (digit < 10) {
			*--pos = '0' + digit;
		} else {
			*--pos = 'a' + digit - 10;
		} /* if */
	} while (val != 0L);

	if (negative) {
		*--pos = '-';
	} /* if */

    _tcsncpy(str,pos,&buffer[64] - pos + 1); //needed for unicode to work
    return str;
}
