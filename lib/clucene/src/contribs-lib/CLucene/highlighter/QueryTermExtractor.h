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
#ifndef _lucene_search_highlight_querytermextractor_
#define _lucene_search_highlight_querytermextractor_

CL_CLASS_DEF(search, Query)
CL_CLASS_DEF(search, BooleanQuery)
CL_CLASS_DEF(search, PhraseQuery)
CL_CLASS_DEF(search, TermQuery)
CL_CLASS_DEF(index, IndexReader)
CL_CLASS_DEF(index, Term)

#include "WeightedTerm.h"

CL_NS_DEF2(search,highlight)

/**
 * Utility class used to extract the terms used in a query, plus any weights.
 * This class will not find terms for MultiTermQuery, RangeQuery and PrefixQuery classes
 * so the caller must pass a rewritten query (see Query.rewrite) to obtain a list of 
 * expanded terms. 
 * 
 */
class CLUCENE_CONTRIBS_EXPORT QueryTermExtractor
{
	QueryTermExtractor(){
	}
public:

    /**
     * Extracts all terms texts of a given Query into an returned array of WeightedTerms
     *
     * @param query      Query to extract term texts from
     * @param prohibited <code>true</code> to extract "prohibited" terms, too
     * @param fieldName field name used for filtering query terms, MUST be interned prior to this call
     * @return an array of the terms used in a query, plus their weights.Memory owned by the caller
     */
	static WeightedTerm** getTerms(const Query *query, bool prohibited = false, const TCHAR* fieldName = NULL);

	/**
	 * Extracts all terms texts of a given Query into an array of WeightedTerms
	 *
	 * @param query      Query to extract term texts from
	 * @param reader used to compute IDF which can be used to a) score selected fragments better 
	 * b) use graded highlights eg chaning intensity of font color
	 * @param fieldName the field on which Inverse Document Frequency (IDF) calculations are based
	 * @return an array of the terms used in a query, plus their weights.
	 */
	static WeightedTerm** getIdfWeightedTerms(const Query* query, CL_NS(index)::IndexReader* reader, const TCHAR* fieldName);

    /**
     * Extracts all terms texts of a given Query into given array of WeightedTerms
     *
     * @param query      Query to extract term texts from
     * @param prohibited <code>true</code> to extract "prohibited" terms, too
     * @param fieldName field name used for filtering query terms, MUST be interned prior to this call
     * @return an array of the terms used in a query, plus their weights.Memory owned by the caller
     */
	static void getTerms(const Query * query, WeightedTermList*, bool prohibited = false, const TCHAR* fieldName = NULL);

	static void getTermsFromBooleanQuery(const BooleanQuery * query, WeightedTermList* terms, bool prohibited, const TCHAR* fieldName);
// 	static void getTermsFromFilteredQuery(const FilteredQuery * query, WeightedTermList* terms);
};

CL_NS_END2



#endif
