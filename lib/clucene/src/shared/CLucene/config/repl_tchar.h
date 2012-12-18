/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _REPL_TCHAR_H
#define _REPL_TCHAR_H

#ifndef _CL_HAVE_TCHAR_H
    #if defined(_UCS2)

        //note: descriptions with * in front have replacement functions

        //formatting functions
        #define _sntprintf swprintf //* make a formatted a string
        #define _tprintf wprintf //* print a formatted string

        //this one has no replacement functions yet, but it is only used in the tests
        #define _vsntprintf vsnwprintf //* print a formatted string using variable arguments

        //we are using the internal functions of the compiler here
        //if LUCENE_USE_INTERNAL_CHAR_FUNCTIONS is defined, thesse
        //will be replaced by internal functions
        #define _istalnum iswalnum //* alpha/numeric char check
        #define _istalpha iswalpha //* alpha char check
        #define _istspace iswspace //* space char check
        #define _istdigit iswdigit //* digit char check
        #define _totlower towlower //* convert char to lower case
        #define _totupper towupper //* convert char to lower case
        #define _tcslwr wcslwr //* convert string to lower case

        //these are the string handling functions
        //we may need to create wide-character/multi-byte replacements for these
        #define _tcscpy wcscpy //copy a string to another string
        #define _tcsncpy wcsncpy //copy a specified amount of one string to another string.
        #define _tcscat wcscat //copy a string onto the end of the other string
    		#define _tcsncat wcsncat
        #define _tcschr wcschr //find location of one character
        #define _tcsstr wcsstr //find location of a string
        #define _tcslen wcslen //get length of a string
        #define _tcscmp wcscmp //case sensitive compare two strings
        #define _tcsncmp wcsncmp //case sensitive compare two strings
        #define _tcscspn wcscspn //location of any of a set of character in a string

				//string compare
        #ifdef _CL_HAVE_FUNCTION_WCSICMP
            #define _tcsicmp wcsicmp //* case insensitive compare two string
        #else
            #define _tcsicmp wcscasecmp //* case insensitive compare two string
        #endif
				#if defined(_CL_HAVE_FUNCTION_WCSDUP)
			  	#define _tcsdup	wcsdup
			  #else
			  	#define _tcsdup	lucene_wcsdup
			  #endif

        //conversion functions
        #define _tcstod wcstod //convert a string to a double
        #define _tcstoi64 wcstoll //* convers a string to an 64bit bit integer
        #define _itot _i64tot
        #define _i64tot lltow //* converts a 64 bit integer to a string (with base)
    #else //if defined(_ASCII)

        //formatting functions
        #define _sntprintf snprintf
        #define _tprintf printf
        #define _vsntprintf vsnprintf

        //we are using the internal functions of the compiler here
        //if LUCENE_USE_INTERNAL_CHAR_FUNCTIONS is defined, thesse
        //will be replaced by internal functions
        #define _istalnum isalnum
        #define _istalpha isalpha
        #define _istspace isspace
        #define _istdigit isdigit
        #define _totlower tolower
        #define _totupper toupper
        #define _tcslwr strlwr

        //these are the string handling functions
        #define _tcscpy strcpy
        #define _tcsncpy strncpy
        #define _tcscat strcat
    		#define _tcsncat strncat
        #define _tcschr strchr
        #define _tcsstr strstr
        #define _tcslen strlen
        #define _tcscmp strcmp
        #define _tcsncmp strncmp
        #define _tcsicmp strcasecmp
        #define _tcscspn strcspn
        #define _tcsdup strdup //string duplicate
        //converstion methods
        #define _tcstod strtod
        #define _tcstoi64 strtoll
        #define _itot _i64tot 
        #define _i64tot lltoa

    #endif

#else //HAVE_TCHAR_H
    #include <tchar.h>

    //some tchar headers miss these...
    #ifndef _tcstoi64
        #if defined(_UCS2)
        	#define _tcstoi64 wcstoll //* convers a string to an 64bit bit integer
        #else
        	#define _tcstoi64 strtoll
        #endif
    #endif

#endif //HAVE_TCHAR_H

#ifndef _ttoi
  #define _ttoi(x) (int)_tcstoi64(x,NULL,10)
#endif

#ifndef _itot
  #define _itot(i, buf, radix) lucene_i64tot(i, buf, radix)
#endif

namespace std
{
#ifndef tstring
  #ifdef _UNICODE
    typedef wstring tstring;
  #else
    typedef string tstring;
  #endif
#endif
};

#define STRCPY_AtoA(target,src,len) strncpy(target,src,len)
#define STRDUP_AtoA(x) strdup(x)

#if defined(_UCS2)
  #define stringDuplicate(x) _tcsdup(x)

  #if defined(_CL_HAVE_FUNCTION_WCSDUP)
  	#define STRDUP_WtoW	wcsdup
  #else
  	#define STRDUP_WtoW	lucene_wcsdup
  #endif
  #define STRDUP_TtoT STRDUP_WtoW
  #define STRDUP_WtoT STRDUP_WtoW
  #define STRDUP_TtoW STRDUP_WtoW

  #define STRDUP_AtoW(x) CL_NS(util)::Misc::_charToWide(x)
  #define STRDUP_AtoT STRDUP_AtoW

  #define STRDUP_WtoA(x) CL_NS(util)::Misc::_wideToChar(x)
  #define STRDUP_TtoA STRDUP_WtoA

  #define STRCPY_WtoW(target,src,len) _tcsncpy(target,src,len)
  #define STRCPY_TtoW STRCPY_WtoW
  #define STRCPY_WtoT STRCPY_WtoW
  //#define _tcscpy STRCPY_WtoW

  #define STRCPY_AtoW(target,src,len) CL_NS(util)::Misc::_cpycharToWide(src,target,len)
  #define STRCPY_AtoT STRCPY_AtoW

  #define STRCPY_WtoA(target,src,len) CL_NS(util)::Misc::_cpywideToChar(src,target,len)
  #define STRCPY_TtoA STRCPY_WtoA
#else
  #define stringDuplicate(x) strdup(x)
  #define STRDUP_AtoT STRDUP_AtoA
  #define STRDUP_TtoA STRDUP_AtoA
  #define STRDUP_TtoT STRDUP_AtoA

  #define STRDUP_WtoT(x) xxxxxxxxxxxxxxx //not possible
  #define STRCPY_WtoT(target,src,len) xxxxxxxxxxxxxxx //not possible

  #define STRCPY_AtoT STRCPY_AtoA
  #define STRCPY_TtoA STRCPY_AtoA
  //#define _tcscpy STRCPY_AtoA
#endif


#endif //_REPL_TCHAR_H
