/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_PhrasePositions_
#define _lucene_search_PhrasePositions_

#include "CLucene/index/Terms.h"

CL_NS_DEF(search)

/**
 * Position of a term in a document that takes into account the term offset within the phrase. 
 */
class PhrasePositions:LUCENE_BASE {
public:
	int32_t doc;					  // current doc
	int32_t position;					  // position in doc
	int32_t count;					  // remaining pos in this doc
	int32_t offset;					  // position in phrase
	CL_NS(index)::TermPositions* tp;				  // stream of positions
	PhrasePositions* _next;				  // used to make lists
	bool repeats;       // there's other pp for same term (e.g. query="1st word 2nd word"~1) 

	PhrasePositions(CL_NS(index)::TermPositions* Tp, const int32_t o);
	~PhrasePositions();

	bool next();
	bool skipTo(int32_t target);

	void firstPosition();

	/**
	* Go to next location of this term current document, and set 
	* <code>position</code> as <code>location - offset</code>, so that a 
	* matching exact phrase is easily identified when all PhrasePositions 
	* have exactly the same <code>position</code>.
	*/
	bool nextPosition();
};
CL_NS_END
#endif
