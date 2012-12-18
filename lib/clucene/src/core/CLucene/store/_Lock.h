/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_intlLock_
#define _lucene_store_intlLock_

#include "Lock.h"
#include <limits.h>

CL_NS_DEF(store)


class LocksType: public CL_NS(util)::CLHashSet<const char*, CL_NS(util)::Compare::Char>
{
public:
	LocksType()
	{
		setDoDelete(false);
	}
	virtual ~LocksType(){
	}
};

  class SingleInstanceLock: public LuceneLock {
  private:
	  const char* lockName;
	  LocksType* locks;
	  DEFINE_MUTEX(*locks_LOCK)

  public:
	  SingleInstanceLock( LocksType* locks, _LUCENE_THREADMUTEX* locks_LOCK, const char* lockName );
    virtual ~SingleInstanceLock();
	  bool obtain();
	  void release();
	  bool isLocked();
	  std::string toString();

    static const char* getClassName();
    const char* getObjectName() const;
  };



  class NoLock: public LuceneLock {
  public:
	  bool obtain();
	  void release();
	  bool isLocked();
	  std::string toString();

    static const char* getClassName();
    const char* getObjectName() const;
  };

  class FSLock: public LuceneLock {
  private:
	  char* lockFile;
  	char* lockDir;
    int filemode;
  public:
	  FSLock( const char* _lockDir, const char* name, int filemode = -1 );
	  ~FSLock();

	  bool obtain();
	  void release();
	  bool isLocked();
	  std::string toString();

    static const char* getClassName();
    const char* getObjectName() const;
  };

  // Utility class for executing code with exclusive access.
  template<typename T>
  class LuceneLockWith {
  private:
    LuceneLock* lock;
    int64_t lockWaitTimeout;

  protected:
    // Code to execute with exclusive access.
    virtual T doBody() = 0;

  // Constructs an executor that will grab the named lock.
  public:
    /** Constructs an executor that will grab the named lock.
     *  Defaults lockWaitTimeout to LUCENE_COMMIT_LOCK_TIMEOUT.
     *  @deprecated Kept only to avoid breaking existing code.
     */
    LuceneLockWith(LuceneLock* lock, int64_t lockWaitTimeout) {
      this->lock = lock;
      this->lockWaitTimeout = lockWaitTimeout;
    }
    virtual ~LuceneLockWith(){
	}

    /** Calls {@link #doBody} while <i>lock</i> is obtained.  Blocks if lock
     * cannot be obtained immediately.  Retries to obtain lock once per second
     * until it is obtained, or until it has tried ten times. Lock is released when
     * {@link #doBody} exits. */
    T runAndReturn() {
        bool locked = false;
        T ret = NULL;
        try {
            locked = lock->obtain(lockWaitTimeout);
            ret = doBody();
        }_CLFINALLY(
            if (locked)
                lock->release();
        );
		return ret;
    }

	/** @see runAndReturn
     * Same as runAndReturn, except doesn't return any value.
	 * The only difference is that no void values are used
	 */
	void run() {
        bool locked = false;
        try {
            locked = lock->obtain(lockWaitTimeout);
            doBody();
        }_CLFINALLY(
            if (locked)
                lock->release();
        );
    }
  };

CL_NS_END
#endif
