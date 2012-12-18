/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_SharedHeader.h"
#include "CLucene/LuceneThreads.h"
#include "_threads.h"
#include <assert.h>

CL_NS_DEF(util)

#ifndef _CL_DISABLE_MULTITHREADING

#if defined(_LUCENE_DONTIMPLEMENT_THREADMUTEX)
	//do nothing
	#if defined(_LUCENE_PRAGMA_WARNINGS)
	 #pragma message ("==================Not implementing any thread mutex==================")
	#else
	 #warning "==================Not implementing any thread mutex=================="
	#endif



#elif defined(_CL_HAVE_WIN32_THREADS)
	struct mutex_thread::Internal{
	    CRITICAL_SECTION mtx;
	};

	mutex_thread::mutex_thread(const mutex_thread& clone):
		_internal(new Internal)
	{
		InitializeCriticalSection(&_internal->mtx);
	}
	mutex_thread::mutex_thread():
		_internal(new Internal)
	{
		InitializeCriticalSection(&_internal->mtx);
	}

	mutex_thread::~mutex_thread()
	{
		DeleteCriticalSection(&_internal->mtx);
		delete _internal;
	}

	void mutex_thread::lock()
	{
		EnterCriticalSection(&_internal->mtx);
	}

	void mutex_thread::unlock()
	{
		LeaveCriticalSection(&_internal->mtx);
	}

  _LUCENE_THREADID_TYPE mutex_thread::_GetCurrentThreadId(){
      return GetCurrentThreadId();
  }
  void mutex_thread::_exitThread(int val){
  	ExitThread(val);
  }

  int32_t mutex_thread::atomic_increment(_LUCENE_ATOMIC_INT *theInteger){
#ifdef _M_X64
    return _InterlockedIncrement64(theInteger);
#else
    return InterlockedIncrement(theInteger);
#endif
  }
  int32_t mutex_thread::atomic_decrement(_LUCENE_ATOMIC_INT *theInteger){
#ifdef _M_X64
    return _InterlockedDecrement64(theInteger);
#else
    return InterlockedDecrement(theInteger);
#endif
  }



	class shared_condition::Internal{
	public:
	    void* _event;
	    Internal(){
	    	_event = CreateEventA( NULL, false, false, NULL );
	    }
	    ~Internal(){
	    	CloseHandle( _event );
	    }
	};
	shared_condition::shared_condition(){
		_internal = new Internal;
	}
	shared_condition::~shared_condition(){
		delete _internal;
	}
	void shared_condition::Wait(mutex_thread* shared_lock){
        shared_lock->unlock();
        _cl_dword_t dwRes = WaitForSingleObject( _internal->_event, 0xFFFFFFFF );
		assert ( 0x0 == dwRes );
        shared_lock->lock();
	}
	void shared_condition::NotifyAll(){
		bool bRes = SetEvent(_internal->_event);
        assert( bRes );
	}

	_LUCENE_THREADID_TYPE mutex_thread::CreateThread(luceneThreadStartRoutine* func, void* arg){
	    return (_LUCENE_THREADID_TYPE) ::_beginthread (func, 0, arg);
	}
	void mutex_thread::JoinThread(_LUCENE_THREADID_TYPE id){
	    WaitForSingleObject((void*)id, 0xFFFFFFFF);
	}


#elif defined(_CL_HAVE_PTHREAD)
  #ifndef _REENTRANT
      #error ACK! You need to compile with _REENTRANT defined since this uses threads
  #endif

	#ifdef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE
		bool mutex_pthread_attr_initd=false;
		pthread_mutexattr_t mutex_thread_attr;
	#endif

	#ifdef _CL__CND_DEBUG
		#define _CLPTHREAD_CHECK(c,m) CND_PRECONDITION(c==0,m)
	#else
		#define _CLPTHREAD_CHECK(c,m) c;
	#endif

	struct mutex_thread::Internal{
  	pthread_mutex_t mtx;
  	#ifndef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE
  	pthread_t lockOwner;
  	unsigned int lockCount;
  	#endif
  };

	mutex_thread::mutex_thread(const mutex_thread& /*clone*/):
		_internal(new Internal)
	{
		#ifdef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE
			_CLPTHREAD_CHECK(pthread_mutex_init(&_internal->mtx, &mutex_thread_attr), "mutex_thread(clone) constructor failed")
		#else
		  	#if defined(__hpux) && defined(_DECTHREADS_)
				_CLPTHREAD_CHECK(pthread_mutex_init(&_internal->mtx, pthread_mutexattr_default), "mutex_thread(clone) constructor failed")
			#else
				_CLPTHREAD_CHECK(pthread_mutex_init(&_internal->mtx, 0), "mutex_thread(clone) constructor failed")
			#endif
			_internal->lockCount=0;
			_internal->lockOwner=0;
		#endif
	}
	mutex_thread::mutex_thread():
		_internal(new Internal)
	{
		#ifdef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE
	  	if ( mutex_pthread_attr_initd == false ){
	  		pthread_mutexattr_init(&mutex_thread_attr);
		  	pthread_mutexattr_settype(&mutex_thread_attr, PTHREAD_MUTEX_RECURSIVE);
		  	mutex_pthread_attr_initd = true;
		}
		_CLPTHREAD_CHECK(pthread_mutex_init(&_internal->mtx, &mutex_thread_attr), "mutex_thread(clone) constructor failed")
		#else
	  	#if defined(__hpux) && defined(_DECTHREADS_)
			_CLPTHREAD_CHECK(pthread_mutex_init(&_internal->mtx, pthread_mutexattr_default), "mutex_thread(clone) constructor failed")
		#else
			_CLPTHREAD_CHECK(pthread_mutex_init(&_internal->mtx, 0), "mutex_thread(clone) constructor failed")
		#endif
		_internal->lockCount=0;
		_internal->lockOwner=0;
		#endif
	}

	mutex_thread::~mutex_thread()
	{
		_CLPTHREAD_CHECK(pthread_mutex_destroy(&_internal->mtx), "~mutex_thread destructor failed")
		delete _internal;
	}

    _LUCENE_THREADID_TYPE mutex_thread::_GetCurrentThreadId(){
        return pthread_self();
    }
      
    int32_t atomic_threads::atomic_increment(_LUCENE_ATOMIC_INT *theInteger){
      #ifdef _CL_HAVE_GCC_ATOMIC_FUNCTIONS
        return __sync_add_and_fetch(theInteger, 1);
      #else
        SCOPED_LOCK_MUTEX(theInteger->THIS_LOCK)
        return ++theInteger->value;
      #endif
    }
    int32_t atomic_threads::atomic_decrement(_LUCENE_ATOMIC_INT *theInteger){
      #ifdef _CL_HAVE_GCC_ATOMIC_FUNCTIONS
        return __sync_sub_and_fetch(theInteger, 1);
      #else
        SCOPED_LOCK_MUTEX(theInteger->THIS_LOCK)
        return --theInteger->value;
      #endif
    }


	_LUCENE_THREADID_TYPE mutex_thread::CreateThread(luceneThreadStartRoutine* func, void* arg){
	    _LUCENE_THREADID_TYPE ret;
	    int nRes = pthread_create(&ret, NULL, func, arg);
	    assert( nRes == 0 );
	    return ret;
	}
	void mutex_thread::JoinThread(_LUCENE_THREADID_TYPE id){
	    pthread_join(id, NULL);
	}

	void mutex_thread::lock()
	{
		#ifndef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE
		pthread_t currentThread = pthread_self();
		if( pthread_equal( _internal->lockOwner, currentThread ) ) {
			++_internal->lockCount;
		} else {
			_CLPTHREAD_CHECK(pthread_mutex_lock(&_internal->mtx), "mutex_thread::lock")
			_internal->lockOwner = currentThread;
			_internal->lockCount = 1;
		}
		#else
		_CLPTHREAD_CHECK(pthread_mutex_lock(&_internal->mtx), "mutex_thread::lock")
		#endif
	}

	void mutex_thread::unlock()
	{
		#ifndef _CL_HAVE_PTHREAD_MUTEX_RECURSIVE
		--_internal->lockCount;
		if( _internal->lockCount == 0 )
		{
			_internal->lockOwner = 0;
			_CLPTHREAD_CHECK(pthread_mutex_unlock(&_internal->mtx), "mutex_thread::unlock")
		}
		#else
		_CLPTHREAD_CHECK(pthread_mutex_unlock(&_internal->mtx), "mutex_thread::unlock")
		#endif
	}


	struct shared_condition::Internal{
	    pthread_cond_t condition;
	    Internal(){
	    	  pthread_cond_init (&condition, NULL);
	    }
	    ~Internal(){
			  pthread_cond_destroy(&condition);
	    }
	};
	shared_condition::shared_condition(){
		_internal = new Internal;
	}
	shared_condition::~shared_condition(){
		delete _internal;
	}
	void shared_condition::Wait(mutex_thread* shared_lock){
   	int res = 0;
   	res = pthread_cond_wait(&_internal->condition, &shared_lock->_internal->mtx);
   	assert(res == 0);
	}
	void shared_condition::NotifyAll(){
   	int res = 0;
   	res = pthread_cond_broadcast(&_internal->condition);
   	assert(res == 0);
	}


#endif //thread impl choice


mutexGuard::mutexGuard(const mutexGuard& /*clone*/){
	//no autoclone
	mrMutex = NULL;
}
mutexGuard::mutexGuard( _LUCENE_THREADMUTEX& rMutex ) :
	mrMutex(&rMutex)
{
	mrMutex->lock();
}
mutexGuard::~mutexGuard()
{
	mrMutex->unlock();
}

#endif //!_CL_DISABLE_MULTITHREADING

CL_NS_END
