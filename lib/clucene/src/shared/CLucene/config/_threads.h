/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _config_threads_h
#define _config_threads_h

#ifndef _CL_DISABLE_MULTITHREADING
	#if defined(_LUCENE_DONTIMPLEMENT_THREADMUTEX)
		//do nothing
	#elif defined(_CL_HAVE_WIN32_THREADS)
      //we have not explicity included windows.h and windows.h has
      //not been included (check _WINBASE_), then we must define
      //our own definitions to the thread locking functions:
      #ifndef _WINBASE_
      extern "C"{
          struct CRITICAL_SECTION
          {
             struct critical_section_debug * DebugInfo;
             long LockCount;
             long RecursionCount;
             void * OwningThread;
             void * LockSemaphore;
             _cl_dword_t SpinCount;
          };

          __declspec(dllimport) void __stdcall InitializeCriticalSection(CRITICAL_SECTION *);
          __declspec(dllimport) void __stdcall EnterCriticalSection(CRITICAL_SECTION *);
          __declspec(dllimport) void __stdcall LeaveCriticalSection(CRITICAL_SECTION *);
          __declspec(dllimport) void __stdcall DeleteCriticalSection(CRITICAL_SECTION *);
		      __declspec(dllimport) void __stdcall ExitThread(_cl_dword_t);

    	    __declspec(dllimport) unsigned long __stdcall GetCurrentThreadId();

#ifdef _M_X64
          __declspec(dllimport) long long __stdcall _InterlockedIncrement64(__inout long long volatile*);
          __declspec(dllimport) long long __stdcall _InterlockedDecrement64(__inout long long volatile*);
#else
          __declspec(dllimport) long __stdcall InterlockedIncrement(long volatile*);
          __declspec(dllimport) long __stdcall InterlockedDecrement(long volatile*);
#endif
    	    typedef struct  _SECURITY_ATTRIBUTES
          {
            _cl_dword_t nLength;
            void* lpSecurityDescriptor;
            bool bInheritHandle;
          }	SECURITY_ATTRIBUTES;
           __declspec(dllimport) _cl_dword_t __stdcall WaitForSingleObject( void* hHandle, _cl_dword_t dwMilliseconds );
    	  __declspec(dllimport) void* __stdcall CreateEventA(  SECURITY_ATTRIBUTES* lpEventAttributes,
			  bool bManualReset, bool bInitialState, char* lpName );
		  __declspec(dllimport) bool __stdcall SetEvent(void* hEvent);
		  __declspec(dllimport) bool __stdcall CloseHandle(void* hObject);
		  void* _beginthread( void( __stdcall *start_address )( void * ), unsigned stack_size, void *arglist );
      }
      #endif //_WINBASE_
	#elif defined(_CL_HAVE_PTHREAD)
	    #include <pthread.h>
	#endif
#endif

CL_NS_DEF(util)

#ifndef _CL_DISABLE_MULTITHREADING

#if defined(_LUCENE_DONTIMPLEMENT_THREADMUTEX)

#elif defined(_CL_HAVE_WIN32_THREADS)
	class CLuceneThreadIdCompare
	{
	public:
			
		enum
		{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8
		};	// min_buckets = 2 ^^ N, 0 < N

		bool operator()( uint64_t t1, uint64_t t2 ) const{
			return t1 < t2;
		}
	};
	

#elif defined(_CL_HAVE_PTHREAD)

    class CLuceneThreadIdCompare
    {
    public:
    	enum
    	{	// parameters for hash table
    		bucket_size = 4,	// 0 < bucket_size
    		min_buckets = 8
    	};	// min_buckets = 2 ^^ N, 0 < N
    
    	bool operator()( pthread_t t1, pthread_t t2 ) const{
    	    //pthread_equal should be used, but it returns only non-zero if equal, so we can't use it for order compare
    		return t1 < t2;
    	}
    };
	
#endif //thread impl choice


#else //!_CL_DISABLE_MULTITHREADING
	class CLuceneThreadIdCompare
	{
	public:
		enum
		{	// parameters for hash table
			bucket_size = 4,	// 0 < bucket_size
			min_buckets = 8
		};	// min_buckets = 2 ^^ N, 0 < N

		bool operator()( char t1, char t2 ) const{
			return t1 < t2;
		}
	};
#endif //!_CL_DISABLE_MULTITHREADING

CL_NS_END


#endif //_config_threads_h
