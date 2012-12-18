/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "BooleanQuery.h"

#include "BooleanClause.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/_Arrays.h"
#include "SearchHeader.h"
#include "_BooleanScorer.h"
#include "_ConjunctionScorer.h"
#include "Similarity.h"
#include "Explanation.h"
#include "_BooleanScorer2.h"
#include "Scorer.h"

#include <assert.h>

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

	class BooleanClause_Compare:public CL_NS_STD(binary_function)<const BooleanClause*,const BooleanClause*,bool>
	{
	public:
		bool operator()( const BooleanClause* val1, const BooleanClause* val2 ) const {
			return val1->equals(val2);
		}
	};


	class BooleanWeight: public Weight {
	protected:
		Searcher* searcher;
		Similarity* similarity;
		CL_NS(util)::CLVector<Weight*,CL_NS(util)::Deletor::Object<Weight> > weights;
		BooleanQuery::ClausesType* clauses;
		BooleanQuery* parentQuery;
	public:
		BooleanWeight(Searcher* searcher,
			CL_NS(util)::CLVector<BooleanClause*,CL_NS(util)::Deletor::Object<BooleanClause> >* clauses,
			BooleanQuery* parentQuery);
		virtual ~BooleanWeight();
		Query* getQuery();
		float_t getValue();
		float_t sumOfSquaredWeights();
		void normalize(float_t norm);
		Scorer* scorer(CL_NS(index)::IndexReader* reader);
		Explanation* explain(CL_NS(index)::IndexReader* reader, int32_t doc);
	};// BooleanWeight


	BooleanQuery::BooleanQuery( bool disableCoord ):
		clauses(_CLNEW BooleanQuery::ClausesType(true))
	{
		this->minNrShouldMatch = 0;
		this->disableCoord = disableCoord;
	}

  Weight* BooleanQuery::_createWeight(Searcher* searcher) {
		return _CLNEW BooleanWeight(searcher, clauses,this);
	}

	BooleanQuery::BooleanQuery(const BooleanQuery& clone):
		Query(clone),
		clauses(_CLNEW ClausesType(true)),
		disableCoord(clone.disableCoord)
	{
		minNrShouldMatch = clone.minNrShouldMatch;
		for ( uint32_t i=0;i<clone.clauses->size();i++ ){
			BooleanClause* clause = (*clone.clauses)[i]->clone();
			clause->deleteQuery=true;
			add(clause);
		}
	}

  BooleanQuery::~BooleanQuery(){
		clauses->clear();
		_CLDELETE(clauses);
  }

	size_t BooleanQuery::hashCode() const {
		//todo: do cachedHashCode, and invalidate on add/remove clause
		size_t ret = 0;
		for (uint32_t i = 0 ; i < clauses->size(); i++) {
			BooleanClause* c = (*clauses)[i];
			ret = 31 * ret + c->hashCode();
		}
		ret = ret ^ Similarity::floatToByte(getBoost());
		return ret;
	}

  const char* BooleanQuery::getObjectName() const{
    return getClassName();
  }
	const char* BooleanQuery::getClassName(){
    return "BooleanQuery";
  }

   /**
   * Default value is 1024.  Use <code>org.apache.lucene.maxClauseCount</code>
   * system property to override.
   */
   size_t BooleanQuery::maxClauseCount = LUCENE_BOOLEANQUERY_MAXCLAUSECOUNT;
   size_t BooleanQuery::getMaxClauseCount(){
      return maxClauseCount;
   }

   void BooleanQuery::setMaxClauseCount(const size_t maxClauseCount){
       if (maxClauseCount < 1)
           _CLTHROWA(CL_ERR_IllegalArgument, "maxClauseCount must be >= 1");
	   BooleanQuery::maxClauseCount = maxClauseCount;
   }

   Similarity* BooleanQuery::getSimilarity( Searcher* searcher ) {

	   Similarity* result = Query::getSimilarity( searcher );
	   return result;

   }

  void BooleanQuery::add(Query* query, const bool deleteQuery, const bool required, const bool prohibited) {
		BooleanClause* bc = _CLNEW BooleanClause(query,deleteQuery,required, prohibited);
		try{
			add(bc);
		}catch(...){
			_CLDELETE(bc);
			throw;
		}
  }

  void BooleanQuery::add(Query* query, const bool deleteQuery, BooleanClause::Occur occur) {
		BooleanClause* bc = _CLNEW BooleanClause(query,deleteQuery,occur);
		try{
			add(bc);
		}catch(...){
			_CLDELETE(bc);
			throw;
		}
  }

  void BooleanQuery::add(BooleanClause* clause) {
    if (clauses->size() >= getMaxClauseCount())
      _CLTHROWA(CL_ERR_TooManyClauses,"Too Many Clauses");

    clauses->push_back(clause);
  }

  int32_t BooleanQuery::getMinNrShouldMatch(){
  	return minNrShouldMatch;
  }
  bool BooleanQuery::getUseScorer14() {
	  return getAllowDocsOutOfOrder();
  }

  bool BooleanQuery::allowDocsOutOfOrder = false;

  void BooleanQuery::setUseScorer14( bool use14 ) {
	  setAllowDocsOutOfOrder(use14);
  }

  void BooleanQuery::setAllowDocsOutOfOrder(bool allow) {
    allowDocsOutOfOrder = allow;
  }

  bool BooleanQuery::getAllowDocsOutOfOrder() {
    return allowDocsOutOfOrder;
  }


  size_t BooleanQuery::getClauseCount() const {
    return (int32_t) clauses->size();
  }

  TCHAR* BooleanQuery::toString(const TCHAR* field) const{
    StringBuffer buffer;
	bool needParens=(getBoost() != 1.0) /* TODO: || (getMinimumNumberShouldMatch()>0)*/ ;
	if (needParens) {
		buffer.append(_T("("));
	}

    for (uint32_t i = 0 ; i < clauses->size(); i++) {
      BooleanClause* c = (*clauses)[i];
      if (c->prohibited)
        buffer.append(_T("-"));
      else if (c->required)
        buffer.append(_T("+"));

      if ( c->getQuery()->instanceOf(BooleanQuery::getClassName()) ) {	  // wrap sub-bools in parens
        buffer.append(_T("("));

        TCHAR* buf = c->getQuery()->toString(field);
        buffer.append(buf);
        _CLDELETE_CARRAY( buf );

        buffer.append(_T(")"));
      } else {
        TCHAR* buf = c->getQuery()->toString(field);
        buffer.append(buf);
        _CLDELETE_CARRAY( buf );
      }
      if (i != clauses->size()-1)
        buffer.append(_T(" "));
	}

	if (needParens) {
		buffer.append(_T(")"));
	}

	if (getBoost() != 1.0) {
		buffer.appendChar(_T('^'));
		buffer.appendFloat(getBoost(),1);
	}
    return buffer.toString();
  }

		bool BooleanQuery::isCoordDisabled() { return disableCoord; }
		void BooleanQuery::setCoordDisabled( bool disableCoord ) { this->disableCoord = disableCoord; }

    BooleanClause** BooleanQuery::getClauses() const
	{
		CND_MESSAGE(false, "Warning: BooleanQuery::getClauses() is deprecated")
		BooleanClause** ret = _CL_NEWARRAY(BooleanClause*, clauses->size()+1);
		getClauses(ret);
		return ret;
	}

	void BooleanQuery::getClauses(BooleanClause** ret) const
	{
		size_t size=clauses->size();
		for ( uint32_t i=0;i<size;i++ )
			ret[i] = (*clauses)[i];
	}
  Query* BooleanQuery::rewrite(IndexReader* reader) {
    if (clauses->size() == 1) {                    // optimize 1-clause queries
      BooleanClause* c = (*clauses)[0];
      if (!c->prohibited) {			  // just return clause
	      Query* query = c->getQuery()->rewrite(reader);    // rewrite first

	      //if the query doesn't actually get re-written,
	      //then return a clone (because the BooleanQuery
	      //will register different to the returned query.
	      if ( query == c->getQuery() )
		      query = query->clone();

        if (getBoost() != 1.0f) {                 // incorporate boost
	        query->setBoost(getBoost() * query->getBoost());
        }

        return query;
      }
    }

    BooleanQuery* clone = NULL;                    // recursively rewrite
    for (uint32_t i = 0 ; i < clauses->size(); i++) {
      BooleanClause* c = (*clauses)[i];
      Query* query = c->getQuery()->rewrite(reader);
      if (query != c->getQuery()) {                     // clause rewrote: must clone
         if (clone == NULL)
            clone = (BooleanQuery*)this->clone();
         clone->clauses->set (i, _CLNEW BooleanClause(query, true, c->getOccur()));
      }
   }
   if (clone != NULL) {
      return clone;                               // some clauses rewrote
   } else
      return this;                                // no clauses rewrote
  }

    void BooleanQuery::extractTerms( TermSet * termset ) const
    {
		for (size_t i = 0 ; i < clauses->size(); i++)
        {
            BooleanClause* clause = (*clauses)[i];
            clause->getQuery()->extractTerms( termset );
        }
    }

  Query* BooleanQuery::clone()  const{
    BooleanQuery* clone = _CLNEW BooleanQuery(*this);
    return clone;
  }

  /** Returns true iff <code>o</code> is equal to this. */
  bool BooleanQuery::equals(Query* o)const {
     if (!(o->instanceOf(BooleanQuery::getClassName())))
        return false;
     const BooleanQuery* other = (BooleanQuery*)o;

     bool ret = (this->getBoost() == other->getBoost());
     if ( ret ){
	     CLListEquals<BooleanClause,BooleanClause_Compare, const BooleanQuery::ClausesType, const BooleanQuery::ClausesType> comp;
	     ret = comp.equals(this->clauses,other->clauses);
     }
    return ret;
  }


	float_t BooleanWeight::getValue() { return parentQuery->getBoost(); }
	Query* BooleanWeight::getQuery() { return (Query*)parentQuery; }

	BooleanWeight::BooleanWeight(Searcher* searcher,
		CLVector<BooleanClause*,Deletor::Object<BooleanClause> >* clauses, BooleanQuery* parentQuery)
	{
		this->searcher = searcher;
		this->similarity = parentQuery->getSimilarity( searcher );
		this->parentQuery = parentQuery;
		this->clauses = clauses;
		for (uint32_t i = 0 ; i < clauses->size(); i++) {
			weights.push_back((*clauses)[i]->getQuery()->_createWeight(searcher));
		}
	}
	BooleanWeight::~BooleanWeight(){
		this->weights.clear();
	}

    float_t BooleanWeight::sumOfSquaredWeights() {
      float_t sum = 0.0f;
      for (uint32_t i = 0 ; i < weights.size(); i++) {
        BooleanClause* c = (*clauses)[i];
        Weight* w = weights[i];
        float_t s = w->sumOfSquaredWeights();         // sum sub weights
        if (!c->isProhibited())
          // only add to sum for non-prohibited clauses
          sum += s;
      }
      sum *= parentQuery->getBoost() * parentQuery->getBoost();             // boost each sub-weight
      return sum ;
    }

    void BooleanWeight::normalize(float_t norm) {
      norm *= parentQuery->getBoost();                         // incorporate boost
      for (uint32_t i = 0 ; i < weights.size(); i++) {
        Weight* w = weights[i];
        // normalize all clauses, (even if prohibited in case of side affects)
        w->normalize(norm);
      }
    }

    Scorer* BooleanWeight::scorer(IndexReader* reader){
      BooleanScorer2* result = _CLNEW BooleanScorer2(similarity,
                                                 parentQuery->minNrShouldMatch,
                                                 parentQuery->allowDocsOutOfOrder);

      for (size_t i = 0 ; i < weights.size(); i++) {
        BooleanClause* c = (*clauses)[i];
        Weight* w = weights[i];
        Scorer* subScorer = w->scorer(reader);
        if (subScorer != NULL)
          result->add(subScorer, c->isRequired(), c->isProhibited());
        else if (c->isRequired()){
          _CLDELETE(result);
          return NULL;
        }
      }

      return result;

    }

	Explanation* BooleanWeight::explain(IndexReader* reader, int32_t doc){
		const int32_t minShouldMatch = parentQuery->getMinNrShouldMatch();
		ComplexExplanation* sumExpl = _CLNEW ComplexExplanation();
		sumExpl->setDescription(_T("sum of:"));
		int32_t coord = 0;
		int32_t maxCoord = 0;
		float_t sum = 0.0f;
		bool fail = false;
		int32_t shouldMatchCount = 0;
		for (size_t i = 0 ; i < weights.size(); i++) {
			BooleanClause* c = (*clauses)[i];
			Weight* w = weights[i];
			Explanation* e = w->explain(reader, doc);
			if (!c->isProhibited()) maxCoord++;
			if (e->isMatch()){
				if (!c->isProhibited()) {
					sumExpl->addDetail(e);
					sum += e->getValue();
					coord++;
				} else {
					StringBuffer buf(100);
					buf.append(_T("match on prohibited clause ("));
					TCHAR* tmp = c->getQuery()->toString();
					buf.append(tmp);
					_CLDELETE_LCARRAY(tmp);
					buf.appendChar(_T(')'));

					Explanation* r = _CLNEW Explanation(0.0f, buf.getBuffer());
					r->addDetail(e);
					sumExpl->addDetail(r);
					fail = true;
				}
				if (c->getOccur() == BooleanClause::SHOULD)
					shouldMatchCount++;
			} else if (c->isRequired()) {
				StringBuffer buf(100);
				buf.append(_T("no match on required clause ("));
				TCHAR* tmp = c->getQuery()->toString();
				buf.append(tmp);
				_CLDELETE_LCARRAY(tmp);
				buf.appendChar(_T(')'));

				Explanation* r = _CLNEW Explanation(0.0f, buf.getBuffer());
				r->addDetail(e);
				sumExpl->addDetail(r);
				fail = true;
			} else {
				_CLLDELETE(e);
			}
		}
		if (fail) {
			sumExpl->setMatch(false);
			sumExpl->setValue(0.0f);
			sumExpl->setDescription(_T("Failure to meet condition(s) of required/prohibited clause(s)"));
			return sumExpl;
		} else if (shouldMatchCount < minShouldMatch) {
			sumExpl->setMatch(false);
			sumExpl->setValue(0.0f);

			StringBuffer buf(60);
			buf.append(_T("Failure to match minimum number of optional clauses: "));
			buf.appendInt(minShouldMatch);
			sumExpl->setDescription(buf.getBuffer());
			return sumExpl;
		}

		sumExpl->setMatch(0 < coord ? true : false);
		sumExpl->setValue(sum);

		float_t coordFactor = similarity->coord(coord, maxCoord);
		if (coordFactor == 1.0f)                      // coord is no-op
			return sumExpl;                             // eliminate wrapper
		else {
			ComplexExplanation* result = _CLNEW ComplexExplanation(sumExpl->isMatch(),
				sum*coordFactor,
				_T("product of:"));
			result->addDetail(sumExpl);

			StringBuffer buf(30);
			buf.append(_T("coord("));
			buf.appendInt(coord);
			buf.appendChar(_T('/'));
			buf.appendInt(maxCoord);
			buf.appendChar(_T(')'));
			result->addDetail(_CLNEW Explanation(coordFactor,buf.getBuffer()));
			return result;
		}
	}

	BooleanClause::BooleanClause(Query* q, const bool DeleteQuery,const bool req, const bool p):
	    query(q),
		occur(SHOULD),
		deleteQuery(DeleteQuery),
		required(req),
		prohibited(p)
	{
		if (required) {
			if (prohibited) {
				// prohibited && required doesn't make sense, but we want the old behaviour:
				occur = MUST_NOT;
			} else {
				occur = MUST;
			}
		} else {
			if (prohibited) {
				occur = MUST_NOT;
			} else {
				occur = SHOULD;
			}
		}
	}

	BooleanClause::BooleanClause(const BooleanClause& clone):
		query(clone.query->clone()),
		occur(clone.occur),
		deleteQuery(true),
		required(clone.required),
		prohibited(clone.prohibited)
	{
	}

	BooleanClause::BooleanClause(Query* q, const bool DeleteQuery, Occur o):
		query(q),
		occur(o),
		deleteQuery(DeleteQuery)
	{
		setFields(occur);
	}


	BooleanClause* BooleanClause::clone() const {
		BooleanClause* ret = _CLNEW BooleanClause(*this);
		return ret;
	}

	BooleanClause::~BooleanClause(){
		if ( deleteQuery )
			_CLDELETE( query );
	}


	/** Returns true if <code>o</code> is equal to this. */
	bool BooleanClause::equals(const BooleanClause* other) const {
		return this->query->equals(other->query)
			&& (this->required == other->required)
			&& (this->prohibited == other->prohibited) // TODO: Remove these
			&& (this->occur == other->getOccur() );
	}

	/** Returns a hash code value for this object.*/
	size_t BooleanClause::hashCode() const {
		return query->hashCode() ^ ( (occur == MUST) ?1:0) ^ ( (occur == MUST_NOT)?2:0);
	}

	BooleanClause::Occur BooleanClause::getOccur() const { return occur; }
	void BooleanClause::setOccur(Occur o) {
		occur = o;
		setFields(o);
	}

	Query* BooleanClause::getQuery() const { return query; }
	void BooleanClause::setQuery(Query* q) {
		if ( deleteQuery )
			_CLDELETE( query );
		query = q;
	}

	bool BooleanClause::isProhibited() const { return prohibited; /* TODO: return (occur == MUST_NOT); */ }
	bool BooleanClause::isRequired() const { return required; /* TODO: return (occur == MUST); */ }

	TCHAR* BooleanClause::toString() const {
		CL_NS(util)::StringBuffer buffer;
		if (occur == MUST)
			buffer.append(_T("+"));
		else if (occur == MUST_NOT)
			buffer.append(_T("-"));
		buffer.append( query->toString() );
		return buffer.toString();
	}

	void BooleanClause::setFields(Occur occur) {
		if (occur == MUST) {
			required = true;
			prohibited = false;
		} else if (occur == SHOULD) {
			required = false;
			prohibited = false;
		} else if (occur == MUST_NOT) {
			required = false;
			prohibited = true;
		} else {
			_CLTHROWT (CL_ERR_UnknownOperator,  _T("Unknown operator"));
		}
	}

CL_NS_END
