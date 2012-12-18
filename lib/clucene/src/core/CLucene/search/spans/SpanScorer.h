/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_SpanScorer_
#define _lucene_search_spans_SpanScorer_

#include "CLucene/search/Scorer.h"
CL_CLASS_DEF2(search,spans,Spans)
CL_CLASS_DEF(search,Explanation)
CL_CLASS_DEF(search,Weight)

CL_NS_DEF2(search, spans)

/**
 * Public for extension only.
 */
class CLUCENE_EXPORT SpanScorer : public CL_NS(search)::Scorer 
{
protected:
    Spans *                     spans;
    CL_NS(search)::Weight *     weight;
    uint8_t*                    norms;
    float_t                     value;

    bool                        firstTime;
    bool                        more;

    int32_t                     doc_;
    float_t                     freq;


public:
    SpanScorer( Spans * spans, Weight * weight, Similarity * similarity, uint8_t* norms );    
    virtual ~SpanScorer();

	bool next();
	bool skipTo( int32_t target );
	int32_t doc() const;
	float_t score();
	CL_NS(search)::Explanation* explain( int32_t docIn );
	TCHAR* toString();

protected:
    bool setFreqCurrentDoc();
};

CL_NS_END2
#endif // _lucene_search_spans_SpanScorer_
