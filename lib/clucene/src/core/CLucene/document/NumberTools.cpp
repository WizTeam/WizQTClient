/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "NumberTools.h"
#include "CLucene/util/Misc.h"

CL_NS_DEF(document)

const TCHAR* NumberTools::MIN_STRING_VALUE = NEGATIVE_PREFIX _T("0000000000000");
const TCHAR* NumberTools::MAX_STRING_VALUE = POSITIVE_PREFIX _T("1y2p0ij32e8e7");

TCHAR* NumberTools::longToString(int64_t l)
{
	if (l == LUCENE_INT64_MIN_SHOULDBE) {
		// special case, because long is not symetric around zero
		return stringDuplicate(MIN_STRING_VALUE);
	}

	TCHAR* buf = _CL_NEWARRAY(TCHAR, STR_SIZE + 1);
	if (l < 0) {
		buf[0] = NEGATIVE_PREFIX[0];
		l = LUCENE_INT64_MAX_SHOULDBE + l + 1;
	} else {
		buf[0] = POSITIVE_PREFIX[0];
	}

	TCHAR tmp[STR_SIZE];
	_i64tot(l, tmp, NUMBERTOOLS_RADIX);
	size_t len = _tcslen(tmp);
	_tcscpy(buf+(STR_SIZE-len),tmp);
	for ( size_t i=1;i<STR_SIZE-len;i++ )
		buf[i] = (int)'0';

	buf[STR_SIZE] = 0;

	return buf;
}

int64_t NumberTools::stringToLong(const TCHAR* str) {
	if (str == NULL) {
		_CLTHROWA(CL_ERR_NullPointer,"string cannot be null");
	}
	if (_tcslen(str) != STR_SIZE) {
		_CLTHROWA(CL_ERR_NumberFormat,"string is the wrong size");
	}

	if (_tcscmp(str, MIN_STRING_VALUE) == 0) {
		return LUCENE_INT64_MIN_SHOULDBE;
	}

	TCHAR prefix = str[0];

	TCHAR* sentinel = NULL;
	int64_t l = _tcstoi64(++str, &sentinel, NUMBERTOOLS_RADIX);

	if (prefix == POSITIVE_PREFIX[0]) {
		// nop
	} else if (prefix == NEGATIVE_PREFIX[0]) {
		l = l - LUCENE_INT64_MAX_SHOULDBE - 1;
	} else {
		_CLTHROWA(CL_ERR_NumberFormat,"string does not begin with the correct prefix");
	}

	return l;
}

NumberTools::~NumberTools(){
}

CL_NS_END
