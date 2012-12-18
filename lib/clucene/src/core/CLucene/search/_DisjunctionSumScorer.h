/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_DisjunctionSumScorer_
#define _lucene_search_DisjunctionSumScorer_

CL_NS_USE(util)
CL_NS_DEF(search)

/** A Scorer for OR like queries, counterpart of <code>ConjunctionScorer</code>.
* This Scorer implements {@link Scorer#skipTo(int)} and uses skipTo() on the given Scorers.
* @java-todo Implement score(HitCollector, int).
*/
class DisjunctionSumScorer : public Scorer {
public:
	typedef CL_NS(util)::CLVector<Scorer*,CL_NS(util)::Deletor::Object<Scorer> > ScorersType;
private:
	/** The minimum number of scorers that should match. */
	int32_t minimumNrMatchers;

	/** The scorerDocQueue contains all subscorers ordered by their current doc(),
	* with the minimum at the top.
	* <br>The scorerDocQueue is initialized the first time next() or skipTo() is called.
	* <br>An exhausted scorer is immediately removed from the scorerDocQueue.
	* <br>If less than the minimumNrMatchers scorers
	* remain in the scorerDocQueue next() and skipTo() return false.
	* <p>
	* After each to call to next() or skipTo()
	* <code>currentSumScore</code> is the total score of the current matching doc,
	* <code>nrMatchers</code> is the number of matching scorers,
	* and all scorers are after the matching doc, or are exhausted.
	*/
	ScorerDocQueue* scorerDocQueue;
	int32_t queueSize; // used to avoid size() method calls on scorerDocQueue

	/** The document number of the current match. */
	int32_t currentDoc;
	float_t currentScore;

	/** Called the first time next() or skipTo() is called to
	* initialize <code>scorerDocQueue</code>.
	*/
	void initScorerDocQueue();

protected:
	/** The number of subscorers. */
	int32_t nrScorers;

	/** The subscorers. */
	DisjunctionSumScorer::ScorersType subScorers;

	/** The number of subscorers that provide the current match. */
	int32_t _nrMatchers;

	/** Expert: Collects matching documents in a range.  Hook for optimization.
	* Note that {@link #next()} must be called once before this method is called
	* for the first time.
	* @param hc The collector to which all matching documents are passed through
	* {@link HitCollector#collect(int, float)}.
	* @param max Do not score documents past this.
	* @return true if more matching documents may remain.
	*/
	bool score( HitCollector* hc, const int32_t max );

	/** Advance all subscorers after the current document determined by the
	* top of the <code>scorerDocQueue</code>.
	* Repeat until at least the minimum number of subscorers match on the same
	* document and all subscorers are after that document or are exhausted.
	* <br>On entry the <code>scorerDocQueue</code> has at least <code>minimumNrMatchers</code>
	* available. At least the scorer with the minimum document number will be advanced.
	* @return true iff there is a match.
	* <br>In case there is a match, </code>currentDoc</code>, </code>currentSumScore</code>,
	* and </code>nrMatchers</code> describe the match.
	*
	* @todo Investigate whether it is possible to use skipTo() when
	* the minimum number of matchers is bigger than one, ie. try and use the
	* character of ConjunctionScorer for the minimum number of matchers.
	* Also delay calling score() on the sub scorers until the minimum number of
	* matchers is reached.
	* <br>For this, a Scorer array with minimumNrMatchers elements might
	* hold Scorers at currentDoc that are temporarily popped from scorerQueue.
	*/
	bool advanceAfterCurrent();

public:
	/** Construct a <code>DisjunctionScorer</code>, using one as the minimum number
	* of matching subscorers.
	* @param subScorers A collection of at least two subscorers.
	* @param minimumNrMatchers The positive minimum number of subscorers that should
	* match to match this query.
	* <br>When <code>minimumNrMatchers</code> is bigger than
	* the number of <code>subScorers</code>,
	* no matches will be produced.
	* <br>When minimumNrMatchers equals the number of subScorers,
	* it more efficient to use <code>ConjunctionScorer</code>.
	*/
	DisjunctionSumScorer( DisjunctionSumScorer::ScorersType* _subScorers, const int32_t _minimumNrMatchers = 1);
	virtual ~DisjunctionSumScorer();

	/** Scores and collects all matching documents.
	* @param hc The collector to which all matching documents are passed through
	* {@link HitCollector#collect(int, float)}.
	* <br>When this method is used the {@link #explain(int)} method should not be used.
	*/
	void score( HitCollector* hc );
	bool next();

	/** Returns the score of the current document matching the query.
	* Initially invalid, until {@link #next()} is called the first time.
	*/
	virtual float_t score();

	int32_t doc() const;

	/** Returns the number of subscorers matching the current document.
	* Initially invalid, until {@link #next()} is called the first time.
	*/
	int32_t nrMatchers() const;

	/** Skips to the first match beyond the current whose document number is
	* greater than or equal to a given target.
	* <br>When this method is used the {@link #explain(int)} method should not be used.
	* <br>The implementation uses the skipTo() method on the subscorers.
	* @param target The target document number.
	* @return true iff there is such a match.
	*/
	bool skipTo( int32_t target );

	virtual TCHAR* toString();

	/** @return An explanation for the score of a given document. */
	Explanation* explain( int32_t doc );
};

CL_NS_END
#endif
