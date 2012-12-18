/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "DateField.h"
#include "CLucene/util/Misc.h"

CL_NS_USE(util)
CL_NS_DEF(document)

DateField::~DateField(){
}

TCHAR* DateField::timeToString(const int64_t time) {
    TCHAR* buf = _CL_NEWARRAY(TCHAR,DATEFIELD_DATE_LEN + 1);
    timeToString(time,buf);
    return buf;
}
void DateField::timeToString(const int64_t time, TCHAR* buf) {
    CND_PRECONDITION (buf, "buf == NULL");
    *buf = '\0';
	if (time < 0)
	  _CLTHROWA (CL_ERR_IllegalArgument,"time too early"); //todo: make richer error

	if (time > DATEFIELD_DATE_MAX)
	  _CLTHROWA (CL_ERR_IllegalArgument, "time too late (past DATEFIELD_DATE_MAX"); //todo: make richer error
	
	_i64tot(time, buf, 36);
	int32_t bufLen = _tcslen(buf);
	
	CND_PRECONDITION (bufLen <= DATEFIELD_DATE_LEN, "timeToString length is greater than 9");
	
	/* Supply leading zeroes if necessary. */
	if (bufLen < DATEFIELD_DATE_LEN) {
	  const int32_t nMissingZeroes = DATEFIELD_DATE_LEN - bufLen;
	  /* Move buffer contents forward to make room for leading zeroes. */
	  for (int32_t i = DATEFIELD_DATE_LEN - 1; i >= nMissingZeroes; i--)
	    buf[i] = buf[i - nMissingZeroes];
	  
	  /* Insert leading zeroes. */
	  {// MSVC6 scoping fix
	  for (int32_t i = 0; i < nMissingZeroes; i++)
	    buf[i] = '0';
	  }
	
	  buf[DATEFIELD_DATE_LEN] = 0;
	}
	
	CND_PRECONDITION (_tcslen(buf) == DATEFIELD_DATE_LEN, "timeToString return is not equal to DATEFIELD_DATE_LEN");
}

int64_t DateField::stringToTime(const TCHAR* time) {
	TCHAR* end;
	return _tcstoi64(time, &end, 36);
}

CL_NS_END
