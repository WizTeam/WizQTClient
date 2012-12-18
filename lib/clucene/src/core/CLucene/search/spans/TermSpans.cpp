/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#include <climits>
#include "CLucene/_ApiHeader.h"

#include <limits.h>

#include "CLucene/index/Terms.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/StringBuffer.h"

#include "_TermSpans.h"

CL_NS_DEF2(search, spans)

TermSpans::TermSpans( CL_NS(index)::TermPositions * positions, CL_NS(index)::Term * term )
{
    this->positions = positions;
    this->term = _CL_POINTER( term );
    doc_ = -1;
    freq = 0;
    count = 0;
    position = 0;
}

TermSpans::~TermSpans()
{
    _CLLDELETE( positions );
    _CLLDECDELETE( term );
}

bool TermSpans::next()
{
    if( count == freq )
    {
        if( ! positions->next())
        {
            doc_ = INT_MAX;
            return false;
        }
        doc_ = positions->doc();
        freq = positions->freq();
        count = 0;
    }
    position = positions->nextPosition();
    count++;
    return true;
}

bool TermSpans::skipTo( int32_t target )
{
    // are we already at the correct position?
    if( doc_ >= target )
        return true;

    if( !positions->skipTo( target ))
    {
        doc_ = INT_MAX;
        return false;
    }

    doc_ = positions->doc();
    freq = positions->freq();
    count = 0;

    position = positions->nextPosition();
    count++;

    return true;
}

TCHAR* TermSpans::toString() const
{
    CL_NS(util)::StringBuffer strBuf( 50 );

    TCHAR * tszTerm = term->toString();
    strBuf.append( _T( "spans(" ));
    strBuf.append( tszTerm );
    strBuf.append( _T( ")@" ));
    if( doc_ == -1 )
        strBuf.append( _T( "START" ));
    else if( doc_ == INT_MAX )
        strBuf.append( _T( "END" ));
    else
    {
        strBuf.appendInt( doc_ );
        strBuf.append( _T( "-" ));
        strBuf.appendInt( position );
    }
    _CLDELETE_CARRAY( tszTerm );

    return strBuf.toString();
}

CL_NS_END2
