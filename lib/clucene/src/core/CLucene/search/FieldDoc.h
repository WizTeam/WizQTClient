/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_FieldDoc_
#define _lucene_search_FieldDoc_

#include "SearchHeader.h"

CL_NS_DEF(search)

/**
 * Expert: A ScoreDoc which also contains information about
 * how to sort the referenced document.  In addition to the
 * document number and score, this object contains an array
 * of values for the document from the field(s) used to sort.
 * For example, if the sort criteria was to sort by fields
 * "a", "b" then "c", the <code>fields</code> object array
 * will have three elements, corresponding respectively to
 * the term values for the document in fields "a", "b" and "c".
 * The class of each element in the array will be either
 * Integer, Float or String depending on the type of values
 * in the terms of each field.
 *
 * @see ScoreDoc
 * @see TopFieldDocs
 */
class CLUCENE_EXPORT FieldDoc: LUCENE_BASE {
public:
	//FieldDoc did inherit from ScoreDoc, but now we make the scoredoc a member
	struct ScoreDoc scoreDoc;

	/** Expert: The values which are used to sort the referenced document.
	 * The order of these will match the original sort criteria given by a
	 * Sort object.  Each Object will be either an Integer, Float or String,
	 * depending on the type of values in the terms of the original field.
	 * @see Sort
	 * @see Searchable#search(Query,Filter,int32_t,Sort)
	 */
	CL_NS(util)::Comparable** fields;

	/** Expert: Creates one of these objects with empty sort information. */
	FieldDoc (int32_t doc, float_t score);
	/** Expert: Creates one of these objects with the given sort information. */
	FieldDoc (int32_t doc, float_t score, CL_NS(util)::Comparable** fields);
    ~FieldDoc();
};

CL_NS_END
#endif
