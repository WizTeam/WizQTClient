/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "MultiFieldQueryParser.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/search/BooleanQuery.h"
#include "CLucene/search/BooleanClause.h"
#include "CLucene/search/PhraseQuery.h"
#include "CLucene/search/MultiPhraseQuery.h"
#include "CLucene/search/SearchHeader.h"
#include "QueryParser.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(search)
CL_NS_USE(analysis)

CL_NS_DEF(queryParser)


MultiFieldQueryParser::MultiFieldQueryParser(const TCHAR** _fields, CL_NS(analysis)::Analyzer* a, BoostMap* _boosts):
	QueryParser(NULL,a), fields(_fields), boosts(_boosts)
{
}
MultiFieldQueryParser::~MultiFieldQueryParser(){
}

Query* MultiFieldQueryParser::getFieldQuery(const TCHAR* field, TCHAR* queryText, const int32_t slop){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::getFieldQuery(fields[i], queryText);
			if (q != NULL) {
				//If the user passes a map of boosts
				if (boosts != NULL) {
					//Get the boost from the map and apply them
            BoostMap::const_iterator itr = boosts->find((TCHAR*)fields[i]);
            if (itr != boosts->end()) {
						q->setBoost(itr->second);
					}
				}
				if (q->instanceOf(PhraseQuery::getClassName())) {
					((PhraseQuery*)q)->setSlop(slop);
				}
        if (q->instanceOf(MultiPhraseQuery::getClassName())) {
					((MultiPhraseQuery*) q)->setSlop(slop);
				}
				clauses.push_back(_CLNEW BooleanClause(q, true, BooleanClause::SHOULD));
			}
		}
		if (clauses.size() == 0)  // happens for stopwords
			return NULL;
		return QueryParser::getBooleanQuery(clauses, true);
	}else{
		return QueryParser::getFieldQuery(field, queryText);
	}
}

Query* MultiFieldQueryParser::getFuzzyQuery(const TCHAR* field, TCHAR* termStr, const float_t minSimilarity){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::getFuzzyQuery(fields[i], termStr, minSimilarity);
			if (q) clauses.push_back(_CLNEW BooleanClause(q,true, BooleanClause::SHOULD) );
		}
		return QueryParser::getBooleanQuery(clauses, true);
	}
	return QueryParser::getFuzzyQuery(field, termStr, minSimilarity);
}

Query* MultiFieldQueryParser::getPrefixQuery(const TCHAR* field, TCHAR* termStr){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::getPrefixQuery(fields[i], termStr);
			if (q) clauses.push_back(_CLNEW BooleanClause(q,true,BooleanClause::SHOULD));
		}
		return QueryParser::getBooleanQuery(clauses, true);
	}
	return QueryParser::getPrefixQuery(field, termStr);
}

Query* MultiFieldQueryParser::getWildcardQuery(const TCHAR* field, TCHAR* termStr){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::getWildcardQuery(fields[i], termStr);
			if (q) clauses.push_back(_CLNEW BooleanClause(q,true,BooleanClause::SHOULD));
		}
		return QueryParser::getBooleanQuery(clauses, true);
	}
	return QueryParser::getWildcardQuery(field, termStr);
}


Query* MultiFieldQueryParser::getRangeQuery(const TCHAR* field, TCHAR* part1, TCHAR* part2, const bool inclusive){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::getRangeQuery(fields[i], part1, part2, inclusive);
			if (q) clauses.push_back(_CLNEW BooleanClause(q,true,BooleanClause::SHOULD));
		}
		return QueryParser::getBooleanQuery(clauses, true);
	}else{
		return QueryParser::getRangeQuery(field, part1, part2, inclusive);
	}
}

//static
Query* MultiFieldQueryParser::parse(const TCHAR** _queries, const TCHAR** _fields, Analyzer* analyzer)
{
	BooleanQuery* bQuery = _CLNEW BooleanQuery();
	for (size_t i = 0; _fields[i]!=NULL; i++)
	{
		if (_queries[i] == NULL) {
			_CLLDELETE(bQuery);
			_CLTHROWA(CL_ERR_IllegalArgument, "_queries.length != _fields.length");
		}
		// TODO: Reuse qp instead of creating it over and over again
		QueryParser* qp = _CLNEW QueryParser(_fields[i], analyzer);
		Query* q = qp->parse(_queries[i]);
		if (q!=NULL && // q never null, just being defensive
			(!(q->instanceOf(BooleanQuery::getClassName()) || ((BooleanQuery*)q)->getClauseCount() > 0))) {
				bQuery->add(q, true, BooleanClause::SHOULD);
		} else
			_CLLDELETE(q);
		_CLLDELETE(qp);
	}
	return bQuery;
}

// static
Query* MultiFieldQueryParser::parse(const TCHAR* query, const TCHAR** _fields, const uint8_t* flags, Analyzer* analyzer) {
	BooleanQuery* bQuery = _CLNEW BooleanQuery();
	for (size_t i = 0; _fields[i]!=NULL; i++) {
	  //TODO: this is really confusing... why not refactor _fields and flags to use a array object.
	  //flags can be NULL since NULL == 0...
		/*if (flags[i] == NULL) {
			_CLLDELETE(bQuery);
			_CLTHROWA(CL_ERR_IllegalArgument, "_fields.length != flags.length");
		}*/
		QueryParser* qp = _CLNEW QueryParser(_fields[i], analyzer);
		Query* q = qp->parse(query);
		if (q!=NULL && // q never null, just being defensive
			(!(q->instanceOf(BooleanQuery::getClassName())) || ((BooleanQuery*)q)->getClauseCount()>0)) {
				bQuery->add(q, true, (BooleanClause::Occur)flags[i]);
		} else
			_CLLDELETE(q);
		_CLLDELETE(qp);
	}
	return bQuery;
}

//static
Query* MultiFieldQueryParser::parse(const TCHAR** _queries, const TCHAR** _fields, const uint8_t* flags, Analyzer* analyzer){
	BooleanQuery* bQuery = _CLNEW BooleanQuery();
	for (size_t i = 0; _fields[i]!=NULL; i++)
	{
	  //TODO: this is really confusing... why not refactor _fields and flags to use a array object.
	  //flags can be NULL since NULL == 0...
		if (_queries[i] == NULL ) { //|| flags[i] == NULL
			_CLLDELETE(bQuery);
			_CLTHROWA(CL_ERR_IllegalArgument, "_queries, _fields, and flags array have have different length");
		}
		QueryParser* qp = _CLNEW QueryParser(_fields[i], analyzer);
		Query* q = qp->parse(_queries[i]);
		if (q!=NULL && // q never null, just being defensive
			(!(q->instanceOf(BooleanQuery::getClassName())) || ((BooleanQuery*)q)->getClauseCount()>0)) {
				bQuery->add(q, true, (BooleanClause::Occur)flags[i]);
		} else
			_CLLDELETE(q);
		_CLLDELETE(qp);
	}
	return bQuery;
}

CL_NS_END
