/*------------------------------------------------------------------------------
* Copyright (C) 2003-2011 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef _lucene_search_MockHitCollector
#define _lucene_search_MockHitCollector

#include "CLucene/search/SearchHeader.h"

CL_NS_DEF(search)

class MockHitCollector : public HitCollector
{
public:

    MockHitCollector() : HitCollector(), collectCalls(0) {}

    virtual ~MockHitCollector() {}

    virtual void collect(const int32_t doc, const float_t score)
    {
        collectCalls++;
    }

    int32_t getCollectCalls() const
    {
        return collectCalls;
    }

private:

    int32_t collectCalls;
};

CL_NS_END

#endif // _lucene_search_MockHitCollector
