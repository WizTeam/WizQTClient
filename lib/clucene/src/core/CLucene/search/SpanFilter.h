/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 * 
 * Distributable under the terms of either the Apache License (Version 2.0) or 
 * the GNU Lesser General Public License, as specified in the COPYING file.
 ------------------------------------------------------------------------------*/
#ifndef _lucene_search_SpanFilter_
#define _lucene_search_SpanFilter_

#include "Filter.h"
#include "SpanFilterResult.h"

CL_NS_DEF(search)

/** Abstract base class providing a mechanism to restrict searches to a subset
 of an index and also maintains and returns position information.

 This is useful if you want to compare the positions from a SpanQuery with the positions of items in
 a filter.  For instance, if you had a SpanFilter that marked all the occurrences of the word "foo" in documents,
 and then you entered a new SpanQuery containing bar, you could not only filter by the word foo, but you could
 then compare position information for post processing.
 */
class CLUCENE_EXPORT SpanFilter : public Filter
{
public:
    virtual ~SpanFilter() {}
	
	/** Returns a SpanFilterResult with true for documents which should be permitted in
 	 * search results, and false for those that should not and Spans for where the true docs match.
 	 * @param reader The {@link org.apache.lucene.index.IndexReader} to load position and bitset information from
 	 * @return A {@link SpanFilterResult}
	 * @throws CLuceneError if there was an issue accessing the necessary information
	 */
	virtual SpanFilterResult * bitSpans( CL_NS(index)::IndexReader * reader ) = 0;

};

CL_NS_END
#endif // _lucene_search_SpanFilter_
