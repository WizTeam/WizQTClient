/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"

#include <assert.h>
#include <algorithm>

#include "_NearSpansOrdered.h"
#include "SpanNearQuery.h"


CL_NS_DEF2( search, spans )

bool spanDocCompare( Spans * s1, Spans * s2 )
{
    return s1->doc() < s2->doc();
}

NearSpansOrdered::NearSpansOrdered( SpanNearQuery * spanNearQuery, CL_NS(index)::IndexReader * reader )
{
    firstTime = true;
    more = false;

    inSameDoc = false;

    matchDoc = -1;
    matchStart = -1;
    matchEnd = -1;

    if( spanNearQuery->getClausesCount() < 2 )
    {
        TCHAR * tszQry = spanNearQuery->toString();
        size_t  bBufLen = _tcslen( tszQry ) + 25;
        TCHAR * tszMsg = _CL_NEWARRAY( TCHAR, bBufLen );
		_sntprintf( tszMsg, bBufLen, _T( "Less than 2 clauses: %s" ), tszQry );
        _CLDELETE_LCARRAY( tszQry );
        _CLTHROWT_DEL( CL_ERR_IllegalArgument, tszMsg );
    }

    allowedSlop = spanNearQuery->getSlop();

    subSpansCount = spanNearQuery->getClausesCount();
    subSpans = _CL_NEWARRAY( Spans *, subSpansCount );
    subSpansByDoc = _CL_NEWARRAY( Spans *, subSpansCount );

    SpanQuery ** clauses = spanNearQuery->getClauses();
    for( size_t i = 0; i < subSpansCount; i++ )
    {
        subSpans[ i ] = clauses[ i ]->getSpans( reader );
        subSpansByDoc[ i ] = subSpans[ i ];             // used in toSameDoc()
    }
    clauses = NULL;

    query = spanNearQuery;                              // kept for toString() only.
}

NearSpansOrdered::~NearSpansOrdered()
{
    for( size_t i = 0; i < subSpansCount; i++ )
        _CLLDELETE( subSpans[ i ] );

    _CLDELETE_LARRAY( subSpans );
    _CLDELETE_LARRAY( subSpansByDoc );
}

bool NearSpansOrdered::next()
{
    if( firstTime )
    {
        firstTime = false;
        for( size_t i = 0; i < subSpansCount; i++ )
        {
            if( ! subSpans[ i ]->next() )
            {
                more = false;
                return false;
            }   
        }
        more = true;
    }
    return advanceAfterOrdered();
}

bool NearSpansOrdered::skipTo( int32_t target )
{
    if( firstTime )
    {
        firstTime = false;
        for( size_t i = 0; i < subSpansCount; i++ )
        {
            if( ! subSpans[ i ]->skipTo( target ))
            {
                more = false;
                return false;
            }
        }
        more = true;
    } 
    else if( more && ( subSpans[ 0 ]->doc() < target ))
    {
        if( subSpans[ 0 ]->skipTo( target ))
        {
            inSameDoc = false;
        } 
        else 
        {
            more = false;
            return false;
        }
    }
    return advanceAfterOrdered();
}

bool NearSpansOrdered::advanceAfterOrdered()
{
    while( more && ( inSameDoc || toSameDoc() ))
    {
        if( stretchToOrder() && shrinkToAfterShortestMatch() )
            return true;
    }
    return false; // no more matches
}

bool NearSpansOrdered::toSameDoc()
{
    sort( subSpansByDoc, subSpansByDoc + subSpansCount, spanDocCompare );
    size_t firstIndex = 0;
    int32_t maxDoc = subSpansByDoc[ subSpansCount-1 ]->doc();
    while( subSpansByDoc[ firstIndex ]->doc() != maxDoc )
    {
        if( ! subSpansByDoc[ firstIndex ]->skipTo( maxDoc ))
        {
            more = false;
            inSameDoc = false;
            return false;
        }
        maxDoc = subSpansByDoc[ firstIndex ]->doc();
        if( ++firstIndex == subSpansCount )
            firstIndex = 0;
    }

#ifdef _DEBUG
    for( size_t i = 0; i < subSpansCount; i++ )
    {
        assert( subSpansByDoc[ i ]->doc() == maxDoc );
//              : " NearSpansOrdered.toSameDoc() spans " + subSpansByDoc[0]
//                                  + "\n at doc " + subSpansByDoc[i].doc()
//                                  + ", but should be at " + maxDoc;
    }
#endif //_DEBUG

    inSameDoc = true;
    return true;
}
  
bool NearSpansOrdered::docSpansOrdered( Spans * spans1, Spans * spans2 ) 
{
    assert( spans1->doc() == spans2->doc() ); // "doc1 " + spans1.doc() + " != doc2 " + spans2.doc();
    int32_t start1 = spans1->start();
    int32_t start2 = spans2->start();
    
    /* Do not call docSpansOrdered(int,int,int,int) to avoid invoking .end() : */ // CLucene - why?
    return ( start1 == start2 ) ? ( spans1->end() < spans2->end() ) : ( start1 < start2 );
}

bool NearSpansOrdered::docSpansOrdered(int start1, int end1, int start2, int end2) 
{
    return ( start1 == start2 ) ? ( end1 < end2 ) : ( start1 < start2 );
}

bool NearSpansOrdered::stretchToOrder()
{
    matchDoc = subSpans[ 0 ]->doc();
    for( size_t i = 1; inSameDoc && ( i < subSpansCount ); i++ )
    {
        while( ! docSpansOrdered( subSpans[ i-1 ], subSpans[ i ] ))
        {
            if( ! subSpans[ i ]->next() )
            {
                inSameDoc = false;
                more = false;
                break;
            }
            else if( matchDoc != subSpans[ i ]->doc() )
            {
                  inSameDoc = false;
                  break;
            }
        }   
    }
    return inSameDoc;
}

bool NearSpansOrdered::shrinkToAfterShortestMatch()
{
    matchStart = subSpans[ subSpansCount - 1 ]->start();
    matchEnd = subSpans[ subSpansCount - 1]->end();

    int32_t matchSlop = 0;
    int32_t lastStart = matchStart;
    int32_t lastEnd   = matchEnd;

    for( int32_t i = (int32_t)subSpansCount - 2; i >= 0; i-- )
    {
        Spans * prevSpans = subSpans[ i ];
        int32_t prevStart = prevSpans->start();
        int32_t prevEnd   = prevSpans->end();
      
        while( true )                   // Advance prevSpans until after (lastStart, lastEnd)
        {
            if( ! prevSpans->next() )
            {
                inSameDoc = false;
                more = false;
                break;                  // Check remaining subSpans for final match.
            }
            else if( matchDoc != prevSpans->doc() )
            {
                inSameDoc = false;      // The last subSpans is not advanced here.
                break;                  // Check remaining subSpans for last match in this document.
            } 
            else 
            {
                int32_t ppStart = prevSpans->start();
                int32_t ppEnd   = prevSpans->end(); // Cannot avoid invoking .end()
                if( ! docSpansOrdered( ppStart, ppEnd, lastStart, lastEnd )) 
                {
                    break;              // Check remaining subSpans.
                } 
                else 
                {                       // prevSpans still before (lastStart, lastEnd)
                    prevStart = ppStart;
                    prevEnd = ppEnd;
                }
            }
        }
      
        assert( prevStart <= matchStart );
        if( matchStart > prevEnd )      // Only non overlapping spans add to slop.
            matchSlop += ( matchStart - prevEnd );

        /* Do not break on (matchSlop > allowedSlop) here to make sure
         * that subSpans[0] is advanced after the match, if any.
         */
        matchStart = prevStart;
        lastStart = prevStart;
        lastEnd = prevEnd;
    }

    return matchSlop <= allowedSlop;    // ordered and allowed slop
}

TCHAR* NearSpansOrdered::toString() const
{
    CL_NS(util)::StringBuffer buffer;
    TCHAR * tszQuery = query->toString();

    buffer.append( _T( "NearSpansOrdered(" ));
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

CL_NS_END2
