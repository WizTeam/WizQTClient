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

#ifndef _lucene_search_highlight_weightedterm_
#define _lucene_search_highlight_weightedterm_


//#include "CLucene/util/VoidList.h"

CL_NS_DEF2(search,highlight)

/** Lightweight class to hold term and a weight value used for scoring this term 
 */
class CLUCENE_CONTRIBS_EXPORT WeightedTerm:LUCENE_BASE
{
private:
	float_t _weight; // multiplier
	TCHAR* _term; //stemmed form
	size_t cachedHashCode;
	WeightedTerm(const WeightedTerm& other);
public:
	WeightedTerm (float_t weight,const TCHAR* term);
	~WeightedTerm();

	/**
	 * @return the term value (stemmed)
	 */
	const TCHAR* getTerm() const;

	/**
	 * @return the weight associated with this term
	 */
	float_t getWeight() const ;

	/**
	 * @param term the term value (stemmed)
	 */
	void setTerm(TCHAR* term);
	/**
	 * @param weight the weight associated with this term
	 */
	void setWeight(float_t weight);

	size_t hashCode();
	WeightedTerm* clone() const;

	/**
	 * Compare weighted terms, according to the term text.
	 * @todo Do we have to take boost factors into account
	 */
	class Compare:LUCENE_BASE, public CL_NS(util)::Compare::_base //<WeightedTerm*>
	{
	public:
	//todo: this should be more efficient, but will be using a hash table soon, anyway
		bool operator()( WeightedTerm* t1, WeightedTerm* t2 ) const;
		size_t operator()( WeightedTerm* t ) const;
	};
};

/** CLHashSet of WeightedTerm */
typedef CL_NS(util)::CLHashSet<WeightedTerm*, WeightedTerm::Compare, CL_NS(util)::Deletor::Object<WeightedTerm> > WeightedTermList;

CL_NS_END2

#endif

