/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_NearSpansUnordered_
#define _lucene_search_spans_NearSpansUnordered_

CL_CLASS_DEF(index, IndexReader)
CL_CLASS_DEF2(search, spans, SpanNearQuery)
#include "CLucene/util/PriorityQueue.h"
#include "Spans.h"

CL_NS_DEF2( search, spans )

class NearSpansUnordered : public Spans 
{
private:
    /////////////////////////////////////////////////////////////////////////////
    class SpansCell : public Spans
    {
    private:
        NearSpansUnordered *    parentSpans;
        Spans *                 spans;
        int32_t                 length;
        int32_t                 index;

    public:
        SpansCell *             nextCell;

    public:
        SpansCell( NearSpansUnordered * parentSpans, Spans * spans, int32_t index );
        virtual ~SpansCell();

        bool next()                     { return adjust( spans->next() ); }
        bool skipTo( int32_t target )   { return adjust( spans->skipTo( target )); }

        int32_t doc() const             { return spans->doc(); }
        int32_t start() const           { return spans->start(); }
        int32_t end() const             { return spans->end(); }

        TCHAR* toString() const;
        
    private:
        bool adjust( bool condition );

    };

    /////////////////////////////////////////////////////////////////////////////
    class CellQueue : public CL_NS(util)::PriorityQueue<SpansCell *, CL_NS(util)::Deletor::Object<SpansCell> >
    {
    public:
        CellQueue( int32_t size ) { initialize( size, false ); }    // All the span cells will be freed in ~NearSpansUnordered() while frein ordered member
        virtual ~CellQueue() {}

    protected:
        bool lessThan( SpansCell * spans1, SpansCell* spans2 );
    };

private:
    SpanNearQuery *     query;

    list<SpansCell *>   ordered;                // spans in query order
    int32_t             slop;                   // from query

    SpansCell *         first;                  // linked list of spans
    SpansCell *         last;                   // sorted by doc only

    int32_t             totalLength;            // sum of current lengths

    CellQueue *         queue;                  // sorted queue of spans
    SpansCell *         max;                    // max element in queue

    bool                more;                   // true iff not done
    bool                firstTime;              // true before first next()

public:
    NearSpansUnordered( SpanNearQuery * query, CL_NS(index)::IndexReader * reader );
    virtual ~NearSpansUnordered();

    bool next();
    bool skipTo( int32_t target );

    int32_t doc() const     { return min()->doc(); }
    int32_t start() const   { return min()->start(); }
    int32_t end() const     { return max->end(); }

    TCHAR* toString() const;

private:
    SpansCell * min() const { return queue->top(); }

    void initList( bool next );
    void addToList( SpansCell * cell );
    void firstToLast();
    void queueToList();
    void listToQueue();
    bool atMatch();
};

CL_NS_END2
#endif // _lucene_search_spans_NearSpansUnordered_
