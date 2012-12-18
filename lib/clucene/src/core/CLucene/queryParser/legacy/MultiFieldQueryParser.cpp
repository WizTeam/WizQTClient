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
#include "CLucene/search/SearchHeader.h"
#include "QueryParser.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(search)
CL_NS_USE(analysis)

CL_NS_DEF2(queryParser,legacy)

MultiFieldQueryParser::MultiFieldQueryParser(const TCHAR** fields, CL_NS(analysis)::Analyzer* a, BoostMap* boosts):
	QueryParser(NULL,a)
{
	this->fields = fields;
    this->boosts = boosts;
}
MultiFieldQueryParser::~MultiFieldQueryParser(){
}

//static
Query* MultiFieldQueryParser::parse(const TCHAR* query, const TCHAR** fields, Analyzer* analyzer)
{
    BooleanQuery* bQuery = _CLNEW BooleanQuery( true );
    int32_t i = 0;
	while ( fields[i] != NULL ){
		Query* q = QueryParser::parse(query, fields[i], analyzer);
		if (q && ( !q->instanceOf(BooleanQuery::getClassName()) || ((BooleanQuery*)q)->getClauseCount() > 0)) {
			//todo: Move to using BooleanClause::Occur
			bQuery->add(q, true, false, false);
		} else {
			_CLDELETE(q);
		}

		i++;
	}
    return bQuery;
}

//static
Query* MultiFieldQueryParser::parse(const TCHAR* query, const TCHAR** fields, const uint8_t* flags, Analyzer* analyzer)
{
    BooleanQuery* bQuery = _CLNEW BooleanQuery( true );
    int32_t i = 0;
    while ( fields[i] != NULL )
    {
		Query* q = QueryParser::parse(query, fields[i], analyzer);
		if (q && ( !q->instanceOf(BooleanQuery::getClassName()) || ((BooleanQuery*)q)->getClauseCount() > 0)) {
			uint8_t flag = flags[i];
			switch (flag)
			{
				//todo: Move to using BooleanClause::Occur
			case MultiFieldQueryParser::REQUIRED_FIELD:
				bQuery->add(q, true, true, false);
				break;
			case MultiFieldQueryParser::PROHIBITED_FIELD:
				bQuery->add(q, true, false, true);
				break;
			default:
				bQuery->add(q, true, false, false);
				break;
			}
		} else {
			_CLDELETE(q);
		}

        i++;
    }
    return bQuery;
}

//not static
CL_NS(search)::Query* MultiFieldQueryParser::parse(const TCHAR* query) {
	return parse(query, this->fields, this->analyzer);
}

Query* MultiFieldQueryParser::GetFieldQuery(const TCHAR* field, TCHAR* queryText, int32_t slop){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::GetFieldQuery(fields[i], queryText);
			if (q != NULL) {
				//If the user passes a map of boosts
				if (boosts != NULL) {
					//Get the boost from the map and apply them
            BoostMap::iterator itr = boosts->find( fields[i]);
            if (itr != boosts->end()) {
						q->setBoost(itr->second);
					}
				}
				if (q->instanceOf(PhraseQuery::getClassName()) ) {
					((PhraseQuery*)q)->setSlop(slop);
				}
				//if (q instanceof MultiPhraseQuery) {
				//	((MultiPhraseQuery) q).setSlop(slop);
				//}
				q = QueryAddedCallback(fields[i], q);
				if ( q )
					clauses.push_back(_CLNEW BooleanClause(q, true, false,false));
			}
		}
		if (clauses.size() == 0)  // happens for stopwords
			return NULL;
        Query* q = QueryParser::GetBooleanQuery(clauses);
		return q;
	}else{
		Query* q = QueryParser::GetFieldQuery(field, queryText);
		if ( q )
			q = QueryAddedCallback(field,q);
		return q;
	}
}


Query* MultiFieldQueryParser::GetFieldQuery(const TCHAR* field, TCHAR* queryText){
	return GetFieldQuery(field, queryText, 0);
}


CL_NS(search)::Query* MultiFieldQueryParser::GetFuzzyQuery(const TCHAR* field, TCHAR* termStr){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::GetFuzzyQuery(fields[i], termStr); //todo: , minSimilarity
			if ( q ){
				q = QueryAddedCallback(fields[i], q);
				if ( q ){
					clauses.push_back(_CLNEW BooleanClause(q,true,false,false) );
				}
			}
		}
		return QueryParser::GetBooleanQuery(clauses);
	}else{
		Query* q = QueryParser::GetFuzzyQuery(field, termStr);//todo: , minSimilarity
		if ( q )
			q = QueryAddedCallback(field,q);
		return q;
	}
}

Query* MultiFieldQueryParser::GetPrefixQuery(const TCHAR* field, TCHAR* termStr){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::GetPrefixQuery(fields[i], termStr);
			if ( q ){
				q = QueryAddedCallback(fields[i],q);
				if ( q ){
					clauses.push_back(_CLNEW BooleanClause(q,true,false,false));
				}
			}
		}
		return QueryParser::GetBooleanQuery(clauses);
	}else{
		Query* q = QueryParser::GetPrefixQuery(field, termStr);
		if ( q )
			q = QueryAddedCallback(field,q);
		return q;
	}
}

Query* MultiFieldQueryParser::GetWildcardQuery(const TCHAR* field, TCHAR* termStr){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::GetWildcardQuery(fields[i], termStr);
			if ( q ){
				q = QueryAddedCallback(fields[i],q);
				if ( q ){
					clauses.push_back(_CLNEW BooleanClause(q,true,false,false));
				}
			}
		}
		return QueryParser::GetBooleanQuery(clauses);
	}else{
		Query* q = QueryParser::GetWildcardQuery(field, termStr);
		if ( q )
			q = QueryAddedCallback(field,q);
		return q;
	}
}


Query* MultiFieldQueryParser::GetRangeQuery(const TCHAR* field, TCHAR* part1, TCHAR* part2, bool inclusive){
	if (field == NULL) {
		vector<BooleanClause*> clauses;
		for (int i = 0; fields[i]!=NULL; ++i) {
			Query* q = QueryParser::GetRangeQuery(fields[i], part1, part2, inclusive);
			if ( q ){
				q = QueryAddedCallback(fields[i],q);
				if ( q ){
					clauses.push_back(_CLNEW BooleanClause(q,true,false,false));
				}
			}
		}
		return QueryParser::GetBooleanQuery(clauses);
	}else{
		Query* q = QueryParser::GetRangeQuery(field, part1, part2, inclusive);
		if ( q )
			q = QueryAddedCallback(field,q);
		return q;
	}
}

CL_NS_END2
