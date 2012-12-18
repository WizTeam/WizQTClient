/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Compare_
#define _lucene_search_Compare_


//#include "FieldSortedHitQueue.h"
#include "Sort.h"
#include "FieldCache.h"

CL_NS_DEF(search)


class CLUCENE_EXPORT ScoreDocComparators:LUCENE_BASE {
protected:
	ScoreDocComparators();
public:
  ~ScoreDocComparators();

	class CLUCENE_EXPORT Relevance:public ScoreDocComparator {
	public:
		int32_t compare (struct ScoreDoc* i, struct ScoreDoc* j);
		CL_NS(util)::Comparable* sortValue (struct ScoreDoc* i);
		int32_t sortType();
	};

	class CLUCENE_EXPORT IndexOrder:public ScoreDocComparator{
	public:
		IndexOrder();
		int32_t compare (struct ScoreDoc* i, struct ScoreDoc* j);
		CL_NS(util)::Comparable* sortValue (struct ScoreDoc* i);
		int32_t sortType();
	};


	class CLUCENE_EXPORT String: public ScoreDocComparator {
		FieldCache::StringIndex* index;
		int32_t length;
	public:
		String(FieldCache::StringIndex* index, int32_t len);
		int32_t compare (struct ScoreDoc* i, struct ScoreDoc* j);
		CL_NS(util)::Comparable* sortValue (struct ScoreDoc* i);

		int32_t sortType();
	};

	class CLUCENE_EXPORT Int32:public ScoreDocComparator{
		int32_t* fieldOrder;
		int32_t length;
	public:
		Int32(int32_t* fieldOrder, int32_t len);
		int32_t compare (struct ScoreDoc* i, struct ScoreDoc* j);
		CL_NS(util)::Comparable* sortValue (struct ScoreDoc* i);
		int32_t sortType();
	};

	class CLUCENE_EXPORT Float:public ScoreDocComparator {
		float_t* fieldOrder;
		int32_t length;
	public:
		Float(float_t* fieldOrder, int32_t len);
		int32_t compare (struct ScoreDoc* i, struct ScoreDoc* j);
		CL_NS(util)::Comparable* sortValue (struct ScoreDoc* i);
		int32_t sortType();
	};
};


CL_NS_END
#endif
