/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_TermScorer_
#define _lucene_search_TermScorer_


#include "Scorer.h"
#include "CLucene/index/Terms.h"
CL_CLASS_DEF(search,Similarity)
#include "SearchHeader.h"

CL_NS_DEF(search)
    
/** Expert: A <code>Scorer</code> for documents matching a <code>Term</code>.
*/
class TermScorer: public Scorer {
private:
	CL_NS(index)::TermDocs* termDocs;
	uint8_t* norms;
	Weight* weight;
	const float_t weightValue;
	int32_t _doc;

	int32_t docs[32];	  // buffered doc numbers
	int32_t freqs[32];	  // buffered term freqs
	int32_t pointer;
	int32_t pointerMax;

	float_t scoreCache[LUCENE_SCORE_CACHE_SIZE];
public:

	/** Construct a <code>TermScorer</code>.
	* @param weight The weight of the <code>Term</code> in the query.
	* @param td An iterator over the documents matching the <code>Term</code>.
	* @param similarity The </code>Similarity</code> implementation to be used for score computations.
	* @param norms The field norms of the document fields for the <code>Term</code>.
	*
	* @memory TermScorer takes TermDocs and deletes it when TermScorer is cleaned up */
	TermScorer(Weight* weight, CL_NS(index)::TermDocs* td, 
		Similarity* similarity, uint8_t* _norms);

	virtual ~TermScorer();

	/** Returns the current document number matching the query.
	* Initially invalid, until {@link #next()} is called the first time.
	*/
	int32_t doc() const;


	/** Advances to the next document matching the query.
	* <br>The iterator over the matching documents is buffered using
	* {@link TermDocs#read(int[],int[])}.
	* @return true iff there is another document matching the query.
	*/
	bool next();

	float_t score();

	/** Skips to the first match beyond the current whose document number is
	* greater than or equal to a given target. 
	* <br>The implementation uses {@link TermDocs#skipTo(int)}.
	* @param target The target document number.
	* @return true iff there is such a match.
	*/
	bool skipTo(int32_t target);

	/** Returns an explanation of the score for a document.
	* <br>When this method is used, the {@link #next()} method
	* and the {@link #score(HitCollector)} method should not be used.
	* @param doc The document number for the explanation.
	*/
	Explanation* explain(int32_t doc);

	/** Returns a string representation of this <code>TermScorer</code>. */
	virtual TCHAR* toString();
};
CL_NS_END
#endif
