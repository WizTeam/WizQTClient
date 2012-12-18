/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_NearSpansOrdered_
#define _lucene_search_spans_NearSpansOrdered_

CL_CLASS_DEF(index, IndexReader)
CL_CLASS_DEF2(search, spans, SpanNearQuery)
#include "Spans.h"

CL_NS_DEF2( search, spans )

/** A Spans that is formed from the ordered subspans of a SpanNearQuery
 * where the subspans do not overlap and have a maximum slop between them.
 * <p>
 * The formed spans only contains minimum slop matches.<br>
 * The matching slop is computed from the distance(s) between
 * the non overlapping matching Spans.<br>
 * Successive matches are always formed from the successive Spans
 * of the SpanNearQuery.
 * <p>
 * The formed spans may contain overlaps when the slop is at least 1.
 * For example, when querying using
 * <pre>t1 t2 t3</pre>
 * with slop at least 1, the fragment:
 * <pre>t1 t2 t1 t3 t2 t3</pre>
 * matches twice:
 * <pre>t1 t2 .. t3      </pre>
 * <pre>      t1 .. t2 t3</pre>
 */
class NearSpansOrdered : public Spans 
{
private:
    int32_t         allowedSlop;
    bool            firstTime;
    bool            more;

    /** The spans in the same order as the SpanNearQuery */
    Spans **        subSpans;
    size_t          subSpansCount;

    /** Indicates that all subSpans have same doc() */
    bool            inSameDoc;

    int32_t         matchDoc;
    int32_t         matchStart;
    int32_t         matchEnd;

    Spans **        subSpansByDoc;

    SpanNearQuery * query;

public:
    NearSpansOrdered( SpanNearQuery * spanNearQuery, CL_NS(index)::IndexReader * reader );
    virtual ~NearSpansOrdered();

    bool next();
    bool skipTo( int32_t target );

    int32_t doc() const         { return matchDoc; }
    int32_t start() const       { return matchStart; }
    int32_t end() const         { return matchEnd; }

    TCHAR* toString() const;

    /** Check whether two Spans in the same document are ordered.
     * @param spans1 
     * @param spans2 
     * @return true iff spans1 starts before spans2
     *              or the spans start at the same position,
     *              and spans1 ends before spans2.
     */
    static bool docSpansOrdered( Spans * spans1, Spans * spans2 );

private:
    /** Advances the subSpans to just after an ordered match with a minimum slop
     * that is smaller than the slop allowed by the SpanNearQuery.
     * @return true iff there is such a match.
     */
    bool advanceAfterOrdered();
  
    /** Advance the subSpans to the same document */
    bool toSameDoc();

    /** Like {@link #docSpansOrdered(Spans,Spans)}, but use the spans
     * starts and ends as parameters.
     */
    static bool docSpansOrdered( int32_t start1, int32_t end1, int32_t start2, int32_t end2 );

    /** Order the subSpans within the same document by advancing all later spans
     * after the previous one.
     */
    bool stretchToOrder();

    /** The subSpans are ordered in the same doc, so there is a possible match.
     * Compute the slop while making the match as short as possible by advancing
     * all subSpans except the last one in reverse order.
     */
    bool shrinkToAfterShortestMatch();
};

CL_NS_END2
#endif // _lucene_search_spans_NearSpansOrdered_
