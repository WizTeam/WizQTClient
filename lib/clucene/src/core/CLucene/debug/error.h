/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_debug_error_
#define _lucene_debug_error_


#define CL_ERR_UNKNOWN -1
#define CL_ERR_IO 1
#define CL_ERR_NullPointer 2
#define CL_ERR_Runtime 3
#define CL_ERR_IllegalArgument 4
#define CL_ERR_Parse 5
#define CL_ERR_TokenMgr 6
#define CL_ERR_UnsupportedOperation 7
#define CL_ERR_InvalidState 8
#define CL_ERR_IndexOutOfBounds 9
#define CL_ERR_TooManyClauses 10
#define CL_ERR_RAMTransaction 11
#define CL_ERR_InvalidCast 12
#define CL_ERR_IllegalState 13 // Sub-error: AlreadyClosed
#define CL_ERR_UnknownOperator 14
#define CL_ERR_ConcurrentModification 15
#define CL_ERR_CorruptIndex 16
#define CL_ERR_NumberFormat 17
#define CL_ERR_AlreadyClosed 18
#define CL_ERR_StaleReader 19
#define CL_ERR_LockObtainFailed 20
#define CL_ERR_Merge 21 //< Exception thrown if there are any problems while executing a merge.
#define CL_ERR_MergeAborted 22
#define CL_ERR_OutOfMemory 23
#define CL_ERR_FieldReader 24

////////////////////////////////////////////////////////
//error try/throw/catch definitions
////////////////////////////////////////////////////////
#ifdef _CL_DISABLE_NATIVE_EXCEPTIONS
 #include <setjmp.h>
 
 /*
 #define try struct pj_exception_state_t pj_x_except__; int pj_x_code__; \
                if(1){ \
                pj_push_exception_handler_(&pj_x_except__); \
				pj_x_code__ = pj_setjmp(pj_x_except__.state); \
				if (pj_x_code__ == 0)
 #define _CLCATCHEND pj_pop_exception_handler_(); \
			    } else {}
 #define _CLCATCH else if (pj_x_code__ == (id)) _CLCATCHEND
 #define _CLCATCHANY else _CLCATCHEND
 #define _RETHROW pj_throw_exception_(pj_x_code__)
 
 #define _CLFINALLY(x) else{x _RETHROW}_CLCATCHEND x
 #define _CLTHROWA(number) pj_throw_exception_(number)
 #define _CLTHROWT(number) pj_throw_exception_(number)
 #define _THROWA_DEL(number) _CLDELETE_CaARRAY(str); pj_throw_exception_(number)
 #define _THROWT_DEL(number) _CLDELETE_CARRAY(str); pj_throw_exception_(number)
 
 */
#else
class CLUCENE_EXPORT CLuceneError
{
#ifndef _ASCII
	char* _awhat;
#endif
	TCHAR* _twhat;
	int error_number;
public:
	CLuceneError();
	CLuceneError(const CLuceneError& clone);
#ifndef _ASCII
	CLuceneError(int num, const char* str, bool ownstr);
#endif
	CLuceneError(int num, const TCHAR* str, bool ownstr);
  	int number() const{return error_number;}
		char* what();
		TCHAR* twhat();
		~CLuceneError() throw();

	void set(int num, const TCHAR*, bool ownstr=false);
#ifndef _ASCII
	void set(int num, const char*, bool ownstr=false);
#endif
};
	
 //#define _THROWS //does nothing
 #define _TRY try
 #define _CLCATCH_ERR(err_num, cleanup_code, else_code) catch(CLuceneError& err){if (err.number()!=err_num){cleanup_code;throw err;}else {else_code;}}
 #define _CLCATCH_ERR_ELSE(err_num, else_code) catch(CLuceneError& err){if (err.number()!=err_num){throw err;}else {else_code;}}
 #define _CLCATCH_ERR_CLEANUP(err_num, cleanup_code) catch(CLuceneError& err){if (err.number()!=err_num){cleanup_code;throw err;}}
 #define _CLFINALLY(x) catch(...){ x; throw; } x //note: code x is not run if return is called
 #define _CLTHROWA(number, str) throw CLuceneError(number, str,false)
 #define _CLTHROWT(number, str) throw CLuceneError(number, str,false)
 #define _CLTHROWA_DEL(number, str) throw CLuceneError(number, str,true) //throw a string ensures the value is deleted
 #define _CLTHROWT_DEL(number, str) throw CLuceneError(number, str,true) //throw a string ensures the value is deleted


#endif //_LUCENE_DISABLE_EXCEPTIONS
//
////////////////////////////////////////////////////////

#endif
