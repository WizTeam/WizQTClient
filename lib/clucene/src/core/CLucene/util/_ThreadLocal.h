/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_ThreadLocal_H
#define _lucene_util_ThreadLocal_H

CL_NS_DEF ( util )


/**
* A class which holds thread specific data. Calls to get() or set() or to the data kept in the _ThreadLocal
* is invalid after _ThreadLocal has been destroyed.
*/
class _ThreadLocal
{
	private:
		class Internal;
		Internal* _internal;
	public:
		_ThreadLocal ( CL_NS ( util ) ::AbstractDeletor* _deletor );
		void* get();

		/**
		* Call this function to clear the local thread data for this
		* ThreadLocal. Calling set(NULL) does the same thing, except
		* this function is virtual and can be called without knowing
		* the template.
		*/
		void setNull();
		void set ( void* t );
		virtual ~_ThreadLocal();

		/**
		* For early cleanup of thread data, call this function. It will clear out any
		* thread specific data. Useful if you have a long running thread that doesn't
		* need to access clucene anymore.
		* The thread local code tries to call this automatically when a thread ends.
		* Some implementations may be impossible (or not implemented) to detect thread
		* endings... then you would have to run this function yourself.
		*/
		static void UnregisterCurrentThread();

        static void RemoveThreadLocal( _ThreadLocal * tl );


		/**
		* Call this function to shutdown CLucene
		*/
		static CLUCENE_LOCAL void _shutdown();

		/**
		* A hook called when CLucene is starting or shutting down,
		* this can be used for setting up and tearing down static
		* variables
		*/
		typedef void ShutdownHook ( bool startup );
};


/**
* A templated class of _ThreadLocal
* @see _ThreadLocal
*/
template<typename T,typename _deletor>
class ThreadLocal: public _ThreadLocal
{
	public:
		ThreadLocal() :
				_ThreadLocal ( _CLNEW _deletor )
		{

		}
		virtual ~ThreadLocal()
		{
		}
		T get()
		{
			return ( T ) _ThreadLocal::get();
		}
		void set ( T t )
		{
			_ThreadLocal::set ( ( T ) t );
		}
};
CL_NS_END
#endif
