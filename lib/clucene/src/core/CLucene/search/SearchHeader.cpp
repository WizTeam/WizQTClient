/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "SearchHeader.h"
#include "CLucene/document/Document.h"
#include "Similarity.h"
#include "BooleanQuery.h"
#include "Searchable.h"
#include "Hits.h"
#include "_FieldDocSortedHitQueue.h"
#include <assert.h>

CL_NS_USE(index)
CL_NS_DEF(search)

CL_NS(document)::Document* Searchable::doc(const int32_t i){
    CL_NS(document)::Document* ret = _CLNEW CL_NS(document)::Document;
    if (!doc(i,ret) )
        _CLDELETE(ret);
    return ret;
}

//static
Query* Query::mergeBooleanQueries(CL_NS(util)::ArrayBase<Query*>* queries) {
  std::vector<BooleanClause*> allClauses;

  CL_NS(util)::ValueArray<BooleanClause*> clauses;
  for (size_t i = 0; i < queries->length; i++) {
    assert(BooleanQuery::getClassName() == queries->values[i]->getObjectName());
    BooleanQuery* booleanQuery = (BooleanQuery*)queries->values[i];
	  clauses.resize((booleanQuery->getClauseCount()));
    booleanQuery->getClauses(clauses.values);
    for (size_t j = 0; j < clauses.length; j++) {
      allClauses.push_back(clauses.values[j]->clone());
    }
  }

  bool coordDisabled = ( queries->length == 0 ) ? false : ((BooleanQuery*)queries->values[0])->isCoordDisabled();
  BooleanQuery* result = _CLNEW BooleanQuery(coordDisabled);
  std::vector<BooleanClause*>::iterator i = allClauses.begin();
  while ( i != allClauses.end() ){
    result->add(*i);
	i++;
  }
  return result;
}

Query::Query(const Query& clone):boost(clone.boost){
}
Weight* Query::_createWeight(Searcher* /*searcher*/){
	_CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: Query::_createWeight");
}

Query::Query():
   boost(1.0f)
{
}
Query::~Query(){
}

const char* Query::getQueryName() const{ return getObjectName(); }
/** Expert: called to re-write queries into primitive queries. */
Query* Query::rewrite(CL_NS(index)::IndexReader* /*reader*/){
   return this;
}

Query* Query::combine(CL_NS(util)::ArrayBase<Query*>* queries){
  std::vector<Query*> uniques;
  for (size_t i = 0; i < queries->length; i++) {
    Query* query = queries->values[i];
    CL_NS(util)::ValueArray<BooleanClause*> clauses;
    // check if we can split the query into clauses
    bool splittable = query->instanceOf(BooleanQuery::getClassName());
    if(splittable){
      BooleanQuery* bq = (BooleanQuery*) query;
      splittable = bq->isCoordDisabled();
      clauses.resize(bq->getClauseCount());
      bq->getClauses(clauses.values);
      for (size_t j = 0; splittable && j < clauses.length; j++) {
        splittable = (clauses[j]->getOccur() == BooleanClause::SHOULD);
      }
    }
    if(splittable){
      for (size_t j = 0; j < clauses.length; j++) {
        uniques.push_back(clauses[j]->getQuery());
      }
    } else {
      uniques.push_back(query);
    }
  }
  // optimization: if we have just one query, just return it
  if(uniques.size() == 1){
    return *uniques.begin();
  }
  std::vector<Query*>::iterator it = uniques.begin();
  BooleanQuery* result = _CLNEW BooleanQuery(true);
  while (it != uniques.end() ){
    result->add(*it, BooleanClause::SHOULD);

    it++;
  }
  return result;
}
Similarity* Query::getSimilarity(Searcher* searcher) {
   return searcher->getSimilarity();
}
TCHAR* Query::toString() const{
   return toString(LUCENE_BLANK_STRING);
}

void Query::setBoost(float_t b) { boost = b; }

float_t Query::getBoost() const { return boost; }

Weight* Query::weight(Searcher* searcher){
    Query* query = searcher->rewrite(this);
    Weight* weight = query->_createWeight(searcher);
    float_t sum = weight->sumOfSquaredWeights();
    float_t norm = getSimilarity(searcher)->queryNorm(sum);
    weight->normalize(norm);
    return weight;
}

void Query::extractTerms( TermSet * termset ) const
{
	_CLTHROWA( CL_ERR_UnsupportedOperation,"UnsupportedOperationException: Query::extractTerms" );
}

TopFieldDocs::TopFieldDocs (int32_t totalHits, FieldDoc** fieldDocs, int32_t scoreDocsLen, SortField** fields):
 TopDocs (totalHits, NULL, scoreDocsLen)
{
	this->fields = fields;
	this->fieldDocs = fieldDocs;
	this->scoreDocs = new ScoreDoc[scoreDocsLen];
	for (int32_t i=0;i<scoreDocsLen;i++ )
		this->scoreDocs[i] = this->fieldDocs[i]->scoreDoc;
}
TopFieldDocs::~TopFieldDocs(){
	if ( fieldDocs ){
		for (int32_t i=0;i<scoreDocsLength;i++)
			_CLLDELETE(fieldDocs[i]);
		_CLDELETE_LARRAY(fieldDocs);
	}
	if ( fields != NULL ){
       for ( int32_t i=0;fields[i]!=NULL;i++ )
           _CLLDELETE(fields[i]);
       _CLDELETE_LARRAY(fields);
    }
}

TopDocs::TopDocs(const int32_t th, ScoreDoc*sds, int32_t scoreDocsLen):
    totalHits(th),
	scoreDocs(sds),
	scoreDocsLength(scoreDocsLen)
{
//Func - Constructor
//Pre  - sds may or may not be NULL
//       sdLength >= 0
//Post - The instance has been created

}

TopDocs::~TopDocs(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed

	delete[] scoreDocs;
}



Searcher::Searcher(){
	similarity = Similarity::getDefault();
}
Searcher::~Searcher(){
}

Hits* Searcher::search(Query* query) {
	return search(query, (Filter*)NULL );
}

Hits* Searcher::search(Query* query, Filter* filter) {
	return _CLNEW Hits(this, query, filter);
}

Hits* Searcher::search(Query* query, const Sort* sort){
	return _CLNEW Hits(this, query, NULL, sort);
}

Hits* Searcher::search(Query* query, Filter* filter, const Sort* sort){
	return _CLNEW Hits(this, query, filter, sort);
}

void Searcher::_search(Query* query, HitCollector* results) {
	_search(query, NULL, results);
}

void Searcher::setSimilarity(Similarity* similarity) {
	this->similarity = similarity;
}

Similarity* Searcher::getSimilarity(){
	return this->similarity;
}

const char* Searcher::getClassName(){
	return "Searcher";
}

const char* Searcher::getObjectName() const{
	return Searcher::getClassName();
}


Weight::~Weight(){
}

TCHAR* Weight::toString(){
     return STRDUP_TtoT(_T("Weight"));
}


Searchable::~Searchable(){
}


CL_NS_END
