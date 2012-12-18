/*------------------------------------------------------------------------------
* Copyright (C) 2003-2011 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef _lucene_search_MockScorer
#define _lucene_search_MockScorer

#include "CLucene/search/Scorer.h"

CL_NS_DEF(search)

class MockScorer : public Scorer
{
public:

    MockScorer(Similarity* similarity) :
      Scorer(similarity), nextCalls(0), skipToCalls(0), scoreHitCollectorCalls(0), scoreCalls(0)
    {
        /* empty */
    }

    virtual bool next()
    {
        nextCalls++;
        return false;
    }

    virtual int32_t doc() const
    {
        return 0;
    }

    virtual bool skipTo(int32_t target)
    {
        skipToCalls++;
        return true;
    }

    virtual Explanation* explain(int32_t doc)
    {
        return NULL;
    }

    virtual TCHAR* toString()
    {
        return NULL;
    }

    virtual float_t score()
    {
        scoreCalls++;
        return 0;
    }

    virtual void score(HitCollector* hc)
    {
        scoreHitCollectorCalls++;
    }

    int32_t getNextCalls() const
    {
        return nextCalls;
    }

    int32_t getSkipToCalls() const
    {
        return skipToCalls;
    }

    int32_t getScoreCalls() const
    {
        return scoreCalls;
    }

    int32_t getScoreHitCollectorCalls() const
    {
        return scoreHitCollectorCalls;
    }

private:

    int32_t nextCalls;
    int32_t skipToCalls;
    int32_t scoreHitCollectorCalls;
    int32_t scoreCalls;
};

CL_NS_END

#endif // _lucene_search_MockScorer
