/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_SharedHeader.h"
#include "CLucene/util/StringBuffer.h"

#include <limits.h> //MB_LEN_MAX

CL_NS_USE(util)

//print a variable argument to a stream
//currently special number formatting is not supported. it is very minimalistic
void lucene_vfnwprintf(StringBuffer* buffer, size_t /*count*/, const wchar_t * format, va_list& valist){
	const wchar_t *iter = format;
	StringBuffer* tmp = NULL;
	if ( buffer == NULL )
		tmp = _CLNEW StringBuffer;
	else
		tmp = buffer;

	while (*iter)
	{
		while (*iter && *iter != '%')
		{
			tmp->appendChar(*iter++);
		}
		if (*iter == '%')
		{
			if (iter[1] == '%')
			{
				//just print a %
				tmp->appendChar('%');
				iter += 2;
				continue;
			}

			iter++;
			switch (*iter)
			{
				case 's':
				{
					//todo: this is faulty. it doesn't heed count

					//print a string or null
					const TCHAR *wstr = va_arg(valist, TCHAR *);
					if ( !wstr )
						wstr = _T("(null)");

					tmp->append(wstr);
					iter++;
					break;
				}

				case 'c':
					tmp->appendChar((TCHAR)va_arg(valist, int));
					iter++;
					break;

				default:
				{
					//todo: this is faulty. it doesn't heed count

					if (*iter == 'p')
						tmp->appendInt((int32_t)va_arg(valist, long));
					else
					{
						if (*iter == 'a' || *iter == 'A' ||
							*iter == 'e' || *iter == 'E' ||
							*iter == 'f' || *iter == 'F' || 
							*iter == 'g' || *iter == 'G')
							tmp->appendFloat((float_t)va_arg(valist, double),8);
						else if (*iter == 'd' || *iter == 'i' ){
							tmp->appendInt((int32_t)va_arg(valist, int));
						}else if (*iter == 'l' ){
							TCHAR b[100];
							_i64tot((int64_t)va_arg(valist, int64_t),b,10);
							tmp->append(b);
						}/*else{
							TCHAR b[100];
							_i64tot((int64_t)va_arg(valist, void*),b,10);
							tmp->append(b);
						}*/
					}
					iter++;
					break;
				}
			}
		}
	}

	
	if ( buffer == NULL ){
		//we are supposed to be writing to the console
#ifdef _UCS2
		TCHAR* pointer = tmp->getBuffer();
		char ob[MB_LEN_MAX];
		size_t v;
		size_t len = tmp->length();
		for (size_t i=0;i<len;i++){
			v = wctomb(ob,*pointer);
			if ( v > 0 ){
				ob[v]='\0';
				fputs(ob,stdout);
			}
			pointer++;
		}
		

#else
		fputs(tmp->getBuffer(),stdout);
#endif
		_CLDELETE(tmp);
	}
}

#ifdef _UCS2
//print a list of arguments to a string
int lucene_snwprintf(wchar_t* strbuf, size_t count, const wchar_t * format, ...){
	va_list ap;
  va_start(ap, format);
	StringBuffer buffer;
  lucene_vfnwprintf(&buffer,count,format,ap);
  va_end(ap);

	size_t ret = cl_min(count,(size_t)(buffer.length()+1));
	wcsncpy(strbuf,buffer.getBuffer(),ret);
  return ret;
}

//print a list of arguments to the stdout
void lucene_wprintf(const wchar_t * format, ...){
	va_list ap;
    va_start(ap, format);
	lucene_vfnwprintf(NULL,LUCENE_INT32_MAX_SHOULDBE,format,ap);
    va_end(ap);
}

//print a variable argument to a string
int lucene_vsnwprintf(wchar_t * strbuf, size_t count, const wchar_t * format, va_list& ap){
	StringBuffer buffer;
    lucene_vfnwprintf(&buffer,count,format,ap);
	int ret = cl_min((size_t)count,buffer.length()+1);
	wcsncpy(strbuf,buffer.getBuffer(),ret);
    return ret;
}
#endif
