/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_EmptySpans_
#define _lucene_search_spans_EmptySpans_

#include "Spans.h"
#include <assert.h>

CL_NS_DEF2( search, spans )

/** CLucene specific: Empty span enumeration, used for optimized cases 
 *  when there are no clauses in SpanNearQuery or SpanOrQuery 
 */
class EmptySpans : public Spans 
{
public:
    EmptySpans()                    {}
    virtual ~ EmptySpans()          {}

    bool next()                     { return false; }
    bool skipTo( int32_t target )   { return false; }

    int32_t doc() const             { assert( false ); return -1; }
    int32_t start() const           { assert( false ); return 0; }
    int32_t end() const             { assert( false ); return 1; }

    TCHAR* toString() const         { return STRDUP_TtoT( _T( "spans()" )); }
};

CL_NS_END2
#endif // _lucene_search_spans_EmptySpans_
