/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/util/Misc.h"

#include "CLucene/search/Sort.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/search/FieldCache.h"
#include "CLucene/index/TermVector.h"
#include "CLucene/index/_IndexFileNameFilter.h"
#include "CLucene/search/FieldSortedHitQueue.h"
#include "CLucene/store/LockFactory.h"
#include "CLucene/util/_StringIntern.h"
#include "CLucene/util/_ThreadLocal.h"

#if defined(_MSC_VER) && defined(_DEBUG)
	#define CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

CL_NS_USE(util)
CL_NS_USE(search)
CL_NS_USE(index)
CL_NS_USE(store)

//clears all static memory. do not attempt to do anything else
//in clucene after calling this function
void _lucene_shutdown(){
  FieldSortedHitQueue::_shutdown();
  Sort::_shutdown();
  ScoreDocComparator::_shutdown();
  SortField::_shutdown();
  FieldCache::_shutdown();
  Similarity::_shutdown();
  CLStringIntern::_shutdown();
  NoLockFactory::_shutdown();
  _ThreadLocal::_shutdown();
  IndexFileNameFilter::_shutdown();
  _CLDELETE (TermVectorOffsetInfo_EMPTY_OFFSET_INFO);
}
