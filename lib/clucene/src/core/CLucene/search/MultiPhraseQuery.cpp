/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "MultiPhraseQuery.h"
#include "SearchHeader.h"

#include "BooleanClause.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Explanation.h"
#include "Similarity.h"

#include "CLucene/index/_Term.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/MultipleTermPositions.h"

#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/util/_Arrays.h"

#include "_ExactPhraseScorer.h"
#include "_SloppyPhraseScorer.h"

CL_NS_USE(index)
CL_NS_USE(util)

CL_NS_DEF(search)

class MultiPhraseWeight : public Weight {
private:
	Similarity* similarity;
    float_t value;
    float_t idf;
    float_t queryNorm;
    float_t queryWeight;

	MultiPhraseQuery* parentQuery;

public:
	MultiPhraseWeight(Searcher* searcher, MultiPhraseQuery* _parentQuery) : similarity(_parentQuery->getSimilarity(searcher)),
		value(0), idf(0), queryNorm(0), queryWeight(0), parentQuery(_parentQuery) {

		// compute idf
		for (size_t i = 0; i < parentQuery->termArrays->size(); i++){
			ArrayBase<Term*>* terms = parentQuery->termArrays->at(i);
      for ( size_t j=0;j<terms->length;j++ ){
        idf += parentQuery->getSimilarity(searcher)->idf(terms->values[j], searcher);
			}
		}
	}
	virtual ~MultiPhraseWeight(){};

    Query* getQuery() { return parentQuery; }
    float_t getValue() { return value; }

    float_t sumOfSquaredWeights() {
      queryWeight = idf * parentQuery->getBoost();             // compute query weight
      return queryWeight * queryWeight;           // square it
    }

    void normalize(float_t _queryNorm) {
      this->queryNorm = _queryNorm;
      queryWeight *= _queryNorm;                   // normalize query weight
      value = queryWeight * idf;                  // idf for document
    }

	Scorer* scorer(IndexReader* reader) {
		const size_t termArraysSize = parentQuery->termArrays->size();
		if (termArraysSize == 0)                  // optimize zero-term case
			return NULL;

		TermPositions** tps = _CL_NEWARRAY(TermPositions*,termArraysSize+1);
		for (size_t i=0; i<termArraysSize; i++) {
			ArrayBase<Term*>* terms = parentQuery->termArrays->at(i);

			TermPositions* p;
			if (terms->length > 1 )
        p = _CLNEW MultipleTermPositions(reader, terms);
			else
				p = reader->termPositions((*terms)[0]);

			if (p == NULL)
				return NULL;

			tps[i] = p;
		}
		tps[termArraysSize] = NULL;

		Scorer* ret = NULL;

		ValueArray<int32_t> positions;
		parentQuery->getPositions(positions);
		const int32_t slop = parentQuery->getSlop();
		if (slop == 0)
			ret = _CLNEW ExactPhraseScorer(this, tps, positions.values, similarity,
																reader->norms(parentQuery->field));
		else
			ret = _CLNEW SloppyPhraseScorer(this, tps, positions.values, similarity,
															slop, reader->norms(parentQuery->field));

		positions.deleteArray();

		//tps can be deleted safely. SloppyPhraseScorer or ExactPhraseScorer will take care
		//of its values
		_CLDELETE_LARRAY(tps);

		return ret;
	}

	Explanation* explain(IndexReader* reader, int32_t doc){
		ComplexExplanation* result = _CLNEW ComplexExplanation();

		StringBuffer buf(100);
		buf.append(_T("weight("));
		TCHAR* queryString = getQuery()->toString();
		buf.append(queryString);
		buf.append(_T(" in "));
		buf.appendInt(doc);
		buf.append(_T("), product of:"));
		result->setDescription(buf.getBuffer());
		buf.clear();

		buf.append(_T("idf("));
		buf.append(queryString);
		buf.appendChar(_T(')'));
		Explanation* idfExpl = _CLNEW Explanation(idf, buf.getBuffer());
		buf.clear();

		// explain query weight
		Explanation* queryExpl = _CLNEW Explanation();
		buf.append(_T("queryWeight("));
		buf.append(queryString);
		buf.append(_T("), product of:"));
		queryExpl->setDescription(buf.getBuffer());
		buf.clear();

		Explanation* boostExpl = _CLNEW Explanation(parentQuery->getBoost(), _T("boost"));
		if (parentQuery->getBoost() != 1.0f)
			queryExpl->addDetail(boostExpl);

		queryExpl->addDetail(idfExpl);

		Explanation* queryNormExpl = _CLNEW Explanation(queryNorm,_T("queryNorm"));
		queryExpl->addDetail(queryNormExpl);

		queryExpl->setValue(boostExpl->getValue() *
			idfExpl->getValue() *
			queryNormExpl->getValue());

		result->addDetail(queryExpl);

		// explain field weight
		ComplexExplanation* fieldExpl = _CLNEW ComplexExplanation();
		buf.append(_T("fieldWeight("));
		buf.append(queryString);
		buf.append(_T(" in "));
		buf.appendInt(doc);
		buf.append(_T("), product of:"));
		fieldExpl->setDescription(buf.getBuffer());
		buf.clear();
		_CLDELETE_LCARRAY(queryString);

		Explanation* tfExpl = scorer(reader)->explain(doc);
		fieldExpl->addDetail(tfExpl);
		fieldExpl->addDetail(idfExpl);

		Explanation* fieldNormExpl = _CLNEW Explanation();
		uint8_t* fieldNorms = reader->norms(parentQuery->field);
		float_t fieldNorm =
			fieldNorms!=NULL ? Similarity::decodeNorm(fieldNorms[doc]) : 0.0f;
		fieldNormExpl->setValue(fieldNorm);

		buf.append(_T("fieldNorm(field="));
		buf.append(parentQuery->field);
		buf.append(_T(", doc="));
		buf.appendInt(doc);
		buf.appendChar(_T(')'));
		fieldNormExpl->setDescription(buf.getBuffer());
		buf.clear();

		fieldExpl->addDetail(fieldNormExpl);

		fieldExpl->setMatch(tfExpl->isMatch());
		fieldExpl->setValue(tfExpl->getValue() *
			idfExpl->getValue() *
			fieldNormExpl->getValue());

		if (queryExpl->getValue() == 1.0f){
			_CLLDELETE(result);
			return fieldExpl;
		}

		result->addDetail(fieldExpl);
		result->setMatch(fieldExpl->getMatch());

		// combine them
		result->setValue(queryExpl->getValue() * fieldExpl->getValue());

		return result;
	}
};

Query* MultiPhraseQuery::rewrite(IndexReader* /*reader*/) {
  if (termArrays->size() == 1) {                 // optimize one-term case
	  ArrayBase<Term*>* terms = termArrays->at(0);
	  BooleanQuery* boq = _CLNEW BooleanQuery(true);
    for ( size_t i=0;i<terms->length;i++ ){
		  boq->add(_CLNEW TermQuery((*terms)[i]), BooleanClause::SHOULD);
	  }
	  boq->setBoost(getBoost());
	  return boq;
  } else {
	  return this;
  }
}

void MultiPhraseQuery::extractTerms( TermSet * termset ) const
{
    for( size_t i = 0; i < termArrays->size(); i++ )
    {
        ArrayBase<Term*> * terms = termArrays->at( i );
	    for( size_t j=0; j < terms->length; j++ )
        {
            Term * pTerm = terms->values[ j ];
            if( pTerm && termset->end() == termset->find( pTerm ))
                termset->insert( _CL_POINTER( pTerm ));
	    }
    }
}

MultiPhraseQuery::MultiPhraseQuery():
  field(NULL),
  termArrays(_CLNEW CL_NS(util)::CLArrayList<CL_NS(util)::ArrayBase<CL_NS(index)::Term*>*>),
  positions(_CLNEW CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>),
  slop(0)
{
}

MultiPhraseQuery::MultiPhraseQuery( const MultiPhraseQuery& clone ):
    Query(clone)
{
    this->field = clone.field ? STRDUP_TtoT( clone.field ) : NULL;
    this->slop  = clone.slop;

    this->termArrays = _CLNEW CL_NS(util)::CLArrayList<CL_NS(util)::ArrayBase<CL_NS(index)::Term*>*>();
    this->positions  = _CLNEW CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>();

    size_t size = clone.positions->size();
    for( size_t i = 0; i < size; i++ )
    {
        int32_t n = (*clone.positions)[i];
        this->positions->push_back( n );
    }

    size = clone.termArrays->size();
    for( size_t j = 0; j < size; j++ )
    {
        CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* termsToClone = (*clone.termArrays)[ j ];
        CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* terms = _CLNEW CL_NS(util)::ValueArray<CL_NS(index)::Term*>( termsToClone->length );
        for( size_t t = 0; t < termsToClone->length; t++ )
            terms->values[ t ] = _CL_POINTER( termsToClone->values[ t ] );

        this->termArrays->push_back( terms );
    }
}

MultiPhraseQuery::~MultiPhraseQuery(){
	for (size_t i = 0; i < termArrays->size(); i++){
		for ( size_t j=0;j<termArrays->at(i)->length;j++ ) {
			_CLLDECDELETE(termArrays->at(i)->values[j]);
		}
		_CLLDELETE(termArrays->at(i));
	}
	_CLLDELETE(termArrays);
	_CLLDELETE(positions);
	_CLDELETE_LCARRAY(field);
}

Query * MultiPhraseQuery::clone() const
{
    return _CLNEW MultiPhraseQuery( *this );
}

void MultiPhraseQuery::setSlop(const int32_t s) { slop = s; }

int32_t MultiPhraseQuery::getSlop() const { return slop; }

void MultiPhraseQuery::add(CL_NS(index)::Term* term) {
	ValueArray<CL_NS(index)::Term*> _terms(1);
  _terms[0] = term;
	add(&_terms);
}

void MultiPhraseQuery::add(const CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* terms) {
	int32_t position = 0;
	if (positions->size() > 0)
		position = (*positions)[positions->size()-1] + 1;

	add(terms, position);
}

void MultiPhraseQuery::add(const CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* _terms, const int32_t position) {
	if (termArrays->size() == 0)
		field = STRDUP_TtoT((*_terms)[0]->field());

  CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* terms = _CLNEW CL_NS(util)::ValueArray<CL_NS(index)::Term*>(_terms->length);
  for ( size_t i=0;i<_terms->length;i++ ){
		if ( _tcscmp(_terms->values[i]->field(), field) != 0) {
			TCHAR buf[250];
			_sntprintf(buf,250,_T("All phrase terms must be in the same field (%s): %s"),field, (*terms)[i]->field());
			_CLTHROWT(CL_ERR_IllegalArgument,buf);
		}
    terms->values[i] = _CL_POINTER(_terms->values[i]);
	}
	termArrays->push_back(terms);
	positions->push_back(position);
}
const CL_NS(util)::CLArrayList<CL_NS(util)::ArrayBase<CL_NS(index)::Term*>*>* MultiPhraseQuery::getTermArrays() {
  return termArrays;
}

void MultiPhraseQuery::getPositions(ValueArray<int32_t>& result) const {
	result.length = positions->size();
	result.values = _CL_NEWARRAY(int32_t,result.length);
	for (size_t i = 0; i < result.length; i++)
		result.values[i] = (*positions)[i];
}

Weight* MultiPhraseQuery::_createWeight(Searcher* searcher) {
	return _CLNEW MultiPhraseWeight(searcher, this);
}

TCHAR* MultiPhraseQuery::toString(const TCHAR* f) const {
	StringBuffer buffer(100);
	if (_tcscmp(f, field)!=0) {
		buffer.append(field);
		buffer.appendChar(_T(':'));
	}

	buffer.appendChar(_T('"'));

  CL_NS(util)::CLArrayList<CL_NS(util)::ArrayBase<CL_NS(index)::Term*>*>::iterator i;
  i = termArrays->begin();
  while (i != termArrays->end()){
		CL_NS(util)::ArrayBase<CL_NS(index)::Term*>& terms = *(*i);
		if (terms.length > 1) {
			buffer.appendChar(_T('('));
			for (size_t j = 0; j < terms.length; j++) {
				buffer.append(terms[j]->text());
				if (j < terms.length-1)
					buffer.appendChar(_T(' '));
			}
			buffer.appendChar(_T(')'));
		} else {
			buffer.append(terms[0]->text());
		}
		if (i+1 != termArrays->end() )
			buffer.appendChar(_T(' '));

    i++;
	}
	buffer.appendChar(_T('"'));

	if (slop != 0) {
		buffer.appendChar(_T('~'));
		buffer.appendInt(slop);
	}

	buffer.appendBoost(getBoost());

	return buffer.giveBuffer();
}

class TermArray_Equals:public CL_NS_STD(binary_function)<const Term**,const Term**,bool>
{
public:
	bool operator()( CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* val1, CL_NS(util)::ArrayBase<CL_NS(index)::Term*>* val2 ) const{
    if ( val1->length != val2->length )
      return false;
    for ( size_t i=0;i<val1->length;i++ ){
      if (!val1->values[i]->equals(val2->values[i])) return false;
		}
		return true;
	}
};

bool MultiPhraseQuery::equals(Query* o) const {
	if (!(o->instanceOf(MultiPhraseQuery::getObjectName()))) return false;
	MultiPhraseQuery* other = static_cast<MultiPhraseQuery*>(o);
	bool ret = (this->getBoost() == other->getBoost()) && (this->slop == other->slop);

	if (ret){
		CLListEquals<int32_t,Equals::Int32,
			const CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>,
			const CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32> > comp;
		ret = comp.equals(this->positions,other->positions);
	}

	if (ret){
		if (this->termArrays->size() != other->termArrays->size())
			return false;

		for (size_t i=0; i<this->termArrays->size();i++){
			CLListEquals<Term*,TermArray_Equals,
				const CL_NS(util)::CLVector<CL_NS(util)::ArrayBase<CL_NS(index)::Term*>*>,
				const CL_NS(util)::CLVector<CL_NS(util)::ArrayBase<CL_NS(index)::Term*>*> > comp;
			ret = comp.equals(this->termArrays,other->termArrays);
		}
	}
	return ret;
}

// TODO: Test hashed value if conforms with JL
size_t MultiPhraseQuery::hashCode() const {
	size_t ret = Similarity::floatToByte(getBoost()) ^ slop;

	{ //msvc6 scope fix
        for( size_t i = 0; i < termArrays->size(); i++ )
        {
		    for( size_t j = 0; j < termArrays->at( i )->length; j++ )
            {
                ret = 31 * ret + termArrays->at(i)->values[j]->hashCode();
            }
		}
	}
	{ //msvc6 scope fix
		for ( size_t i=0;i<positions->size();i++ )
			ret = 31 * ret + (*positions)[i];
	}
	ret ^= 0x4AC65113;

	return ret;
}

CL_NS_END
