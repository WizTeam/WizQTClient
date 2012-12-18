  /*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_StringIntern.h"
CL_NS_DEF(util)

typedef CL_NS(util)::CLHashMap<TCHAR*,int,CL_NS(util)::Compare::TChar,CL_NS(util)::Equals::TChar,CL_NS(util)::Deletor::tcArray, CL_NS(util)::Deletor::DummyInt32 > __wcsintrntype;
typedef CL_NS(util)::CLHashMap<char*,int,CL_NS(util)::Compare::Char,CL_NS(util)::Equals::Char,CL_NS(util)::Deletor::acArray, CL_NS(util)::Deletor::DummyInt32 > __strintrntype;
__wcsintrntype StringIntern_stringPool(true);
__strintrntype StringIntern_stringaPool(true);

bool StringIntern_blanksinitd=false;
__wcsintrntype::iterator StringIntern_wblank;

//STATIC_DEFINE_MUTEX(StringIntern_THIS_LOCK);
DEFINE_MUTEX(StringIntern_THIS_LOCK)
	  

    void CLStringIntern::_shutdown(){
    #ifdef _DEBUG
		SCOPED_LOCK_MUTEX(StringIntern_THIS_LOCK)
        if ( StringIntern_stringaPool.size() > 0 ){
            printf("WARNING: stringaPool still contains intern'd strings (refcounts):\n");
            __strintrntype::iterator itr = StringIntern_stringaPool.begin();
            while ( itr != StringIntern_stringaPool.end() ){
                printf(" %s (%d)\n",(itr->first), (itr->second));
                ++itr;
            }
        }
        
        if ( StringIntern_stringPool.size() > 0 ){
            printf("WARNING: stringPool still contains intern'd strings (refcounts):\n");
            __wcsintrntype::iterator itr = StringIntern_stringPool.begin();
            while ( itr != StringIntern_stringPool.end() ){
                _tprintf(_T(" %s (%d)\n"),(itr->first), (itr->second));
                ++itr;
            }
        }
    #endif
    }

	const TCHAR* CLStringIntern::intern(const TCHAR* str){
		if ( str == NULL )
			return NULL;
		if ( str[0] == 0 )
			return LUCENE_BLANK_STRING;

		SCOPED_LOCK_MUTEX(StringIntern_THIS_LOCK)

		__wcsintrntype::iterator itr = StringIntern_stringPool.find((TCHAR*)str);
		if ( itr==StringIntern_stringPool.end() ){
			TCHAR* ret = STRDUP_TtoT(str);
			StringIntern_stringPool[ret]= 1;
			return ret;
		}else{
			(itr->second)++;
			return itr->first;
		}
	}

	bool CLStringIntern::unintern(const TCHAR* str){
		if ( str == NULL )
			return false;
		if ( str[0] == 0 )
			return false; // warning: a possible memory leak, since str may be never freed!

		SCOPED_LOCK_MUTEX(StringIntern_THIS_LOCK)

		__wcsintrntype::iterator itr = StringIntern_stringPool.find((TCHAR*)str);
		if ( itr != StringIntern_stringPool.end() ){
			if ( (itr->second) == 1 ){
				StringIntern_stringPool.removeitr(itr);
				return true;
			}else
				(itr->second)--;
		}
		return false;
	}
	
	const char* CLStringIntern::internA(const char* str, const int8_t count, const bool use_provided){
		if ( str == NULL )
			return NULL;
		if ( str[0] == 0 )
			return _LUCENE_BLANK_ASTRING;

		SCOPED_LOCK_MUTEX(StringIntern_THIS_LOCK)

		__strintrntype::iterator itr = StringIntern_stringaPool.find((char*)str);
		if ( itr==StringIntern_stringaPool.end() ){
			char* ret = (use_provided) ? const_cast<char*>(str) : STRDUP_AtoA(str);
			StringIntern_stringaPool[ret] = count;
			return ret;
		}else{
			if (use_provided) _CLDELETE_LCaARRAY((char*)str); // delete the provided string if already exists
			(itr->second) = (itr->second) + count;
			return itr->first;
		}
	}
	
	bool CLStringIntern::uninternA(const char* str, const int8_t count){
		if ( str == NULL )
			return false;
		if ( str[0] == 0 )
			return false; // warning: a possible memory leak, since str may be never freed!

		SCOPED_LOCK_MUTEX(StringIntern_THIS_LOCK)

		__strintrntype::iterator itr = StringIntern_stringaPool.find((char*)str);
		if ( itr!=StringIntern_stringaPool.end() ){
			if ( (itr->second) == count ){
				StringIntern_stringaPool.removeitr(itr);
				return true;
			}else
				(itr->second) = (itr->second) - count;
		}
		return false;
	}
CL_NS_END
