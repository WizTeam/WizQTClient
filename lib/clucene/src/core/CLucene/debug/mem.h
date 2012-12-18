/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_debug_mem_h
#define _lucene_debug_mem_h

//todo: this is a hack...
#ifndef CND_PRECONDITION
  #include <assert.h>
	#define CND_PRECONDITION(x,y) assert(x)
#endif

//Macro for creating new objects
#if defined(LUCENE_ENABLE_REFCOUNT)
   #define _CLNEW new
#else
   #define _CLNEW new
#endif
#define _CL_POINTER(x) (x==NULL?NULL:(x->__cl_addref()>=0?x:x)) //return a add-ref'd object
#define _CL_DECREF(x) ((x)==NULL?NULL:((x)->__cl_decref()>=0?(x):(x))) //return a add-ref'd object
#define _CL_LDECREF(x) if ((x)!=NULL) (x)->__cl_decref();

//Macro for creating new arrays
#define _CL_NEWARRAY(type,size) (type*)calloc(size, sizeof(type))
#define _CLDELETE_ARRAY(x) {free(x); x=NULL;}
#define _CLDELETE_LARRAY(x) {free(x);}
#ifndef _CLDELETE_CARRAY
	#define _CLDELETE_CARRAY(x) {free(x); x=NULL;}
	#define _CLDELETE_LCARRAY(x) {free(x);}
#endif

//a shortcut for deleting a carray and all its contents
#define _CLDELETE_CARRAY_ALL(x) {if ( x!=NULL ){ for(int xcda=0;x[xcda]!=NULL;xcda++)_CLDELETE_CARRAY(x[xcda]);}_CLDELETE_ARRAY(x)};
#define _CLDELETE_LCARRAY_ALL(x) {if ( x!=NULL ){ for(int xcda=0;x[xcda]!=NULL;xcda++)_CLDELETE_LCARRAY(x[xcda]);}_CLDELETE_LARRAY(x)};
#define _CLDELETE_CaARRAY_ALL(x) {if ( x!=NULL ){ for(int xcda=0;x[xcda]!=NULL;xcda++)_CLDELETE_CaARRAY(x[xcda]);}_CLDELETE_ARRAY(x)};
#define _CLDELETE_ARRAY_ALL(x) {if ( x!=NULL ){ for(int xcda=0;x[xcda]!=NULL;xcda++)_CLDELETE(x[xcda]);}_CLDELETE_ARRAY(x)};
#ifndef _CLDELETE_CaARRAY
		#define _CLDELETE_CaARRAY _CLDELETE_CARRAY
		#define _CLDELETE_LCaARRAY _CLDELETE_LCARRAY
#endif

//Macro for deleting
#ifdef LUCENE_ENABLE_REFCOUNT
	#define _CLDELETE(x) if (x!=NULL){ CND_PRECONDITION(_LUCENE_ATOMIC_INT_GET((x)->__cl_refcount)>=0,"__cl_refcount was < 0"); if (_LUCENE_ATOMIC_INT_GET((x)->__cl_decref()) <= 0)delete x; x=NULL; }
	#define _CLLDELETE(x) if (x!=NULL){ CND_PRECONDITION(_LUCENE_ATOMIC_INT_GET((x)->__cl_refcount)>=0,"__cl_refcount was < 0"); if ((x)->__cl_decref() <= 0)delete x; }
#else
	#define _CLDELETE(x) {delete x;x=NULL;}
	#define _CLLDELETE(x) {delete x;}
#endif

//_CLDECDELETE deletes objects which are *always* refcounted
#define _CLDECDELETE(x) if (x!=NULL){ CND_PRECONDITION(_LUCENE_ATOMIC_INT_GET((x)->__cl_refcount)>=0,"__cl_refcount was < 0"); _LUCENE_ATOMIC_DECDELETE(&(x)->__cl_refcount, x); x=NULL; }
#define _CLLDECDELETE(x) if (x!=NULL){ CND_PRECONDITION(_LUCENE_ATOMIC_INT_GET((x)->__cl_refcount)>=0,"__cl_refcount was < 0"); _LUCENE_ATOMIC_DECDELETE(&(x)->__cl_refcount, x); }
#define _LUCENE_ATOMIC_DECDELETE(theInteger, theObject) { if ( _LUCENE_ATOMIC_DEC(theInteger) == 0) delete theObject;} 

//_VDelete should be used for deleting non-clucene objects.
//when using reference counting, _CLDELETE casts the object
//into a LuceneBase*.
#define _CLVDELETE(x) {delete x;x=NULL;}

#endif //_lucene_debug_mem_h
