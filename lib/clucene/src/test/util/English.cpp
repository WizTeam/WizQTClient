/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
CL_NS_USE(util)

void English::IntToEnglish(int32_t i, StringBuffer* result)
{
	if (i == 0)
	{
		result->append(_T("zero"));
		return ;
	}
	if (i < 0)
	{
		result->append(_T("minus "));
		i = - i;
	}
	if (i >= 1000000000)
	{
		// billions
		IntToEnglish(i / 1000000000, result);
		result->append(_T("billion, "));
		i = i % 1000000000;
	}
	if (i >= 1000000)
	{
		// millions
		IntToEnglish(i / 1000000, result);
		result->append(_T("million, "));
		i = i % 1000000;
	}
	if (i >= 1000)
	{
		// thousands
		IntToEnglish(i / 1000, result);
		result->append(_T("thousand, "));
		i = i % 1000;
	}
	if (i >= 100)
	{
		// hundreds
		IntToEnglish(i / 100, result);
		result->append(_T("hundred "));
		i = i % 100;
	}
	if (i >= 20)
	{
		switch (i / 10)
		{
			
			case 9:  result->append(_T("ninety")); break;
			
			case 8:  result->append(_T("eighty")); break;
			
			case 7:  result->append(_T("seventy")); break;
			
			case 6:  result->append(_T("sixty")); break;
			
			case 5:  result->append(_T("fifty")); break;
			
			case 4:  result->append(_T("forty")); break;
			
			case 3:  result->append(_T("thirty")); break;
			
			case 2:  result->append(_T("twenty")); break;
			}
		i = i % 10;
		if (i == 0)
			result->append(_T(" "));
		else
			result->append(_T("-"));
	}
	switch (i)
	{
		
		case 19:  result->append(_T("nineteen ")); break;
		
		case 18:  result->append(_T("eighteen ")); break;
		
		case 17:  result->append(_T("seventeen ")); break;
		
		case 16:  result->append(_T("sixteen ")); break;
		
		case 15:  result->append(_T("fifteen ")); break;
		
		case 14:  result->append(_T("fourteen ")); break;
		
		case 13:  result->append(_T("thirteen ")); break;
		
		case 12:  result->append(_T("twelve ")); break;
		
		case 11:  result->append(_T("eleven ")); break;
		
		case 10:  result->append(_T("ten ")); break;
		
		case 9:  result->append(_T("nine ")); break;
		
		case 8:  result->append(_T("eight ")); break;
		
		case 7:  result->append(_T("seven ")); break;
		
		case 6:  result->append(_T("six ")); break;
		
		case 5:  result->append(_T("five ")); break;
		
		case 4:  result->append(_T("four ")); break;
		
		case 3:  result->append(_T("three ")); break;
		
		case 2:  result->append(_T("two ")); break;
		
		case 1:  result->append(_T("one ")); break;
		
		case 0:  result->append(_T("")); break;
	}
}

TCHAR* English::IntToEnglish(int32_t i)
{
	StringBuffer result;
	IntToEnglish(i, &result);
	return result.toString();
}
void English::IntToEnglish(int32_t i, TCHAR* buf, int32_t buflen)
{
	StringBuffer result;
	IntToEnglish(i, &result);
	_tcsncpy(buf,result.getBuffer(),buflen);
}

