/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_repl_wctype_h
#define _lucene_repl_wctype_h


////////////////////////////////////////////////////////
// Character functions.
// Here we decide whose character functions to use
////////////////////////////////////////////////////////
#if defined(LUCENE_USE_INTERNAL_CHAR_FUNCTIONS)
    #define stringCaseFold cl_tcscasefold
	#define stringCaseFoldCmp cl_tcscasefoldcmp
    
	#undef _istspace
	#undef _istdigit
	#undef _istalnum
	#undef _istalpha
	#undef _totlower
	#undef _totupper
    #define _istalnum cl_isalnum
    #define _istalpha cl_isletter
    #define _istspace cl_isspace
    #define _istdigit cl_isdigit
    #define _totlower cl_tolower
    #define _totupper cl_toupper

    //here are some functions to help deal with utf8/ucs2 conversions
    //lets let the user decide what mb functions to use... we provide pure utf8 ones no matter what.
    /*#undef _mbtowc
    #undef _mbstowcs
    #undef _wctomb
    #undef _wcstombs
    #define _mbtowc lucene_mbstowc
    #define _mbsstowcs lucene_mbstowcs
    #define _wctomb lucene_wcto_mb
    #define _wcstombs lucene_wcstombs*/
#else 
    //we are using native functions
	//here are some functions to help deal with utf8/ucs2 conversions
	/*#define _mbtowc mbtowc
	#define _wctomb wctomb
	#define _mbstowcs mbstowcs
	#define _wcstombs wcstombs*/

    //we are using native character functions
    #if defined(_ASCII)
        #undef _istspace
        #undef _istdigit
        #undef _istalnum
        #undef _istalpha
        #undef _totlower
        #undef _totupper
        #define _istspace(x) isspace((unsigned char)x)
        #define _istdigit(x) isdigit((unsigned char)x)
        #define _istalnum(x) isalnum((unsigned char)x)
        #define _istalpha(x) isalpha((unsigned char)x)
        #define _totlower(x) tolower((unsigned char)x)
        #define _totupper(x) toupper((unsigned char)x)
    #endif
#endif

//the methods contained in gunichartables.h
typedef unsigned long  clunichar;
CLUCENE_SHARED_EXPORT bool cl_isletter(clunichar c);
CLUCENE_SHARED_EXPORT bool cl_isalnum(clunichar c);
CLUCENE_SHARED_EXPORT bool cl_isdigit(clunichar c);
CLUCENE_SHARED_EXPORT bool cl_isspace (clunichar c);
CLUCENE_SHARED_EXPORT TCHAR cl_tolower (TCHAR c);
CLUCENE_SHARED_EXPORT TCHAR cl_toupper (TCHAR c);

#endif
