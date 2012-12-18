/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "PhraseQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Similarity.h"
#include "Searchable.h"
#include "Explanation.h"

#include "CLucene/index/_Term.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"

#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/VoidList.h"
#include "CLucene/util/_Arrays.h"

#include "_ExactPhraseScorer.h"
#include "_SloppyPhraseScorer.h"

#include <assert.h>

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)



	class PhraseWeight: public Weight {
	private:
		Searcher* searcher;
		float_t value;
		float_t idf;
		float_t queryNorm;
		float_t queryWeight;

		PhraseQuery* parentQuery;
	public:
		PhraseWeight(Searcher* searcher, PhraseQuery* parentQuery);
		virtual ~PhraseWeight();
		TCHAR* toString();

		Query* getQuery();
		float_t getValue();

		float_t sumOfSquaredWeights();
		void normalize(float_t queryNorm);
		Scorer* scorer(CL_NS(index)::IndexReader* reader);
		Explanation* explain(CL_NS(index)::IndexReader* reader, int32_t doc);
		TCHAR* toString(TCHAR* f);
		bool equals(PhraseWeight* o);
	};

  PhraseQuery::PhraseQuery():
	field(NULL), terms(_CLNEW CL_NS(util)::CLVector<CL_NS(index)::Term*>(false) ),
		positions(_CLNEW CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>), slop(0)
  {
  }

  PhraseQuery::PhraseQuery(const PhraseQuery& clone):
	Query(clone),
		terms(_CLNEW CL_NS(util)::CLVector<CL_NS(index)::Term*>(false) ),
		positions(_CLNEW CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>)
  {
      slop  = clone.slop;
	  field = clone.field;
	  int32_t size=clone.positions->size();
	  { //msvc6 scope fix
		  for ( int32_t i=0;i<size;i++ ){
			  int32_t n = (*clone.positions)[i];
			  this->positions->push_back( n );
		  }
	  }
	  size=clone.terms->size();
	  { //msvc6 scope fix
		  for ( int32_t i=0;i<size;i++ ){
			  this->terms->push_back( _CL_POINTER((*clone.terms)[i]));
		  }
	  }
  }
  Query* PhraseQuery::clone() const{
	  return _CLNEW PhraseQuery(*this);
  }

  const TCHAR* PhraseQuery::getFieldName() const{ return field; }

  void PhraseQuery::setSlop(const int32_t s) { slop = s; }
  int32_t PhraseQuery::getSlop() const { return slop; }

  bool PhraseQuery::equals(CL_NS(search)::Query *other) const{
	  if (!(other->instanceOf(PhraseQuery::getClassName())))
            return false;

	  PhraseQuery* pq = (PhraseQuery*)other;
	  bool ret = (this->getBoost() == pq->getBoost()) && (this->slop == pq->slop);

	  if ( ret ){
		  CLListEquals<CL_NS(index)::Term,CL_NS(index)::Term_Equals,
			  const CL_NS(util)::CLVector<CL_NS(index)::Term*>,
			  const CL_NS(util)::CLVector<CL_NS(index)::Term*> > comp;
		  ret = comp.equals(this->terms,pq->terms);
	  }

	  if ( ret ){
		  CLListEquals<int32_t,Equals::Int32,
			  const CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>,
			  const CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32> > comp;
		  ret = comp.equals(this->positions,pq->positions);
	  }
	  return ret;
  }


  PhraseQuery::~PhraseQuery(){
  //Func - Destructor
  //Pre  - true
  //Post 0 The instance has been destroyed

	  //Iterate through all the terms
	  for (size_t i = 0; i < terms->size(); i++){
        _CLLDECDELETE((*terms)[i]);
      }
	  _CLLDELETE(terms);
	  _CLLDELETE(positions);
  }

  size_t PhraseQuery::hashCode() const {
		//todo: do cachedHashCode, and invalidate on add/remove clause
		size_t ret = Similarity::floatToByte(getBoost()) ^ Similarity::floatToByte(slop);

		{ //msvc6 scope fix
			for ( size_t i=0;i<terms->size();i++ )
				ret = 31 * ret + (*terms)[i]->hashCode();
		}
		{ //msvc6 scope fix
			for ( size_t i=0;i<positions->size();i++ )
				ret = 31 * ret + (*positions)[i];
		}
		return ret;
	}

  const char* PhraseQuery::getClassName(){
    return "PhraseQuery";
  }
  const char* PhraseQuery::getObjectName() const{
  //Func - Returns the string "PhraseQuery"
  //Pre  - true
  //Post - The string "PhraseQuery" has been returned
    return getClassName();
  }

  void PhraseQuery::add(Term* term) {
	  CND_PRECONDITION(term != NULL,"term is NULL");

	  int32_t position = 0;

	  if(positions->size() > 0)
		  position = ((*positions)[positions->size()-1]) + 1;

	  add(term, position);
  }

  void PhraseQuery::add(Term* term, int32_t position) {

	  CND_PRECONDITION(term != NULL,"term is NULL");

	  if (terms->size() == 0)
		  field = term->field();
	  else{
		  //Check if the field of the _CLNEW term matches the field of the PhraseQuery
		  //can use != because fields are interned
		  if ( term->field() != field){
			  TCHAR buf[200];
			  _sntprintf(buf,200,_T("All phrase terms must be in the same field: %s"),term->field());
			  _CLTHROWT(CL_ERR_IllegalArgument,buf);
		  }
	  }

	  //Store the _CLNEW term
	  terms->push_back(_CL_POINTER(term));
	  positions->push_back(position);
  }

	void PhraseQuery::getPositions(ValueArray<int32_t>& result) const{
		result.length = positions->size();
		result.values = _CL_NEWARRAY(int32_t,result.length);
		for(size_t i = 0; i < result.length; i++){
			result.values[i] = (*positions)[i];
		}
	}

	Weight* PhraseQuery::_createWeight(Searcher* searcher) {
		if (terms->size() == 1) {			  // optimize one-term case
			Term* term = (*terms)[0];
			Query* termQuery = _CLNEW TermQuery(term);
			termQuery->setBoost(getBoost());
			Weight* ret = termQuery->_createWeight(searcher);
			_CLLDELETE(termQuery);
			return ret;
		}
		return _CLNEW PhraseWeight(searcher,this);
	}


  Term** PhraseQuery::getTerms() const{
  //Func - added by search highlighter

	  //Let size contain the number of terms
      int32_t size = terms->size();
      Term** ret = _CL_NEWARRAY(Term*,size+1);

	  CND_CONDITION(ret != NULL,"Could not allocated memory for ret");

	  //Iterate through terms and copy each pointer to ret
	  for ( int32_t i=0;i<size;i++ ){
          ret[i] = (*terms)[i];
     }
     ret[size] = NULL;
     return ret;
  }

  TCHAR* PhraseQuery::toString(const TCHAR* f) const{
	  //Func - Prints a user-readable version of this query.
	  //Pre  - f != NULL
	  //Post - The query string has been returned

	  if ( terms->size()== 0 )
		  return NULL;

	  StringBuffer buffer(32);
	  if ( f==NULL || _tcscmp(field,f)!=0) {
		  buffer.append(field);
		  buffer.appendChar(_T(':'));
	  }

	  buffer.appendChar( _T('"') );

	  Term *T = NULL;

	  //iterate through all terms
	  for (size_t i = 0; i < terms->size(); i++) {
		  //Get the i-th term
		  T = (*terms)[i];

		  buffer.append( T->text() );
		  //Check if i is at the end of terms
		  if (i != terms->size()-1){
			  buffer.appendChar(_T(' '));
		  }
	  }
	  buffer.appendChar( _T('"') );

	  if (slop != 0) {
		  buffer.appendChar(_T('~'));
		  buffer.appendFloat(slop, 0);
	  }

	  buffer.appendBoost(getBoost());

	  return buffer.giveBuffer();
  }

void PhraseQuery::extractTerms( TermSet * termset ) const
{
    for( size_t i = 0; i < terms->size(); i++ )
    {
        Term * pTerm = (*terms)[i];
        if( pTerm && termset->end() == termset->find( pTerm ))
            termset->insert( _CL_POINTER( pTerm ));
    }
}


 PhraseWeight::PhraseWeight(Searcher* searcher, PhraseQuery* _parentQuery) {
   this->parentQuery=_parentQuery;
   this->value = 0;
   this->idf = 0;
   this->queryNorm = 0;
   this->queryWeight = 0;
   this->searcher = searcher;
 }

 TCHAR* PhraseWeight::toString() {
	return STRDUP_TtoT(_T("weight(PhraseQuery)"));
 }
 PhraseWeight::~PhraseWeight(){
 }


 Query* PhraseWeight::getQuery() { return parentQuery; }
 float_t PhraseWeight::getValue() { return value; }

 float_t PhraseWeight::sumOfSquaredWeights(){
   idf = parentQuery->getSimilarity(searcher)->idf(parentQuery->terms, searcher);
   queryWeight = idf * parentQuery->getBoost();    // compute query weight
   return queryWeight * queryWeight;         // square it
 }

 void PhraseWeight::normalize(float_t queryNorm) {
   this->queryNorm = queryNorm;
   queryWeight *= queryNorm;                   // normalize query weight
   value = queryWeight * idf;                  // idf for document
 }

  Scorer* PhraseWeight::scorer(IndexReader* reader)  {
  //Func -
  //Pre  -
  //Post -

	  //Get the length of terms
      const int32_t tpsLength = (const int32_t)parentQuery->terms->size();

	  //optimize zero-term case
      if (tpsLength == 0)
          return NULL;

    TermPositions** tps = _CL_NEWARRAY(TermPositions*,tpsLength+1);

	//Check if tps has been allocated properly
    CND_CONDITION(tps != NULL,"Could not allocate memory for tps");

    TermPositions* p = NULL;

	//Iterate through all terms
    for (int32_t i = 0; i < tpsLength; i++) {
        //Get the termPostitions for the i-th term
        p = reader->termPositions((*parentQuery->terms)[i]);

		//Check if p is valid
		if (p == NULL) {
			//Delete previous retrieved termPositions
			while (--i >= 0){
				_CLVDELETE(tps[i]);  //todo: not a clucene object... should be
			}
            _CLDELETE_ARRAY(tps);
            return NULL;
        }

        //Store p at i in tps
        tps[i] = p;
    }
	tps[tpsLength] = NULL;

    Scorer* ret = NULL;

    ValueArray<int32_t> positions;
	parentQuery->getPositions(positions);
	int32_t slop = parentQuery->getSlop();
	if ( slop != 0)
		 // optimize exact case
		 //todo: need to pass these: this, tps,
         ret = _CLNEW SloppyPhraseScorer(this,tps,positions.values,
								parentQuery->getSimilarity(searcher),
								slop, reader->norms(parentQuery->field));
	else
	    ret = _CLNEW ExactPhraseScorer(this, tps, positions.values,
									parentQuery->getSimilarity(searcher),
                                    reader->norms(parentQuery->field));
	positions.deleteArray();

    CND_CONDITION(ret != NULL,"Could not allocate memory for ret");

	//tps can be deleted safely. SloppyPhraseScorer or ExactPhraseScorer will take care
	//of its values

    _CLDELETE_LARRAY(tps);
    return ret;
  }

  Explanation* PhraseWeight::explain(IndexReader* reader, int32_t doc){
	  Explanation* result = _CLNEW Explanation();
	  TCHAR descbuf[LUCENE_SEARCH_EXPLANATION_DESC_LEN+1];
	  TCHAR* tmp;

	  tmp = getQuery()->toString();
	  _sntprintf(descbuf,LUCENE_SEARCH_EXPLANATION_DESC_LEN,_T("weight(%s in %d), product of:"),
		  tmp,doc);
	  _CLDELETE_LCARRAY(tmp);
	  result->setDescription(descbuf);

	  StringBuffer docFreqs;
	  StringBuffer query;
	  query.appendChar('"');
	  for (size_t i = 0; i < parentQuery->terms->size(); i++) {
		  if (i != 0) {
			  docFreqs.appendChar(' ');
			  query.appendChar(' ');
		  }

		  Term* term = (*parentQuery->terms)[i];

		  docFreqs.append(term->text());
		  docFreqs.appendChar('=');
		  docFreqs.appendInt(searcher->docFreq(term));

		  query.append(term->text());
	  }
	  query.appendChar('\"');

	  _sntprintf(descbuf,LUCENE_SEARCH_EXPLANATION_DESC_LEN,
		  _T("idf(%s: %s)"),parentQuery->field,docFreqs.getBuffer());
	  Explanation* idfExpl = _CLNEW Explanation(idf, descbuf);

	  // explain query weight
	  Explanation* queryExpl = _CLNEW Explanation();
	  tmp = getQuery()->toString();
	  _sntprintf(descbuf,LUCENE_SEARCH_EXPLANATION_DESC_LEN,
		  _T("queryWeight(%s), product of:"),tmp);
	  _CLDELETE_LCARRAY(tmp);
	  queryExpl->setDescription(descbuf);

	  Explanation* boostExpl = _CLNEW Explanation(parentQuery->getBoost(), _T("boost"));
	  bool deleteBoostExpl = false;
	  if (parentQuery->getBoost() != 1.0f)
	    queryExpl->addDetail(boostExpl);
	  else
	    deleteBoostExpl = true;
	  queryExpl->addDetail(idfExpl);

	  Explanation* queryNormExpl = _CLNEW Explanation(queryNorm,_T("queryNorm"));
	  queryExpl->addDetail(queryNormExpl);

	  queryExpl->setValue(boostExpl->getValue() *
		  idfExpl->getValue() *
		  queryNormExpl->getValue());

	  if (deleteBoostExpl)
	    _CLLDELETE(boostExpl);

	  result->addDetail(queryExpl);

	  // explain field weight
	  Explanation* fieldExpl = _CLNEW Explanation();
	  _sntprintf(descbuf,LUCENE_SEARCH_EXPLANATION_DESC_LEN,
		  _T("fieldWeight(%s:%s in %d), product of:"),
		  parentQuery->field,query.getBuffer(),doc);
	  fieldExpl->setDescription(descbuf);


	  Scorer* sc = scorer(reader);
	  Explanation* tfExpl = sc->explain(doc);
	  _CLLDELETE(sc);
	  fieldExpl->addDetail(tfExpl);
	  fieldExpl->addDetail( _CLNEW Explanation(idfExpl->getValue(), idfExpl->getDescription()) );

	  Explanation* fieldNormExpl = _CLNEW Explanation();
	  uint8_t* fieldNorms = reader->norms(parentQuery->field);
	  float_t fieldNorm =
		  fieldNorms!=NULL ? Similarity::decodeNorm(fieldNorms[doc]) : 0.0f;
	  fieldNormExpl->setValue(fieldNorm);


	  _sntprintf(descbuf,LUCENE_SEARCH_EXPLANATION_DESC_LEN,
		  _T("fieldNorm(field=%s, doc=%d)"),parentQuery->field,doc);
	  fieldNormExpl->setDescription(descbuf);
	  fieldExpl->addDetail(fieldNormExpl);

	  fieldExpl->setValue(tfExpl->getValue() *
		  idfExpl->getValue() *
		  fieldNormExpl->getValue());

	  if (queryExpl->getValue() == 1.0f){
		  _CLLDELETE(result);
		  return fieldExpl;
	  }

	  result->addDetail(fieldExpl);

	  // combine them
	  result->setValue(queryExpl->getValue() * fieldExpl->getValue());

	  return result;
  }


CL_NS_END
