/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_spans_Spans_
#define _lucene_search_spans_Spans_

CL_NS_DEF2( search, spans )

/** Expert: an enumeration of span matches.  Used to implement span searching.
 * Each span represents a range of term positions within a document.  Matches
 * are enumerated in order, by increasing document number, within that by
 * increasing start position and finally by increasing end position. */
class CLUCENE_EXPORT Spans 
{
public:
    /** Empty base destructor */
    virtual ~Spans() {};

    /** Move to the next match, returning true iff any such exists. */
    virtual bool next() = 0;
  
    /** Skips to the first match beyond the current, whose document number is
     * greater than or equal to <i>target</i>. <p>Returns true iff there is such
     * a match.  <p>Behaves as if written: <pre>
     *   boolean skipTo(int target) {
     *     do {
     *       if (!next())
     * 	     return false;
     *     } while (target > doc());
     *     return true;
     *   }
     * </pre>
     * Most implementations are considerably more efficient than that.
     */
    virtual bool skipTo( int32_t target ) = 0;

    /** Returns the document number of the current match.  Initially invalid. */
    virtual int32_t doc() const = 0;

    /** Returns the start position of the current match.  Initially invalid. */
    virtual int32_t start() const = 0;

    /** Returns the end position of the current match.  Initially invalid. */
    virtual int32_t end() const = 0;

    /** Returns the string representation of the spans */
    virtual TCHAR* toString() const = 0;
};

CL_NS_END2
#endif // _lucene_search_spans_Spans_
