/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"

#include "_NearSpansUnordered.h"
#include "_NearSpansOrdered.h"
#include "SpanNearQuery.h"

CL_NS_DEF2( search, spans )


/////////////////////////////////////////////////////////////////////////////
NearSpansUnordered::SpansCell::SpansCell( NearSpansUnordered * parentSpans, Spans * spans, int32_t index )
{
    this->parentSpans = parentSpans;
    this->spans  = spans;
    this->index  = index;
    this->length = -1;
}

NearSpansUnordered::SpansCell::~SpansCell()
{
    _CLLDELETE( spans );
}

bool NearSpansUnordered::SpansCell::adjust( bool condition )
{
    if( length != -1 )
        parentSpans->totalLength -= length;  // subtract old length

    if( condition )
    {
        length = end() - start(); 
        parentSpans->totalLength += length;  // add new length

        if( ! parentSpans->max 
            || doc() > parentSpans->max->doc()
            || ( doc() == parentSpans->max->doc() && end() > parentSpans->max->end()))
        {
            parentSpans->max = this;
        }
    }
    parentSpans->more = condition;
    return condition;
}

TCHAR* NearSpansUnordered::SpansCell::toString() const
{
    CL_NS(util)::StringBuffer buffer;
    TCHAR * tszSpans = spans->toString();

    buffer.append( tszSpans );
    buffer.append( _T( "#" ));
    buffer.appendInt( index );

    _CLDELETE_LARRAY( tszSpans );
    return buffer.toString();
}


/////////////////////////////////////////////////////////////////////////////
bool NearSpansUnordered::CellQueue::lessThan(SpansCell * spans1, SpansCell* spans2 )
{
     if( spans1->doc() == spans2->doc() )
         return NearSpansOrdered::docSpansOrdered( spans1, spans2 );
     else 
        return spans1->doc() < spans2->doc();
}


/////////////////////////////////////////////////////////////////////////////
NearSpansUnordered::NearSpansUnordered( SpanNearQuery * query, CL_NS(index)::IndexReader * reader )
{
    // this->ordered = new ArrayList();
    this->more = true;
    this->firstTime = true;

    this->max = NULL;                       // CLucene specific, SpansCell::adjust tests this member to NULL
    this->first = NULL;                     // CLucene specific
    this->last = NULL;                      // CLucene specific, addToList test this member to NULL 
    this->totalLength = 0;                  // CLucene specific

    this->query = query;
    this->slop = query->getSlop();

    SpanQuery ** clauses = query->getClauses();
    this->queue = _CLNEW CellQueue( query->getClausesCount() );
    for( size_t i = 0; i < query->getClausesCount(); i++ )
    {
        SpansCell * cell = _CLNEW SpansCell( this, clauses[ i ]->getSpans( reader ), i );
        ordered.push_back( cell );
    }
    clauses = NULL;
}

NearSpansUnordered::~NearSpansUnordered()
{
    for( list<SpansCell *>::iterator iCell = ordered.begin(); iCell != ordered.end(); iCell++ )
        _CLLDELETE( *iCell );

    _CLLDELETE( queue );
}

bool NearSpansUnordered::next()
{
    if( firstTime )
    {
        initList( true );
        listToQueue();                          // initialize queue
        firstTime = false;
    } 
    else if( more )
    {
        if( min()->next() )                     // trigger further scanning
            queue->adjustTop();                 // maintain queue
        else 
            more = false;
    }

    while( more )
    {
        bool queueStale = false;
        if( min()->doc() != max->doc() )        // maintain list
        {             
            queueToList();
            queueStale = true;
        }

        // skip to doc w/ all clauses
        while( more && first->doc() < last->doc() )
        {
            more = first->skipTo( last->doc() );// skip first upto last
            firstToLast();                      // and move it to the end
            queueStale = true;
        }

        if( ! more ) 
            return false;

        // found doc w/ all clauses
        if( queueStale )                        // maintain the queue
        {
            listToQueue();
            queueStale = false;
        }

        if( atMatch() )
            return true;
      
        more = min()->next();
        if( more )
            queue->adjustTop();                 // maintain queue
    }

    return false;                               // no more matches
}

bool NearSpansUnordered::skipTo( int32_t target )
{
    if( firstTime )                             // initialize
    {
        initList( false );
        for( SpansCell * cell = first; more && cell; cell = cell->nextCell )
        {
            more = cell->skipTo( target );      // skip all
        }
        
        if( more )
            listToQueue();

        firstTime = false;
    } 
    else 
    {                                           // normal case
        while( more && min()->doc() < target )  // skip as needed
        {
            if( min()->skipTo( target )) 
                queue->adjustTop();
            else 
                more = false;
        }
    }

    return more && ( atMatch() ||  next() );
}

TCHAR* NearSpansUnordered::toString() const
{
    CL_NS(util)::StringBuffer buffer;
    TCHAR * tszQuery = query->toString();

    buffer.append( _T( "NearSpansUnordered(" ));
    buffer.append( tszQuery );
    buffer.append( _T( ")@" ));
    if( firstTime )
        buffer.append( _T( "START" ));
    else if( more )
    {
        buffer.appendInt( doc() );
        buffer.append( _T( ":" ));
        buffer.appendInt( start() );
        buffer.append( _T( "-" ));
        buffer.appendInt( end() );
    }
    else
        buffer.append( _T( "END" ));

    _CLDELETE_ARRAY( tszQuery );

    return buffer.toString();
}

void NearSpansUnordered::initList( bool next ) 
{
    for( list<SpansCell *>::iterator iCell = ordered.begin(); more && iCell != ordered.end(); iCell++ )
    {
        if( next )
            more = (*iCell)->next();            // move to first entry

        if( more )
            addToList( *iCell );                // add to list
    }
}

void NearSpansUnordered::addToList( SpansCell * cell )
{
    if( last )                                  // add next to end of list
        last->nextCell = cell;
    else
        first = cell;
    
    last = cell;
    cell->nextCell = NULL;
}

void NearSpansUnordered::firstToLast()
{
    last->nextCell = first;			            // move first to end of list
    last = first;
    first = first->nextCell;
    last->nextCell = NULL;
}

void NearSpansUnordered::queueToList()
{
    last = NULL;
    first = NULL;
    while( queue->top() ) 
        addToList( queue->pop() );
}
  
void NearSpansUnordered::listToQueue()
{
    queue->clear();                             // rebuild queue
    for( SpansCell * cell = first; cell; cell = cell->nextCell )
        queue->put( cell );                     // add to queue from list
}

bool NearSpansUnordered::atMatch() 
{
    return ( min()->doc() == max->doc() )
        && (( max->end() - min()->start() - totalLength ) <= slop );
}

CL_NS_END2
