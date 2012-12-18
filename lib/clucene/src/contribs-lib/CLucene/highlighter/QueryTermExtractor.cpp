/**
 * Copyright 2002-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CLucene/_ApiHeader.h"
#include "QueryTermExtractor.h"

#include "CLucene/search/Query.h"
#include "CLucene/search/BooleanQuery.h"
#include "CLucene/search/TermQuery.h"
#include "CLucene/search/PhraseQuery.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(index)

	WeightedTerm** QueryTermExtractor::getTerms(const Query * query, bool prohibited, const TCHAR* fieldName) 
	{
		WeightedTermList terms(false);
		getTerms(query,&terms,prohibited,fieldName);

		// Return extracted terms
		WeightedTerm** ret = _CL_NEWARRAY(WeightedTerm*,terms.size()+1);
		terms.toArray_nullTerminated(ret);

		return ret;
	}

	void QueryTermExtractor::getTerms(const Query * query, WeightedTermList * terms, bool prohibited, const TCHAR* fieldName) 
	{
		if (query->instanceOf( BooleanQuery::getClassName() ))
        {
			getTermsFromBooleanQuery((BooleanQuery *) query, terms, prohibited, fieldName);
        }
// FilteredQuery not implemented yet
// 		else if (query->instanceOf( FilteredQuery::getClassName() ))
// 			getTermsFromFilteredQuery((FilteredQuery *) query, terms);
		else
        {
            TermSet nonWeightedTerms;
            query->extractTerms(&nonWeightedTerms);
            for (TermSet::iterator iter = nonWeightedTerms.begin(); iter != nonWeightedTerms.end(); iter++)
            {
                Term * term = (Term *)(*iter);
                if ( fieldName == NULL || term->field() == fieldName )
                    terms->insert(_CLNEW WeightedTerm(query->getBoost(), term->text()));
                _CLLDECDELETE( term );
            }
        }
	}

	/**
  	* Extracts all terms texts of a given Query into an array of WeightedTerms
  	*
  	* @param query      Query to extract term texts from
  	* @param reader used to compute IDF which can be used to a) score selected fragments better
  	* b) use graded highlights eg chaning intensity of font color
  	* @param fieldName the field on which Inverse Document Frequency (IDF) calculations are based
  	* @return an array of the terms used in a query, plus their weights.
  	*/
  	WeightedTerm** QueryTermExtractor::getIdfWeightedTerms(const Query* query, IndexReader* reader, const TCHAR* fieldName)
  	{
  	    WeightedTermList terms(true);
		getTerms(query,&terms,false,fieldName);

  	    int32_t totalNumDocs=reader->numDocs();
		
		WeightedTermList::iterator itr = terms.begin();
  	    while ( itr != terms.end() )
  		{
  			try
  			{
				Term* term = _CLNEW Term(fieldName,(*itr)->getTerm());
  				int32_t docFreq=reader->docFreq(term);
				_CLDECDELETE(term);

  				//IDF algorithm taken from DefaultSimilarity class
  				float_t idf=(float_t)(log(totalNumDocs/(float_t)(docFreq+1)) + 1.0);
  				(*itr)->setWeight((*itr)->getWeight() * idf);
  			}catch (CLuceneError& e){
  				if ( e.number()!=CL_ERR_IO )
					throw e;
  			}

			itr++;
  		}
  	   
		// Return extracted terms
		WeightedTerm** ret = _CL_NEWARRAY(WeightedTerm*,terms.size()+1);
		terms.toArray_nullTerminated(ret);

		return ret;
  	}

	void QueryTermExtractor::getTermsFromBooleanQuery(const BooleanQuery * query, WeightedTermList * terms, bool prohibited, const TCHAR* fieldName)
	{
		uint32_t numClauses = query->getClauseCount();
		BooleanClause** queryClauses = _CL_NEWARRAY(BooleanClause*,numClauses);
		query->getClauses(queryClauses);

		for (uint32_t i = 0; i < numClauses; i++)
		{
			if (prohibited || !queryClauses[i]->prohibited){
				Query* qry = queryClauses[i]->getQuery();
				getTerms(qry, terms, prohibited, fieldName);
			}
		}
		_CLDELETE_ARRAY(queryClauses);
	}

// FilteredQuery not implemented yet
//     void QueryTermExtractor::getTermsFromFilteredQuery(const FilteredQuery * query, WeightedTermList * terms, bool prohibited)
//     {
//         getTerms(query->getQuery(), terms, prohibited);
//     }

CL_NS_END2
