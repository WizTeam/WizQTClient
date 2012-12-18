/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_StringIntern_H
#define _lucene_util_StringIntern_H

//#include "Equators.h"
//#include "_VoidMap.h"

CL_NS_DEF(util)
 /** Functions for intern'ing strings. This
  * is a process of pooling strings thus using less memory,
  * and furthermore allows intern'd strings to be directly
  * compared:
  * string1==string2, rather than _tcscmp(string1,string2)
  */
  class CLStringIntern{
  public:
    
	/** 
	* Internalise the specified string.
	* \return Returns a pointer to the internalised string
	*/
	static const char* internA(const char* str, const int8_t count=1, const bool use_provided=false);

	/** 
	* Uninternalise the specified string. Decreases
	* the reference count and frees the string if 
	* reference count is zero
	* \returns true if string was destroyed, otherwise false
	*/
	static bool uninternA(const char* str, const int8_t count=1);

	/** 
	* Internalise the specified string.
	* \return Returns a pointer to the internalised string
	*/
	static const TCHAR* intern(const TCHAR* str);
	
	/** 
	* Uninternalise the specified string. Decreases
	* the reference count and frees the string if 
	* reference count is zero
	* \returns true if string was destroyed, otherwise false
	*/
	static bool unintern(const TCHAR* str);

	/** Cleanup static data */
	static CLUCENE_LOCAL void _shutdown();
  };

CL_NS_END
#endif
