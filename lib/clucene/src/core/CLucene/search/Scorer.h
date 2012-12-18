/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Scorer_
#define _lucene_search_Scorer_

CL_CLASS_DEF(search,Similarity)
CL_CLASS_DEF(search,HitCollector)
CL_CLASS_DEF(search,Explanation)

CL_NS_DEF(search)

/**
* Expert: Common scoring functionality for different types of queries.
*
* <p>
* A <code>Scorer</code> either iterates over documents matching a
* query in increasing order of doc Id, or provides an explanation of
* the score for a query for a given document.
* </p>
* <p>
* Document scores are computed using a given <code>Similarity</code>
* implementation.
* </p>
* @see BooleanQuery#setAllowDocsOutOfOrder
*/
class CLUCENE_EXPORT Scorer {
private:
	Similarity* similarity;
protected:
	/** Constructs a Scorer.
	* @param similarity The <code>Similarity</code> implementation used by this scorer.
	*/
	Scorer(Similarity* _similarity);

public:
	virtual ~Scorer();

	/** Returns the Similarity implementation used by this scorer. */
	Similarity* getSimilarity()  const;

	/** Scores and collects all matching documents.
	* @param hc The collector to which all matching documents are passed through
	* {@link HitCollector#collect(int, float)}.
	* <br>When this method is used the {@link #explain(int)} method should not be used.
	*/
	virtual void score(HitCollector* hc) ;

	/** Expert: Collects matching documents in a range.  Hook for optimization.
	* Note that {@link #next()} must be called once before this method is called
	* for the first time.
	* @param hc The collector to which all matching documents are passed through
	* {@link HitCollector#collect(int, float)}.
	* @param max Do not score documents past this.
	* @return true if more matching documents may remain.
	*/
	virtual bool score( HitCollector* results, const int32_t maxDoc );

	/**
	* Advances to the document matching this Scorer with the lowest doc Id
	* greater than the current value of {@link #doc()} (or to the matching
	* document with the lowest doc Id if next has never been called on
	* this Scorer).
	*
	* <p>
	* When this method is used the {@link #explain(int)} method should not
	* be used.
	* </p>
	*
	* @return true iff there is another document matching the query.
	* @see BooleanQuery#setAllowDocsOutOfOrder
	*/
	virtual bool next() = 0;

	/** Returns the current document number matching the query.
	* Initially invalid, until {@link #next()} is called the first time.
	*/
	virtual int32_t doc() const = 0;

	/** Returns the score of the current document matching the query.
	* Initially invalid, until {@link #next()} or {@link #skipTo(int)}
	* is called the first time.
	*/
	virtual float_t score() = 0;

	/**
	* Skips to the document matching this Scorer with the lowest doc Id
	* greater than or equal to a given target.
	*
	* <p>
	* The behavior of this method is undefined if the target specified is
	* less than or equal to the current value of {@link #doc()}.
	* <p>
	* Behaves as if written:
	* <pre>
	*   boolean skipTo(int target) {
	*     do {
	*       if (!next())
	* 	     return false;
	*     } while (target > doc());
	*     return true;
	*   }
	* </pre>
	* Most implementations are considerably more efficient than that.
	* </p>
	*
	* <p>
	* When this method is used the {@link #explain(int)} method should not
	* be used.
	* </p>
	*
	* @param target The target document number.
	* @return true iff there is such a match.
	* @see BooleanQuery#setAllowDocsOutOfOrder
	*/
	virtual bool skipTo(int32_t target) = 0;

	/** Returns an explanation of the score for a document.
	* <br>When this method is used, the {@link #next()}, {@link #skipTo(int)} and
	* {@link #score(HitCollector)} methods should not be used.
	* @param doc The document number for the explanation.
	*/
	virtual Explanation* explain(int32_t doc) = 0;

	/** Returns a string which explains the object */
	virtual TCHAR* toString() = 0;

	static bool sort(const Scorer* elem1, const Scorer* elem2);
};
CL_NS_END
#endif
