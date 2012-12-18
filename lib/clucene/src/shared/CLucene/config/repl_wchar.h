/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_repl_wchar_h
#define _lucene_repl_wchar_h

#include <stdarg.h>
#include "repl_tchar.h"
#ifdef _CL_HAVE_STRING_H
    #include <string.h>
#endif
#ifdef _CL_HAVE_WCHAR_H
    #include <wchar.h>
#endif


CLUCENE_SHARED_EXPORT int cl_tcscasefoldcmp(const TCHAR * dst, const TCHAR * src);
CLUCENE_SHARED_EXPORT TCHAR* cl_tcscasefold( TCHAR * str, int len=-1 );

//we provide utf8 conversion functions
CLUCENE_SHARED_EXPORT size_t lucene_utf8towc  (wchar_t& ret, const char *s);
CLUCENE_SHARED_EXPORT size_t lucene_utf8towcs(wchar_t *, const char *,  size_t maxslen);
CLUCENE_SHARED_EXPORT size_t lucene_wctoutf8  (char * ret, const wchar_t  str);
CLUCENE_SHARED_EXPORT size_t lucene_wcstoutf8 (char *,  const wchar_t *, size_t maxslen);
#ifdef _ASCII
#define lucene_wcstoutf8string(str,strlen) str
#else
CLUCENE_SHARED_EXPORT std::string lucene_wcstoutf8string(const wchar_t* str, size_t strlen);
#endif
CLUCENE_SHARED_EXPORT size_t lucene_utf8charlen(const unsigned char p); //< the number of characters that this first utf8 character will expect

//string function replacements
#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS) || (defined(_UCS2) && !defined(_CL_HAVE_FUNCTION_WCSCASECMP)) || (defined(_ASCII) && !defined(_CL_HAVE_FUNCTION_STRCASECMP))
    CLUCENE_SHARED_EXPORT int lucene_tcscasecmp(const TCHAR *, const TCHAR *);
    #undef _tcsicmp
    #define _tcsicmp lucene_tcscasecmp
#endif
#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS) || (defined(_UCS2) && !defined(_CL_HAVE_FUNCTION_WCSLWR)) || (defined(_ASCII) && !defined(_CL_HAVE_FUNCTION_STRLWR))
    CLUCENE_SHARED_EXPORT TCHAR* lucene_tcslwr( TCHAR* str );
    #undef _tcslwr
    #define _tcslwr lucene_tcslwr
#endif

//conversion functions
#if (defined(_ASCII) && !defined(_CL_HAVE_FUNCTION_LLTOA)) || (defined(_UCS2) && !defined(_CL_HAVE_FUNCTION_LLTOW))
    CLUCENE_SHARED_EXPORT TCHAR* lucene_i64tot( int64_t value, TCHAR* str, int radix);
    #undef _i64tot
    #define _i64tot lucene_i64tot
#endif
#if !defined(_CL_HAVE_FUNCTION_WCSDUP)
    CLUCENE_SHARED_EXPORT wchar_t* lucene_wcsdup( const wchar_t* str);
#endif
#if (defined(_UCS2) && !defined(_CL_HAVE_FUNCTION_WCSTOLL)) || (defined(_ASCII) && !defined(_CL_HAVE_FUNCTION_STRTOLL))
	CLUCENE_SHARED_EXPORT int64_t lucene_tcstoi64(const TCHAR* str, TCHAR**end, int radix);
    #undef _tcstoi64
    #define _tcstoi64 lucene_tcstoi64
#endif
#if defined(_UCS2) && !defined(_CL_HAVE_FUNCTION_WCSTOD)
    CLUCENE_SHARED_EXPORT double lucene_tcstod(const TCHAR *value, TCHAR **end);
    #undef _tcstod
    #define _tcstod lucene_tcstod
#endif

//printf functions
#if (defined(_UCS2) && (!defined(_CL_HAVE_FUNCTION__SNWPRINTF)) || defined(_CL_HAVE_SNWPRINTF_BUG) )
    #undef _sntprintf
    #define _sntprintf lucene_snwprintf
    CLUCENE_SHARED_EXPORT int lucene_snwprintf(wchar_t* strbuf, size_t count, const wchar_t * format, ...);
#endif
#if defined(_UCS2) && !defined(_CL_HAVE_FUNCTION_WPRINTF)
    #undef _tprintf
    #define _tprintf lucene_wprintf
    CLUCENE_SHARED_EXPORT void lucene_wprintf(const wchar_t * format, ...);
#endif
#if defined(_UCS2) && (!defined(_CL_HAVE_FUNCTION__VSNWPRINTF) || defined(_CL_HAVE_SNWPRINTF_BUG) )
    #undef _vsntprintf
    #define _vsntprintf lucene_vsnwprintf
    CLUCENE_SHARED_EXPORT int lucene_vsnwprintf(wchar_t * strbuf, size_t count, const wchar_t * format, va_list& ap);
#endif



//todo: if _CL_HAVE_SNPRINTF_BUG fails(snprintf overflow),we should use our own
//function. but we don't have it currently, and our functions are dubious anyway...

#endif //end of _lucene_repl_wchar_h
