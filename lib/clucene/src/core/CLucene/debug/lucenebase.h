/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_debug_lucenebase_
#define _lucene_debug_lucenebase_

#include "CLucene/LuceneThreads.h"

CL_NS_DEF(debug)

//Lucenebase is the superclass of all clucene objects. It provides
//memory debugging tracking and/or reference counting
class CLUCENE_EXPORT LuceneBase{
public:
	_LUCENE_ATOMIC_INT __cl_refcount;
	LuceneBase(){
		_LUCENE_ATOMIC_INT_SET(__cl_refcount,1);
	}
	inline int __cl_getref(){
		return _LUCENE_ATOMIC_INT_GET(__cl_refcount);
	}
  inline int __cl_addref(){ return _LUCENE_ATOMIC_INC(&__cl_refcount); }
  inline int __cl_decref(){ return _LUCENE_ATOMIC_DEC(&__cl_refcount); }
  virtual ~LuceneBase(){};
};

class CLUCENE_EXPORT LuceneVoidBase{
	public:
    virtual ~LuceneVoidBase(){};
};

#if defined(LUCENE_ENABLE_REFCOUNT)
   #define LUCENE_BASE public CL_NS(debug)::LuceneBase
#else
   #define LUCENE_BASE public CL_NS(debug)::LuceneVoidBase
#endif
#define LUCENE_REFBASE public CL_NS(debug)::LuceneBase //this is the base of classes who *always* need refcounting


CL_NS_END
#endif //_lucene_debug_lucenebase_
