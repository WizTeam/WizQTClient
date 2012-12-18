/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_BaseTestRangeFilter
#define _lucene_search_BaseTestRangeFilter

#include "test.h"

class BaseTestRangeFilter 
{
public:
    static const bool F = false;
    static const bool T = true;
    
    RAMDirectory* index;
    
    int32_t maxR;
    int32_t minR;

    int32_t minId;
    int32_t maxId;

    const size_t intLength;

    CuTest* tc;

    BaseTestRangeFilter(CuTest* _tc);
    virtual ~BaseTestRangeFilter();

    /**
     * a simple padding function that should work with any int
     */
    std::tstring pad(int32_t n);
    
private:
    void build();

public:
    void testPad();
};


#endif

