/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_SpanFilterResult_
#define _lucene_search_SpanFilterResult_

#include "CLucene/util/BitSet.h"
#include "CLucene/util/VoidList.h"

CL_NS_DEF(search)

/**
 * The results of a SpanQueryFilter. 
 * Wraps the BitSet and the position information from the SpanQuery
 *
 * NOTE: This API is still experimental and subject to change. 
 **/
class CLUCENE_EXPORT SpanFilterResult 
{
public:
    class StartEnd
    {
    private:
        int32_t start;
        int32_t end;

    public:
        StartEnd( int32_t _start, int32_t _end ) : start(_start), end(_end) 
        {}

        virtual ~StartEnd() 
        {}

        /**
         * @return The end position of this match
         */
        int32_t getEnd()
        { 
            return end; 
        }

        /**
         * The Start position
         * @return The start position of this match
         */
        int32_t getStart() 
        { 
            return start; 
        }
    };


    class PositionInfo 
    {
    private:
        int32_t                         doc;
        CL_NS(util)::CLList<StartEnd*>* positions;

    public:
        PositionInfo( int32_t _doc ) : doc(_doc) 
        {
            positions = _CLNEW CL_NS(util)::CLList<StartEnd*>();
        }

        virtual ~PositionInfo() 
        {
            _CLLDELETE( positions );
        }

        void addPosition( int32_t start, int32_t end ) 
        {
            positions->push_back( _CLNEW StartEnd( start, end ));
        }

        int32_t getDoc()    
        { 
            return doc; 
        }

        /**
         * @return A List of {@link org.apache.lucene.search.SpanFilterResult.StartEnd} objects
         */
        CL_NS(util)::CLList<StartEnd*>* getPositions()
        {
            return positions;
        }
    };


private:
    CL_NS(util)::BitSet *                   bits;
    CL_NS(util)::CLList<PositionInfo *> *  positions; //Spans spans;

public:
    /**
     * Constructor
     * @param bits The bits for the Filter
     * @param positions A List of {@link org.apache.lucene.search.SpanFilterResult.PositionInfo} objects
     */
    SpanFilterResult( CL_NS(util)::BitSet * _bits, CL_NS(util)::CLList<PositionInfo*> * _positions ) :
        bits( _bits ), positions( _positions )
    {}

    virtual ~SpanFilterResult()
    {}

    /**
     * The first entry in the array corresponds to the first "on" bit.
     * Entries are increasing by document order
     * @return A List of PositionInfo objects
     */
    CL_NS(util)::CLList<PositionInfo *> * getPositions()
    {
        return positions;
    }

    CL_NS(util)::BitSet * getBits()
    {
        return bits;
    }
};
  
CL_NS_END
#endif // _lucene_search_SpanFilterResult_



