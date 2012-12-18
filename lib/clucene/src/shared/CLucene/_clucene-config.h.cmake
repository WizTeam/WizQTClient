#ifndef _SRC_CLUCENE_INTERNAL_CLUCENE_CONFIG_H
#define _SRC_CLUCENE_INTERNAL_CLUCENE_CONFIG_H 1
 
/* src/shared/CLucene/_clucene-config.h. 
*  Generated automatically at end of cmake.
*  These are internal definitions, and this file does not need to be distributed
*/

/* CMake will look for these functions: */
#cmakedefine _CL_HAVE_FUNCTION__VSNWPRINTF
#cmakedefine _CL_HAVE_FUNCTION__SNWPRINTF
#cmakedefine _CL_HAVE_FUNCTION_WCSCASECMP
#cmakedefine _CL_HAVE_FUNCTION_WCSCAT  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSCHR  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSCMP  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSCPY  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSCSPN  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSICMP
#cmakedefine _CL_HAVE_FUNCTION_WCSLEN  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSNCMP  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSNCPY  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSSTR  1 
#cmakedefine _CL_HAVE_FUNCTION_WCSTOD 1
#cmakedefine _CL_HAVE_FUNCTION_WCSDUP 1
#cmakedefine _CL_HAVE_FUNCTION_WCSTOLL 1
#cmakedefine _CL_HAVE_FUNCTION_WCSUPR 1
#cmakedefine _CL_HAVE_FUNCTION_GETTIMEOFDAY 1
#cmakedefine _CL_HAVE_FUNCTION_MAPVIEWOFFILE 1

#cmakedefine _CL_HAVE_FUNCTION_LLTOA 1
#cmakedefine _CL_HAVE_FUNCTION_LLTOW 1
#cmakedefine _CL_HAVE_FUNCTION_PRINTF  1 
#cmakedefine _CL_HAVE_FUNCTION_SNPRINTF  1 
#cmakedefine _CL_HAVE_FUNCTION_MMAP  1 
#cmakedefine _CL_HAVE_FUNCTION_STRLWR 1
#cmakedefine _CL_HAVE_FUNCTION_STRTOLL 1
#cmakedefine _CL_HAVE_FUNCTION_STRUPR 1
#cmakedefine _CL_HAVE_FUNCTION_GETPAGESIZE 1
#cmakedefine _CL_HAVE_FUNCTION_USLEEP 1
#cmakedefine _CL_HAVE_FUNCTION_SLEEP 1

${SYMBOL_CL_MAX_PATH}
//this is the max filename... for now its just the same,
//but this could change, so we use a different name
#define CL_MAX_NAME CL_MAX_PATH
//this used to be CL_MAX_NAME * 32, but as Alex Hudson points out, this could come to be 128kb.
//the above logic for CL_MAX_NAME should be correct enough to handle all file names
#define CL_MAX_DIR CL_MAX_PATH

${SYMBOL__O_RANDOM}
${SYMBOL__O_BINARY}
${SYMBOL__S_IREAD}
${SYMBOL__S_IWRITE}
${TYPE__TIMEB}

#define _ILONG(x) x ## L
#define _ILONGLONG(x) ${_CL_ILONGLONG_VALUE}

${FUNCTION_FILESTAT}
${TYPE_CL_STAT_T}
${FUNCTION_FILESIZE}
${FUNCTION_FILESEEK}
${FUNCTION_FILETELL}
${FUNCTION_FILEHANDLESTAT}
${FUNCTION__REALPATH}
${FUNCTION__RENAME}
${FUNCTION__CLOSE}
${FUNCTION__READ}
${FUNCTION__CL_OPEN}
${FUNCTION__WRITE}
${FUNCTION__SNPRINTF}
${FUNCTION__MKDIR}
${FUNCTION__UNLINK}
${FUNCTION__FTIME}
${FUNCTION_SLEEPFUNCTION}

/* CMake will determine these specifics. Things like bugs, etc */

/* Does not support new float byte<->float conversions */
#cmakedefine _CL_HAVE_NO_FLOAT_BYTE 1

/* Define if recursive pthread mutexes are available */
#cmakedefine _CL_HAVE_PTHREAD_MUTEX_RECURSIVE  1 

/** define if you would like to force clucene to use the internal
* character functions.
* Tests may display unpredictable behaviour if this is not defined.
*/
#ifndef LUCENE_USE_INTERNAL_CHAR_FUNCTIONS
	#cmakedefine LUCENE_USE_INTERNAL_CHAR_FUNCTIONS 1
#endif

/** fix ansi for loop scope */
#if @CMAKE_ANSI_FOR_SCOPE@==0
 #define for if (0); else for
#endif


/* Compiler oddities */

//not sure why, but cygwin reports _S_IREAD, but doesn't actually work...
//TODO: make this work properly (this bit shouldn't be necessary)
#ifdef __CYGWIN__
    #define _S_IREAD 0333
    #define _S_IWRITE 0333
#endif

#ifdef __BORLANDC__ //borland compiler
    #define O_RANDOM 0
#endif

#endif
