/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_SpanWeight_
#define _lucene_search_spans_SpanWeight_

#include "CLucene/search/SearchHeader.h"
#include "CLucene/search/Query.h"
CL_CLASS_DEF2(search,spans,SpanQuery)
CL_CLASS_DEF(search,Similarity)
CL_CLASS_DEF(search,Searcher)

CL_NS_DEF2(search, spans)

/**
 * Expert-only. Public for use by other weight implementations
 */
class CLUCENE_EXPORT SpanWeight : public CL_NS(search)::Weight 
{
protected:
    CL_NS(search)::Similarity * similarity;
    float_t                     value;
    float_t                     idf;
    float_t                     queryNorm;
    float_t                     queryWeight;

    CL_NS(search)::TermSet *    terms;
    SpanQuery *                 query;

public:
    SpanWeight( SpanQuery * query, CL_NS(search)::Searcher * searcher );
    virtual ~SpanWeight();

    CL_NS(search)::Scorer * scorer( CL_NS(index)::IndexReader* reader );
    CL_NS(search)::Explanation * explain( CL_NS(index)::IndexReader* reader, int32_t doc );
    CL_NS(search)::Query * getQuery();
    float_t getValue();
    float_t sumOfSquaredWeights();
    void normalize( float_t norm );
};

CL_NS_END2
#endif // _lucene_search_spans_SpanWeight_
