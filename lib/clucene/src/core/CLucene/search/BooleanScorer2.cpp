/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_BooleanScorer2.h"

#include "Scorer.h"
#include "SearchHeader.h"
#include "Similarity.h"
#include "ScorerDocQueue.h"
#include "Explanation.h"

#include "_BooleanScorer.h"
#include "_BooleanScorer.h"
#include "_ConjunctionScorer.h"
#include "_DisjunctionSumScorer.h"

CL_NS_USE(util)
CL_NS_DEF(search)


class BooleanScorer2::Coordinator {
public:
	int32_t maxCoord;
	int32_t nrMatchers; // to be increased by score() of match counting scorers.
	float_t* coordFactors;
	Scorer* parentScorer;

	Coordinator( Scorer* parent ):
		maxCoord(0),
		nrMatchers(0),
		coordFactors(NULL),
		parentScorer(parent)
	{
	}

	virtual ~Coordinator()
	{
		_CLDELETE_ARRAY(coordFactors);
	}

	void init()
	{
		coordFactors = _CL_NEWARRAY( float_t, maxCoord+1 );
		Similarity* sim = parentScorer->getSimilarity();
		for ( int32_t i = 0; i <= maxCoord; i++ ) {
			coordFactors[i] = sim->coord(i, maxCoord);
		}
	}


	void initDoc() {
		nrMatchers = 0;
	}

	float_t coordFactor() {
		return coordFactors[nrMatchers];
	}
};

class BooleanScorer2::SingleMatchScorer: public Scorer {
public:
	Scorer* scorer;
	Coordinator* coordinator;
	int32_t lastScoredDoc;

	SingleMatchScorer( Scorer* _scorer, Coordinator* _coordinator ) :
    Scorer( _scorer->getSimilarity() ), scorer(_scorer), coordinator(_coordinator), lastScoredDoc(-1)
	{
	}

	virtual ~SingleMatchScorer()
	{
		_CLDELETE( scorer );
	}

	float_t score()
	{
		if ( doc() >= lastScoredDoc ) {
			lastScoredDoc = this->doc();
			coordinator->nrMatchers++;
		}
		return scorer->score();
	}

	int32_t doc() const {
		return scorer->doc();
	}

	bool next() {
		return scorer->next();
	}

	bool skipTo( int32_t docNr ) {
		return scorer->skipTo( docNr );
	}

	virtual TCHAR* toString() {
		return scorer->toString();
	}

	Explanation* explain(int32_t doc) {
		return scorer->explain( doc );
	}

};

/** A scorer that matches no document at all. */
class BooleanScorer2::NonMatchingScorer: public Scorer {
public:

	NonMatchingScorer() :
		Scorer( NULL )
	{
	}
	virtual ~NonMatchingScorer() {};

	int32_t doc() const {
		_CLTHROWA(CL_ERR_UnsupportedOperation, "UnsupportedOperationException: BooleanScorer2::NonMatchingScorer::doc");
		return 0;
	}
	bool next() { return false; }
	float_t score() {
		_CLTHROWA(CL_ERR_UnsupportedOperation, "UnsupportedOperationException: BooleanScorer2::NonMatchingScorer::score");
		return 0.0;
	}
	bool skipTo( int32_t /*target*/ ) { return false; }
	virtual TCHAR* toString() { return stringDuplicate(_T("NonMatchingScorer")); }

	Explanation* explain( int32_t /*doc*/ ) {
		Explanation* e = _CLNEW Explanation();
		e->setDescription(_T("No document matches."));
		return e;
	}

};

/** A Scorer for queries with a required part and an optional part.
 * Delays skipTo() on the optional part until a score() is needed.
 * <br>
 * This <code>Scorer</code> implements {@link Scorer#skipTo(int)}.
 */
class BooleanScorer2::ReqOptSumScorer: public Scorer {
private:
	/** The scorers passed from the constructor.
	* These are set to null as soon as their next() or skipTo() returns false.
	*/
	Scorer* reqScorer;
	Scorer* optScorer;
	bool firstTimeOptScorer;

public:
	/** Construct a <code>ReqOptScorer</code>.
	* @param reqScorer The required scorer. This must match.
	* @param optScorer The optional scorer. This is used for scoring only.
	*/
	ReqOptSumScorer( Scorer* _reqScorer, Scorer* _optScorer ) :
      Scorer( NULL ), reqScorer(_reqScorer), optScorer(_optScorer), firstTimeOptScorer(true)
	{
	}

	virtual ~ReqOptSumScorer()
	{
		_CLDELETE( reqScorer );
		_CLDELETE( optScorer );
	}

	/** Returns the score of the current document matching the query.
	* Initially invalid, until {@link #next()} is called the first time.
	* @return The score of the required scorer, eventually increased by the score
	* of the optional scorer when it also matches the current document.
	*/
	float_t score()
	{
		int32_t curDoc = reqScorer->doc();
		float_t reqScore = reqScorer->score();

		if ( firstTimeOptScorer ) {
			firstTimeOptScorer = false;
			if ( !optScorer->skipTo( curDoc ) ) {
				_CLDELETE(optScorer);
				return reqScore;
			}
		} else if ( optScorer == NULL ) {
			return reqScore;
		} else if (( optScorer->doc() < curDoc ) && ( !optScorer->skipTo( curDoc ))) {
			_CLDELETE(optScorer);
			return reqScore;
		}

		return ( optScorer->doc() == curDoc )
			? reqScore + optScorer->score()
			: reqScore;
	}

	int32_t doc() const {
		return reqScorer->doc();
	}

	bool next() {
		return reqScorer->next();
	}

	bool skipTo( int32_t target ) {
		return reqScorer->skipTo( target );
	}

	virtual TCHAR* toString() {
		return stringDuplicate(_T("ReqOptSumScorer"));
	}

	/** Explain the score of a document.
	* @todo Also show the total score.
	* See BooleanScorer.explain() on how to do this.
	*/
	Explanation* explain( int32_t doc ) {
		Explanation* res = _CLNEW Explanation();
		res->setDescription(_T("required, optional"));
		res->addDetail(reqScorer->explain(doc));
		res->addDetail(optScorer->explain(doc));
		return res;
	}
};


/** A Scorer for queries with a required subscorer and an excluding (prohibited) subscorer.
* <br>
* This <code>Scorer</code> implements {@link Scorer#skipTo(int)},
* and it uses the skipTo() on the given scorers.
*/
class BooleanScorer2::ReqExclScorer: public Scorer {
private:
	Scorer* reqScorer;
	Scorer* exclScorer;
	bool firstTime;

public:
	/** Construct a <code>ReqExclScorer</code>.
	* @param reqScorer The scorer that must match, except where
	* @param exclScorer indicates exclusion.
	*/
	ReqExclScorer( Scorer* _reqScorer, Scorer* _exclScorer ) :
      Scorer( NULL ), reqScorer(_reqScorer), exclScorer(_exclScorer), firstTime(true)
	{
	}

	virtual ~ReqExclScorer()
	{
		_CLDELETE( reqScorer );
		_CLDELETE( exclScorer );
	}

	int32_t doc() const {
		return reqScorer->doc();
	}

	/** Returns the score of the current document matching the query.
	* Initially invalid, until {@link #next()} is called the first time.
	* @return The score of the required scorer.
	*/
	float_t score() {
		return reqScorer->score();
	}

	virtual TCHAR* toString() {
		return stringDuplicate(_T("ReqExclScorer"));
	}

	Explanation* explain( int32_t doc ) {
		Explanation* res = _CLNEW Explanation();
		if (exclScorer->skipTo(doc) && (exclScorer->doc() == doc)) {
			res->setDescription(_T("excluded"));
		} else {
			res->setDescription(_T("not excluded"));
			res->addDetail(reqScorer->explain(doc));
		}
		return res;
	}


	bool next()
	{
		if ( firstTime ) {
			if ( !exclScorer->next() ) {
				_CLDELETE( exclScorer );
			}
			firstTime = false;
		}
		if ( reqScorer == NULL ) {
			return false;
		}
		if ( !reqScorer->next() ) {
			_CLDELETE( reqScorer ); // exhausted, nothing left
			return false;
		}
		if ( exclScorer == NULL ) {
			return true;// reqScorer.next() already returned true
		}
		return toNonExcluded();
	}


	/** Skips to the first match beyond the current whose document number is
	* greater than or equal to a given target.
	* <br>When this method is used the {@link #explain(int)} method should not be used.
	* @param target The target document number.
	* @return true iff there is such a match.
	*/
	bool skipTo( int32_t target )
	{
		if ( firstTime ) {
			firstTime = false;
			if ( !exclScorer->skipTo( target )) {
				_CLDELETE( exclScorer ); // exhausted
			}
		}
		if ( reqScorer == NULL ) {
			return false;
		}
		if ( exclScorer == NULL ) {
			return reqScorer->skipTo( target );
		}
		if ( !reqScorer->skipTo( target )) {
			_CLDELETE( reqScorer );
			return false;
		}
		return toNonExcluded();
	}


private:
	/** Advance to non excluded doc.
	* <br>On entry:
	* <ul>
	* <li>reqScorer != null,
	* <li>exclScorer != null,
	* <li>reqScorer was advanced once via next() or skipTo()
	*      and reqScorer.doc() may still be excluded.
	* </ul>
	* Advances reqScorer a non excluded required doc, if any.
	* @return true iff there is a non excluded required doc.
	*/
	bool toNonExcluded()
	{
		int32_t exclDoc = exclScorer->doc();

		do {
			int32_t reqDoc = reqScorer->doc();
			if ( reqDoc < exclDoc ) {
				return true; // reqScorer advanced to before exclScorer, ie. not excluded
			} else if ( reqDoc > exclDoc ) {
				if (! exclScorer->skipTo(reqDoc)) {
					_CLDELETE( exclScorer ); // exhausted, no more exclusions
					return true;
				}
			  exclDoc = exclScorer->doc();
			  if ( exclDoc > reqDoc ) {
				  return true; // not excluded
			  }
			}
		} while ( reqScorer->next() );

		_CLDELETE( reqScorer ); // exhausted, nothing left
		return false;
	}

};

class BooleanScorer2::BSConjunctionScorer: public CL_NS(search)::ConjunctionScorer {
private:
	CL_NS(search)::BooleanScorer2::Coordinator* coordinator;
	int32_t lastScoredDoc;
	int32_t requiredNrMatchers;
  typedef CL_NS(util)::CLVector<Scorer*,CL_NS(util)::Deletor::Object<Scorer> > ScorersType;
public:
	BSConjunctionScorer( CL_NS(search)::BooleanScorer2::Coordinator* _coordinator, ScorersType* _requiredScorers, int32_t _requiredNrMatchers ):
		ConjunctionScorer( Similarity::getDefault(), _requiredScorers ),
		coordinator(_coordinator),
		lastScoredDoc(-1),
		requiredNrMatchers(_requiredNrMatchers)
	{
	}

	virtual ~BSConjunctionScorer(){
	}
	float_t score()
	{
		if ( this->doc() >= lastScoredDoc ) {
			lastScoredDoc = this->doc();
			coordinator->nrMatchers += requiredNrMatchers;
		}
		return ConjunctionScorer::score();
	}
	virtual TCHAR* toString() {return stringDuplicate(_T("BSConjunctionScorer"));}
};

class BooleanScorer2::BSDisjunctionSumScorer: public CL_NS(search)::DisjunctionSumScorer {
private:
	CL_NS(search)::BooleanScorer2::Coordinator* coordinator;
	int32_t lastScoredDoc;
  typedef CL_NS(util)::CLVector<Scorer*,CL_NS(util)::Deletor::Object<Scorer> > ScorersType;
public:
	BSDisjunctionSumScorer(
		CL_NS(search)::BooleanScorer2::Coordinator* _coordinator,
		ScorersType* subScorers,
		int32_t minimumNrMatchers ):
			DisjunctionSumScorer( subScorers, minimumNrMatchers ),
			coordinator(_coordinator),
			lastScoredDoc(-1)
	{
	}

	float_t score() {
		if ( this->doc() >= lastScoredDoc ) {
			lastScoredDoc = this->doc();
			coordinator->nrMatchers += _nrMatchers;
		}
		return DisjunctionSumScorer::score();
	}

	virtual ~BSDisjunctionSumScorer(){
	}
	virtual TCHAR* toString() {return stringDuplicate(_T("BSDisjunctionSumScorer"));}
};

class BooleanScorer2::Internal{
public:
  typedef CL_NS(util)::CLVector<Scorer*,CL_NS(util)::Deletor::Object<Scorer> > ScorersType;

	ScorersType requiredScorers;
	ScorersType optionalScorers;
	ScorersType prohibitedScorers;

	BooleanScorer2::Coordinator *coordinator;
	Scorer* countingSumScorer;

	size_t minNrShouldMatch;
	bool allowDocsOutOfOrder;


	void initCountingSumScorer()
	{
		coordinator->init();
		countingSumScorer = makeCountingSumScorer();
	}

	Scorer* countingDisjunctionSumScorer( ScorersType* scorers, int32_t minNrShouldMatch )
	{
		return _CLNEW BSDisjunctionSumScorer( coordinator, scorers, minNrShouldMatch );
	}

	Scorer* countingConjunctionSumScorer( ScorersType* requiredScorers )
	{
		return _CLNEW BSConjunctionScorer( coordinator, requiredScorers, requiredScorers->size() );
	}

	Scorer* dualConjunctionSumScorer( Scorer* req1, Scorer* req2 )
	{
    ValueArray<Scorer*> scorers(2);
    scorers[0] = req1;
    scorers[1] = req2;
		return _CLNEW CL_NS(search)::ConjunctionScorer( Similarity::getDefault(), &scorers );

	}

	Scorer* makeCountingSumScorer()
	{
		return ( requiredScorers.size() == 0 ) ?
      makeCountingSumScorerNoReq() :
      makeCountingSumScorerSomeReq();
	}

	Scorer* makeCountingSumScorerNoReq()
	{
		if ( optionalScorers.size() == 0 ) {
			optionalScorers.setDoDelete(true);
			return _CLNEW NonMatchingScorer();
		} else {
			size_t nrOptRequired = ( minNrShouldMatch < 1 ) ? 1 : minNrShouldMatch;
			if ( optionalScorers.size() < nrOptRequired ) {
				optionalScorers.setDoDelete(true);
				return _CLNEW NonMatchingScorer();
			} else {
				Scorer* requiredCountingSumScorer =
					( optionalScorers.size() > nrOptRequired )
					? countingDisjunctionSumScorer( &optionalScorers, nrOptRequired )
					:
					( optionalScorers.size() == 1 )
					? _CLNEW SingleMatchScorer((Scorer*) optionalScorers[0], coordinator)
					: countingConjunctionSumScorer( &optionalScorers );
				return addProhibitedScorers( requiredCountingSumScorer );
			}
		}
	}

	Scorer* makeCountingSumScorerSomeReq()
	{
		if ( optionalScorers.size() < minNrShouldMatch ) {
			requiredScorers.setDoDelete(true);
			optionalScorers.setDoDelete(true);
			return _CLNEW NonMatchingScorer();
		} else if ( optionalScorers.size() == minNrShouldMatch ) {
			Internal::ScorersType allReq( false );
			for ( Internal::ScorersType::iterator it = requiredScorers.begin(); it != requiredScorers.end(); it++ ) {
				allReq.push_back( *it );
			}
			for ( Internal::ScorersType::iterator it2 = optionalScorers.begin(); it2 != optionalScorers.end(); it2++ ) {
				allReq.push_back( *it2 );
			}
			return addProhibitedScorers( countingConjunctionSumScorer( &allReq ));
		} else {
			Scorer* requiredCountingSumScorer =
				( requiredScorers.size() == 1 )
				? _CLNEW SingleMatchScorer( (Scorer*)requiredScorers[0], coordinator )
				: countingConjunctionSumScorer( &requiredScorers );
			if ( minNrShouldMatch > 0 ) {
				return addProhibitedScorers(
					dualConjunctionSumScorer(
							requiredCountingSumScorer,
							countingDisjunctionSumScorer(
								&optionalScorers,
								minNrShouldMatch )));
			} else {
				return _CLNEW ReqOptSumScorer(
						addProhibitedScorers( requiredCountingSumScorer ),
						(( optionalScorers.size() == 1 )
								? _CLNEW SingleMatchScorer( (Scorer*)optionalScorers[0], coordinator )
								: countingDisjunctionSumScorer( &optionalScorers, 1 )));
			}
		}
	}

	Scorer* addProhibitedScorers( Scorer* requiredCountingSumScorer )
	{
		return ( prohibitedScorers.size() == 0 )
			? requiredCountingSumScorer
			: _CLNEW ReqExclScorer( requiredCountingSumScorer,
									(( prohibitedScorers.size() == 1 )
											? (Scorer*)prohibitedScorers[0]
											: _CLNEW CL_NS(search)::DisjunctionSumScorer( &prohibitedScorers )));
	}


	Internal( BooleanScorer2* parent, int32_t _minNrShouldMatch, bool _allowDocsOutOfOrder ):
		requiredScorers(false),
		optionalScorers(false),
		prohibitedScorers(false),
	  countingSumScorer(NULL),
		minNrShouldMatch(_minNrShouldMatch),
		allowDocsOutOfOrder(_allowDocsOutOfOrder)
	{
		if ( _minNrShouldMatch < 0 ) {
      _CLTHROWA(CL_ERR_IllegalArgument, "Minimum number of optional scorers should not be negative");
		}

		this->coordinator = _CLNEW Coordinator( parent );

	}
	~Internal(){
		_CLDELETE( coordinator );
		_CLDELETE( countingSumScorer );
		/* TODO: these leak memory... haven't figure out how it should be fixed though...
		requiredScorers.clear();
		optionalScorers.clear();
		prohibitedScorers.clear();
		*/
	}

};





BooleanScorer2::BooleanScorer2( Similarity* similarity, int32_t minNrShouldMatch, bool allowDocsOutOfOrder ):
	Scorer( similarity )
{
	_internal = new Internal(this, minNrShouldMatch, allowDocsOutOfOrder);
}

BooleanScorer2::~BooleanScorer2()
{
	delete _internal;
}

void BooleanScorer2::add( Scorer* scorer, bool required, bool prohibited )
{
	if ( !prohibited ) {
		_internal->coordinator->maxCoord++;
	}

	if ( required ) {
		if ( prohibited ) {
			_CLTHROWA(CL_ERR_IllegalArgument, "scorer cannot be required and prohibited");
		}
		_internal->requiredScorers.push_back( scorer );
	} else if ( prohibited ) {
		_internal->prohibitedScorers.push_back( scorer );
	} else {
		_internal->optionalScorers.push_back( scorer );
	}

}

void BooleanScorer2::score( HitCollector* hc )
{
	if ( _internal->allowDocsOutOfOrder && _internal->requiredScorers.size() == 0 && _internal->prohibitedScorers.size() < 32 ) {

		BooleanScorer* bs = _CLNEW BooleanScorer( getSimilarity(), _internal->minNrShouldMatch );
		Internal::ScorersType::iterator si = _internal->optionalScorers.begin();
		while ( si != _internal->optionalScorers.end() ) {
			bs->add( (*si), false /* required */, false /* prohibited */ );
			si++;
		}
		si = _internal->prohibitedScorers.begin();
		while ( si != _internal->prohibitedScorers.end() ) {
			bs->add( (*si), false /* required */, true /* prohibited */ );
			si++;
		}
		bs->score( hc );
	} else {
		if ( _internal->countingSumScorer == NULL ) {
			_internal->initCountingSumScorer();
		}
		while ( _internal->countingSumScorer->next() ) {
			hc->collect( _internal->countingSumScorer->doc(), score() );
		}
	}
}

int32_t BooleanScorer2::doc() const
{
	return _internal->countingSumScorer->doc();
}

bool BooleanScorer2::next()
{
	if ( _internal->countingSumScorer == NULL ) {
		_internal->initCountingSumScorer();
	}
	return _internal->countingSumScorer->next();
}

float_t BooleanScorer2::score()
{
	_internal->coordinator->initDoc();
	float_t sum = _internal->countingSumScorer->score();
	return sum * _internal->coordinator->coordFactor();
}

bool BooleanScorer2::skipTo( int32_t target )
{
	if ( _internal->countingSumScorer == NULL ) {
		_internal->initCountingSumScorer();
	}
	return _internal->countingSumScorer->skipTo( target );
}

TCHAR* BooleanScorer2::toString()
{
	return stringDuplicate(_T("BooleanScorer2"));
}

Explanation* BooleanScorer2::explain( int32_t /*doc*/ )
{
	_CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: BooleanScorer2::explain");
	/* How to explain the coordination factor?
	initCountingSumScorer();
	return countingSumScorer.explain(doc); // misses coord factor.
	*/
}

bool BooleanScorer2::score( HitCollector* hc, int32_t max )
{
	int32_t docNr = _internal->countingSumScorer->doc();
	while ( docNr < max ) {
		hc->collect( docNr, score() );
		if ( !_internal->countingSumScorer->next() ) {
			return false;
		}
		docNr = _internal->countingSumScorer->doc();
	}
	return true;
}
CL_NS_END
