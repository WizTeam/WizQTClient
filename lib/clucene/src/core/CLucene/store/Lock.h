/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_Lock_
#define _lucene_store_Lock_

#include <limits.h>
#include "CLucene/util/Equators.h"

CL_NS_DEF(store)
  class LocksType;

  class CLUCENE_EXPORT LuceneLock: public CL_NS(util)::NamedObject{
  public:
      LUCENE_STATIC_CONSTANT(int64_t, LOCK_POLL_INTERVAL = 1000);
      LUCENE_STATIC_CONSTANT(int64_t, LOCK_OBTAIN_WAIT_FOREVER = -1);

      /** Attempts to obtain exclusive access and immediately return
      *  upon success or failure.
      * @return true iff exclusive access is obtained
      */
      virtual bool obtain() = 0;

      /** Attempts to obtain an exclusive lock within amount
      *  of time given. Currently polls once per second until
      *  lockWaitTimeout is passed.
      * @param lockWaitTimeout length of time to wait in ms
      * @return true if lock was obtained
      * @throws IOException if lock wait times out or obtain() throws an IOException
      */
      bool obtain(int64_t lockWaitTimeout);

      // Release exclusive access.
      virtual void release() = 0;

      /** Returns true if the resource is currently locked.  Note that one must
      * still call {@link #obtain()} before using the resource. */
      virtual bool isLocked() = 0;

      virtual ~LuceneLock();

      virtual std::string toString() = 0;
  };


CL_NS_END
#endif
