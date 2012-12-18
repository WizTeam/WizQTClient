/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_LockFactory_
#define _lucene_store_LockFactory_

#include "CLucene/LuceneThreads.h"

CL_CLASS_DEF(store,LuceneLock)
CL_CLASS_DEF(store,NoLock)

CL_NS_DEF(store)
class LocksType;

class CLUCENE_EXPORT LockFactory: LUCENE_BASE {
protected:
  std::string lockPrefix;
public:
	
	LockFactory();
	virtual ~LockFactory();
	
	void setLockPrefix( const char* lockPrefix );
	const char* getLockPrefix();
	
	virtual LuceneLock* makeLock( const char* lockName )=0;
	virtual void clearLock( const char* lockName )=0;
};

class CLUCENE_EXPORT SingleInstanceLockFactory: public LockFactory {
private:
	LocksType* locks;
	DEFINE_MUTEX(locks_LOCK)
public:
	SingleInstanceLockFactory();
	~SingleInstanceLockFactory();
	
	LuceneLock* makeLock( const char* lockName );
	void clearLock( const char* lockName );		
};

class CLUCENE_EXPORT NoLockFactory: public LockFactory {
public:
	static NoLockFactory* singleton;
	static NoLock* singletonLock;
	
	static NoLockFactory* getNoLockFactory();
	LuceneLock* makeLock( const char* lockName );
	void clearLock( const char* lockName );
	
	/** called when lucene_shutdown is called */
	static CLUCENE_LOCAL void _shutdown();
};

class CLUCENE_EXPORT FSLockFactory: public LockFactory {
private:
  std::string lockDir;
	int filemode;
public:
  /** Constructs a FS Lock factory. The default file mode is user writable */
	FSLockFactory( const char* lockDir=NULL, int filemode=-1 );
	~FSLockFactory();
		
	void setLockDir( const char* lockDir );
	
	LuceneLock* makeLock( const char* lockName );
	void clearLock( const char* lockName );

  static const char* getClassName();
  const char* getObjectName();
};

CL_NS_END
#endif
