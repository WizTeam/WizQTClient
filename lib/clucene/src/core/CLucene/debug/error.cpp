/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_SharedHeader.h"
#include "error.h"
#include "CLucene/util/Misc.h"
CL_NS_USE(util)


#ifdef _LUCENE_DISABLE_EXCEPTIONS
	#ifdef _LUCENE_PRAGMA_WARNINGS
	 #pragma message ("==================Lucene exceptions are disabled==================")
	#else
	 #warning "==================Lucene exceptions are disabled=================="
	#endif
#else
    CLuceneError::CLuceneError():_twhat(NULL),error_number(0){
#ifndef _ASCII
		_awhat = NULL;
#endif
	}
    CLuceneError::CLuceneError(int num, const char* str, bool ownstr):error_number(num)
	{
#ifdef _ASCII
        _twhat=STRDUP_AtoA(str);

#else
        _awhat=STRDUP_AtoA(str);
		_twhat=NULL;
#endif
		if ( ownstr )
			_CLDELETE_LCaARRAY( const_cast<char*>(str));
    }

#ifndef _ASCII
	CLuceneError::CLuceneError(int num, const TCHAR* str, bool ownstr)
	{
		error_number = num;
		_awhat=NULL;
		_twhat=STRDUP_TtoT(str);
		if ( ownstr )
			_CLDELETE_LCARRAY((TCHAR*)str);
    }
#endif
	
	CLuceneError::CLuceneError(const CLuceneError& clone)
	{
		this->error_number = clone.error_number;
#ifndef _ASCII
		this->_awhat = NULL;
		if ( clone._awhat != NULL )
			this->_awhat = STRDUP_AtoA(clone._awhat);
#endif
		this->_twhat = NULL;
		if ( clone._twhat != NULL )
			this->_twhat = STRDUP_TtoT(clone._twhat);
	}
	CLuceneError::~CLuceneError() throw(){
		_CLDELETE_LCARRAY(_twhat);
#ifndef _ASCII
		_CLDELETE_LCaARRAY(_awhat);
#endif
	}
	char* CLuceneError::what(){
#ifdef _ASCII
        return _twhat;
#else
		if ( _awhat == NULL )
			_awhat = STRDUP_TtoA(_twhat);
		return _awhat;
#endif
	}
	TCHAR* CLuceneError::twhat(){
#ifdef _ASCII
        return _twhat;
#else
		if ( _twhat == NULL )
			_twhat = STRDUP_AtoT(_awhat);
		return _twhat;
#endif
	}

#ifndef _ASCII
	void CLuceneError::set(int num, const char* str, bool ownstr){
		_CLDELETE_CARRAY(_twhat);
		_CLDELETE_CaARRAY(_awhat);
		_awhat=STRDUP_AtoA(str);
		error_number = num;
		if ( ownstr )
			_CLDELETE_LCaARRAY( const_cast<char*>(str));
	}
#endif

	void CLuceneError::set(int num, const TCHAR* str, bool ownstr){
#ifndef _ASCII
        _CLDELETE_CaARRAY(_awhat);
#endif
		_CLDELETE_CARRAY(_twhat);
		_twhat=STRDUP_TtoT(str);
		error_number = num;
		if ( ownstr )
			_CLDELETE_LCARRAY(const_cast<TCHAR*>(str));
	}

#endif //_LUCENE_DISABLE_EXCEPTIONS
