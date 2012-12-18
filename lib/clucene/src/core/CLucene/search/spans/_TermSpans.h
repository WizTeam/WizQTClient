/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_TermSpans_
#define _lucene_search_spans_TermSpans_

#include "CLucene/search/spans/Spans.h"
CL_CLASS_DEF(index, TermPositions)
CL_CLASS_DEF(index, Term)

CL_NS_DEF2(search, spans)

/**
 * Expert:
 * Public for extension only
 */
class TermSpans : public Spans
{
protected:
    CL_NS(index)::TermPositions *   positions;
    CL_NS(index)::Term *            term;
    int32_t                         doc_;
    int32_t                         freq;
    int32_t                         count;
    int32_t                         position;

public:
    TermSpans( CL_NS(index)::TermPositions * positions, CL_NS(index)::Term * term );
    virtual ~TermSpans();

    bool next();
    bool skipTo( int32_t target );

    int32_t doc() const     { return doc_; }
    int32_t start() const   { return position; }
    int32_t end() const     { return position + 1; }

    TCHAR* toString() const;

    CL_NS(index)::TermPositions * getPositions() { return positions; }
};

CL_NS_END2
#endif // _lucene_search_spans_TermSpans_
