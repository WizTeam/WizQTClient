/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_QueryFilter_
#define _lucene_search_QueryFilter_

//#include "CLucene/util/BitSet.h"
//#include "CLucene/index/IndexReader.h"
//#include "SearchHeader.h"
#include "Filter.h"
CL_CLASS_DEF(search,Query)
//#include "CachingWrapperFilter.h"

CL_NS_DEF(search)

class CLUCENE_EXPORT QueryFilter: public Filter
{
private:
	Query* query;
    bool   bDeleteQuery;
protected:
	QueryFilter( const QueryFilter& copy );
public:
	QueryFilter( const Query* query );
	QueryFilter( Query* query, bool bDeleteQuery );
	
	~QueryFilter();
	
	CL_NS(util)::BitSet* bits( CL_NS(index)::IndexReader* reader );
	
	Filter *clone() const;
	
	TCHAR *toString();
};

CL_NS_END
#endif
