/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include <algorithm>

#include "CLucene/_ApiHeader.h"
#include "CLucene/LuceneThreads.h"
#include "_ThreadLocal.h"
#include "CLucene/config/_threads.h"
#include <assert.h>

CL_NS_DEF ( util )

/*
* The concept of ThreadLocal is that a ThreadLocal class stores specific values for each unique thread.
* Several thread-end detection techniques are used to delete the thread data if the thread dies before the ThreadLocal class is shut.
*
* The class->thread data mapping is stored in the _ThreadLocal class.
* The thread->datas mapping is in ThreadData.
*/


//predefine for the shared code...
#if defined(_CL_HAVE_WIN32_THREADS)
	#define INIT_THREAD(ret) ret=true
    extern "C"{

        //todo: move this to StdHeader and make it usable by other functions...
        bool __stdcall DllMain( unsigned short hinstDLL,     // DLL module handle
                                _cl_dword_t fdwReason,  // reason called
                                void*)                  // reserved
        { 
			if ( fdwReason == 3 )
            			_ThreadLocal::UnregisterCurrentThread();

			return true;
        }
    }
#elif defined(_CL_HAVE_PTHREAD)
    pthread_key_t pthread_threadlocal_key;
    pthread_once_t pthread_threadlocal_key_once = PTHREAD_ONCE_INIT;
    #define INIT_THREAD(ret) \
	pthread_once(&pthread_threadlocal_key_once, pthread_threadlocal_make_key); \
	if (pthread_getspecific(pthread_threadlocal_key) == NULL) { pthread_setspecific(pthread_threadlocal_key, (void*)1); } \
	ret = true;

    //the function that is called when the thread shutsdown
    void pthread_threadlocal_destructor(void* /*_holder*/){
        _ThreadLocal::UnregisterCurrentThread();
    }
    //the key initialiser function
    void pthread_threadlocal_make_key()
    {
		(void) pthread_key_create(&pthread_threadlocal_key, &pthread_threadlocal_destructor);
    }
#endif

class _ThreadLocal;

/**
* List that holds the list of ThreadLocals that this thread has data in.
*/
class ThreadLocals : private std::set<_ThreadLocal*>
{
public:
	void UnregisterThread();
	void add(_ThreadLocal* thread);
    void remove(_ThreadLocal* thread);
};

//map of thread<>ThreadLocals
typedef CL_NS ( util ) ::CLMultiMap<_LUCENE_THREADID_TYPE, ThreadLocals*,
	CL_NS ( util ) ::CLuceneThreadIdCompare,
	CL_NS ( util ) ::Deletor::ConstNullVal<_LUCENE_THREADID_TYPE>,
	CL_NS ( util ) ::Deletor::Object<ThreadLocals> > ThreadDataType;
static ThreadDataType*  threadData = NULL; 

#ifndef _CL_DISABLE_MULTITHREADING
	//the lock for locking ThreadData
	//we don't use STATIC_DEFINE_MUTEX, because then the initialization order will be undefined.
	static _LUCENE_THREADMUTEX *threadData_LOCK = NULL;
#endif


class _ThreadLocal::Internal
{
	public:
		typedef CL_NS ( util ) ::CLSet<_LUCENE_THREADID_TYPE, void*,
			CL_NS ( util ) ::CLuceneThreadIdCompare,
			CL_NS ( util ) ::Deletor::ConstNullVal<_LUCENE_THREADID_TYPE>,
			CL_NS ( util ) ::Deletor::ConstNullVal<void*> > LocalsType;
		LocalsType locals;
		DEFINE_MUTEX ( locals_LOCK )
		AbstractDeletor* _deletor;

		Internal ( AbstractDeletor* _deletor ) :
			locals ( false,false )
		{
			this->_deletor = _deletor;
		}
		~Internal()
		{
			//remove all the thread local data for this object
			LocalsType::iterator itr = locals.begin();
			while ( itr != locals.end() )
			{
				void* val = itr->second;
				locals.removeitr ( itr );
				_deletor->Delete ( val );
				itr = locals.begin();
			}

			delete _deletor;
		}
};

_ThreadLocal::_ThreadLocal ( CL_NS ( util ) ::AbstractDeletor* _deletor ) :
		_internal ( _CLNEW Internal ( _deletor ) )
{

}

_ThreadLocal::~_ThreadLocal()
{
	setNull();
	UnregisterCurrentThread();
    RemoveThreadLocal( this );
	delete _internal;
}


void* _ThreadLocal::get()
{
	SCOPED_LOCK_MUTEX(_internal->locals_LOCK)
	return _internal->locals.get ( _LUCENE_CURRTHREADID );
}

void _ThreadLocal::setNull()
{
	//just delete this thread from the locals list
	_LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;
	SCOPED_LOCK_MUTEX(_internal->locals_LOCK)
	Internal::LocalsType::iterator itr = _internal->locals.find ( id );
	if ( itr != _internal->locals.end() )
	{
		void* val = itr->second;
		_internal->locals.removeitr ( itr );
		_internal->_deletor->Delete ( val );
	}
}

void _ThreadLocal::set ( void* t )
{
	if ( t == NULL ){
		setNull();
		return;
	}
	//make sure we have a threadlocal context (for cleanup)
	bool ret;
	INIT_THREAD(ret);
	assert(ret);

	_LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;

	//drop a reference to this ThreadLocal in ThreadData
	{
#ifndef _CL_DISABLE_MULTITHREADING
		//slightly un-usual way of initialising mutex, 
		//because otherwise our initialisation order would be undefined
		if ( threadData_LOCK == NULL )
			threadData_LOCK = _CLNEW _LUCENE_THREADMUTEX;
		SCOPED_LOCK_MUTEX ( *threadData_LOCK );
#endif

		if ( threadData == NULL )
			threadData = _CLNEW ThreadDataType ( false, true );

		ThreadLocals* threadLocals = threadData->get(id);
		if ( threadLocals == NULL ){
			threadLocals = _CLNEW ThreadLocals;
      threadData->insert( std::pair<const _LUCENE_THREADID_TYPE, ThreadLocals*>(id,threadLocals));
		}
		threadLocals->add(this);
	}

	{
		SCOPED_LOCK_MUTEX(_internal->locals_LOCK)
		Internal::LocalsType::iterator itr = _internal->locals.find ( id );
		if ( itr != _internal->locals.end() )
		{
			void* val = itr->second;
			_internal->locals.removeitr ( itr );
			_internal->_deletor->Delete ( val );
		}
	
		if ( t != NULL )
			_internal->locals.put ( id, t );
	}

}

void _ThreadLocal::UnregisterCurrentThread()
{
	if ( threadData == NULL )
		return;
	_LUCENE_THREADID_TYPE id = _LUCENE_CURRTHREADID;
	SCOPED_LOCK_MUTEX ( *threadData_LOCK );

	ThreadDataType::iterator itr = threadData->find(id);
	if ( itr != threadData->end() ){
		ThreadLocals* threadLocals = itr->second;
		threadLocals->UnregisterThread();
		threadData->removeitr(itr);
	}
}

void _ThreadLocal::RemoveThreadLocal( _ThreadLocal * tl )
{
	if ( threadData == NULL )
		return;

    SCOPED_LOCK_MUTEX ( *threadData_LOCK );

	ThreadDataType::iterator itr = threadData->begin();
    for( ThreadDataType::iterator itr = threadData->begin(); itr != threadData->end(); itr++ )
    {
		ThreadLocals* threadLocals = itr->second;
        threadLocals->remove( tl );
        // Remove empty threadLocals
	}
}

void _ThreadLocal::_shutdown()
{
#ifndef _CL_DISABLE_MULTITHREADING
	_CLDELETE(threadData_LOCK);
#endif
	_CLDELETE(threadData);
}



void ThreadLocals::UnregisterThread()
{
	//this should only be accessed from its own thread... if this changes, then this access has to be locked.
    for( ThreadLocals::iterator iTLocal = begin(); iTLocal != end(); iTLocal++ )
        (*iTLocal)->setNull();
    clear();
}
void ThreadLocals::add(_ThreadLocal* thread)
{
	//this should only be accessed from its own thread... if this changes, then this access has to be locked.
    if( end() == find( thread ) )
        insert( thread );
}
void ThreadLocals::remove(_ThreadLocal* thread)
{    
    ThreadLocals::iterator iTLocal = find( thread );
    if( iTLocal != end() )
        erase( iTLocal );
}

CL_NS_END
