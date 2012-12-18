/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "CLucene/_ApiHeader.h"
#include "LockFactory.h"
#include "_Lock.h"
#include "CLucene/util/Misc.h"

#ifdef _CL_HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef _CL_HAVE_UNISTD_H
	#include <unistd.h>
#endif

CL_NS_USE(util)
CL_NS_DEF(store)


LockFactory::LockFactory()
{
}

LockFactory::~LockFactory()
{
}

void LockFactory::setLockPrefix( const char* lockPrefix )
{
  if ( lockPrefix != NULL )
    this->lockPrefix = lockPrefix;
  else
    this->lockPrefix.clear();
}

const char* LockFactory::getLockPrefix()
{
	return lockPrefix.c_str();
}

SingleInstanceLockFactory::SingleInstanceLockFactory()
{
	locks = _CLNEW LocksType();
}

SingleInstanceLockFactory::~SingleInstanceLockFactory()
{
	_CLDELETE( locks );
}

LuceneLock* SingleInstanceLockFactory::makeLock( const char* lockName )
{
#ifdef _CL_DISABLE_MULTITHREADING
  return _CLNEW SingleInstanceLock( locks, NULL, lockName );
#else
	return _CLNEW SingleInstanceLock( locks, &locks_LOCK, lockName );
#endif
}

void SingleInstanceLockFactory::clearLock( const char* lockName )
{
	SCOPED_LOCK_MUTEX(locks_LOCK);
	LocksType::iterator itr = locks->find( lockName );
	if ( itr != locks->end() ) {
		locks->remove( itr );
	}
}


NoLockFactory* NoLockFactory::singleton = NULL;
NoLock* NoLockFactory::singletonLock = NULL;

void NoLockFactory::_shutdown(){
	_CLDELETE(NoLockFactory::singleton);
	_CLDELETE(NoLockFactory::singletonLock);
}

NoLockFactory* NoLockFactory::getNoLockFactory()
{
	if ( singleton == NULL ) {
		singleton = _CLNEW NoLockFactory();
	}
	return singleton;
}

LuceneLock* NoLockFactory::makeLock( const char* /*lockName*/ )
{
	if ( singletonLock == NULL ) {
		singletonLock = _CLNEW NoLock();
	}
	return singletonLock;
}

void NoLockFactory::clearLock( const char* /*lockName*/ )
{
}


FSLockFactory::FSLockFactory( const char* lockDir, int filemode )
{
  setLockDir( lockDir );
  if ( filemode > 0 )
    this->filemode = filemode;
  else
    this->filemode = 0644;
}

FSLockFactory::~FSLockFactory()
{
}

void FSLockFactory::setLockDir( const char* lockDir )
{
	this->lockDir = lockDir;
}

LuceneLock* FSLockFactory::makeLock( const char* lockName )
{
	char name[CL_MAX_DIR];

	if ( !lockPrefix.empty() ) {
		cl_sprintf(name, CL_MAX_DIR, "%s-%s", lockPrefix.c_str(), lockName);
	} else {
		cl_strcpy(name,lockName,CL_MAX_DIR);
	}

	return _CLNEW FSLock( lockDir.c_str(), name, this->filemode );
}

void FSLockFactory::clearLock( const char* lockName )
{
	if ( Misc::dir_Exists( lockDir.c_str() )) {
		char name[CL_MAX_DIR];
		char path[CL_MAX_DIR];
		struct cl_stat_t buf;

		if ( !lockPrefix.empty() ) {
			STRCPY_AtoA(name,lockPrefix.c_str(),lockPrefix.length()+1);
			strcat(name,"-");
			strcat(name,lockName);
		} else {
			strcpy(name,lockName);
		}

		_snprintf(path,CL_MAX_DIR,"%s/%s",lockDir.c_str(),name);

		int32_t ret = fileStat(path,&buf);
		if ( ret==0 && !(buf.st_mode & S_IFDIR) && _unlink( path ) == -1 ) {
			_CLTHROWA(CL_ERR_IO, "Couldn't delete file" ); // TODO: make richer error
		}
	}
}


CL_NS_END
