/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef lucene_sharedheader_h
#define lucene_sharedheader_h

/**
* This header contains definitions and macros for helping create cross-platform code.
* It is primarily for use by the clucene-core library, but is split off so that
* other applications such as the demo, test, benchmarks, etc can benefit from the
* cross platform code. Cross platform code is not distributed with the clucene-core
* and is not available through the shared library.
*/

#include "CLucene/clucene-config.h"

//some early definitions
#if defined(_MSC_VER) || defined(__BORLANDC__)
    #define _LUCENE_PRAGMA_WARNINGS //tell lucene to display warnings using pragmas instead of #warning
#endif


////////////////////////////////////////////////////////
//Are we in unicode mode?
////////////////////////////////////////////////////////
#if defined(_MBCS) || defined(_ASCII)
 #undef _ASCII
 #undef _UCS2
 #define _ASCII
#elif defined(_UNICODE)
 #ifndef _UCS2
  #define _UCS2
 #endif
#elif !defined(_UCS2)
 #define _UCS2
#endif

//msvc needs unicode define so that it uses unicode library
#ifdef _UCS2
	#undef _UNICODE
	#define _UNICODE
	#undef _ASCII
#else
	#undef _UNICODE
	#undef _UCS2
#endif
////////////////////////////////////////////////////////



////////////////////////////////////////////////////////
//platform includes that MUST be included for the public headers to work...
////////////////////////////////////////////////////////
#include <cstddef> //need this for wchar_t, size_t, NULL
#ifdef _CL_HAVE_STDINT_H
    #include <stdint.h> //need this for int32_t, etc
#endif
#include <math.h> //required for float_t
#include <string> //need to include this really early...

#ifdef _CL_HAVE_TCHAR_H
    #include <tchar.h> //required for _T and TCHAR
#endif

////////////////////////////////////////////////////////
//namespace helper
////////////////////////////////////////////////////////
#if defined(_LUCENE_DONTIMPLEMENT_NS_MACROS)
    //do nothing
#elif !defined(DISABLE_NAMESPACE) && defined(_CL_HAVE_NAMESPACES)
	#define CL_NS_DEF(sub) namespace lucene{ namespace sub{
	#define CL_NS_DEF2(sub,sub2) namespace lucene{ namespace sub{ namespace sub2 {

	#define CL_NS_END }}
	#define CL_NS_END2 }}}

	#define CL_NS_USE(sub) using namespace lucene::sub;
	#define CL_NS_USE2(sub,sub2) using namespace lucene::sub::sub2;

	#define CL_NS(sub) lucene::sub
	#define CL_NS2(sub,sub2) lucene::sub::sub2
	    
	#define CL_STRUCT_DEF(sub,clazz) namespace lucene { namespace sub{ struct clazz; } }
	#define CL_CLASS_DEF(sub,clazz) namespace lucene { namespace sub{ class clazz; } }
	#define CL_CLASS_DEF2(sub,sub2, clazz) namespace lucene { namespace sub{ namespace sub2{ class clazz; } } }

	#define CL_TEMPATE_DEF(sub, clazz, typedefs) namespace lucene { namespace sub{ template<typedefs> class clazz; }}
	#define CL_TYPE_DEF(sub, clazz, def) namespace lucene { namespace sub{ typedef def clazz; }}
#else
	#define CL_NS_DEF(sub)
	#define CL_NS_DEF2(sub, sub2)
	#define CL_NS_END
	#define CL_NS_END2
	#define CL_NS_USE(sub)
	#define CL_NS_USE2(sub,sub2)
	#define CL_NS(sub)
	#define CL_NS2(sub,sub2)
	#define CL_CLASS_DEF(sub,clazz) class clazz;
	#define CL_CLASS_DEF2(sub,sub2, clazz) class clazz;
#endif

#if defined(LUCENE_NO_STDC_NAMESPACE)
   //todo: haven't actually tested this on a non-stdc compliant compiler
   #define CL_NS_STD(func) ::func
#else
   #define CL_NS_STD(func) std::func
#endif
//
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// EXPORTS definition
////////////////////////////////////////////////////////
#if defined(_WIN32) || defined(_WIN64)
	#define CLUCENE_EXPORT_DECL __declspec(dllexport)
  #define CLUCENE_IMPORT_DECL __declspec(dllimport)
  #define CLUCENE_LOCAL_DECL
#elif defined(_CL_HAVE_GCCVISIBILITYPATCH)
	#define CLUCENE_EXPORT_DECL __attribute__ ((visibility("default")))
	#define CLUCENE_LOCAL_DECL __attribute__ ((visibility("hidden")))
	#define CLUCENE_IMPORT_DECL
#else
	#define CLUCENE_EXPORT_DECL
	#define CLUCENE_IMPORT_DECL
	#define CLUCENE_LOCAL_DECL
#endif

//define for the libraries
#if defined(clucene_shared_EXPORTS)
	#define CLUCENE_SHARED_EXPORT CLUCENE_EXPORT_DECL
	#define CLUCENE_LOCAL CLUCENE_LOCAL_DECL
#elif defined(MAKE_CLUCENE_SHARED_LIB)
	#define CLUCENE_SHARED_EXPORT //don't export if we are building a static library
#else
    #define CLUCENE_SHARED_EXPORT CLUCENE_IMPORT_DECL
#endif
#if defined(clucene_core_EXPORTS)
	#define CLUCENE_EXPORT CLUCENE_EXPORT_DECL
	#define CLUCENE_LOCAL CLUCENE_LOCAL_DECL
#elif defined(MAKE_CLUCENE_CORE_LIB)
	#define CLUCENE_EXPORT
#else
    #define CLUCENE_EXPORT CLUCENE_IMPORT_DECL
#endif
#if defined(clucene_contribs_lib_EXPORTS)
	#define CLUCENE_CONTRIBS_EXPORT CLUCENE_EXPORT_DECL
	#define CLUCENE_LOCAL CLUCENE_LOCAL_DECL
#elif defined(MAKE_CLUCENE_CONTRIBS_LIB)
	#define CLUCENE_CONTRIBS_EXPORT
#else
    #define CLUCENE_CONTRIBS_EXPORT CLUCENE_IMPORT_DECL
#endif
#ifndef CLUCENE_LOCAL
	#define CLUCENE_LOCAL
#endif

//inline definitions
#if defined(__MINGW32__) || defined(_MSC_VER)
 #define CLUCENE_SHARED_INLINE_EXPORT
 #define CLUCENE_INLINE_EXPORT
 #define CLUCENE_CONTRIBS_INLINE_EXPORT
#else
 #define CLUCENE_SHARED_INLINE_EXPORT CLUCENE_SHARED_EXPORT
 #define CLUCENE_INLINE_EXPORT CLUCENE_EXPORT
 #define CLUCENE_CONTRIBS_INLINE_EXPORT CLUCENE_CONTRIBS_EXPORT
#endif
////////////////////////////////////////////////////////


//todo: put this logic in cmake
#if defined(_MSC_VER)
	#if _MSC_FULL_VER >= 140050320
	    #define _CL_DEPRECATE_TEXT(_Text) __declspec(deprecated(_Text))
	#elif _MSC_VER >= 1300
	    #define _CL_DEPRECATE_TEXT(_Text) __declspec(deprecated)
	#else
	    #define _CL_DEPRECATE_TEXT(_Text)
	#endif
#elif (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
	#define _CL_DEPRECATE_TEXT(_Text)  __attribute__((__deprecated__))
#else
    #define _CL_DEPRECATE_TEXT(_Text)
#endif
#define _CL_DEPRECATED(_NewItem) _CL_DEPRECATE_TEXT("This function or variable has been superceded by newer library or operating system functionality. Consider using " #_NewItem " instead. See online help for details.")
//
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
// boost stuff
////////////////////////////////////////////////////////
#if defined(_MSC_VER)
#  pragma warning (disable : 4251) // disable exported dll function
# endif

////////////////////////////////////////////////////////
//Class interfaces
////////////////////////////////////////////////////////
#include "CLucene/debug/lucenebase.h"
////////////////////////////////////////////////////////

//memory handling macros/functions
#include "CLucene/debug/mem.h"

#ifdef DMALLOC
  #include <stdlib.h>
  #include <string.h>
	#include <dmalloc.h>
#endif

#endif //lucene_sharedheader_h
