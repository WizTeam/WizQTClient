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
#include "WeightedTerm.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/util/Misc.h"

CL_NS_DEF2(search,highlight)

	WeightedTerm::WeightedTerm (float_t weight,const TCHAR* term)
	{
		_weight=weight;
		_term = stringDuplicate(term);
		cachedHashCode = 0;
	}
	
	WeightedTerm::~WeightedTerm()
	{
		_CLDELETE_CARRAY(_term);
	}

	WeightedTerm::WeightedTerm(const WeightedTerm& other) 
	{
		_weight = other.getWeight();
		_term = STRDUP_TtoT(other.getTerm());
	}
	
	WeightedTerm* WeightedTerm::clone() const{
		return _CLNEW WeightedTerm(*this);
	}

	/**
	 * @return the term value (stemmed)
	 */
	const TCHAR* WeightedTerm::getTerm() const
	{
		return _term;
	}

	/**
	 * @return the weight associated with this term
	 */
	float_t WeightedTerm::getWeight() const 
	{
		return _weight;
	}

	/**
	 * @param term the term value (stemmed)
	 */
	void WeightedTerm::setTerm(TCHAR* term)
	{
		_CLDELETE_CARRAY(this->_term);
		this->_term = STRDUP_TtoT(_term);
		cachedHashCode = 0;
	}

	/**
	 * @param weight the weight associated with this term
	 */
	void WeightedTerm::setWeight(float_t weight) {
		this->_weight = _weight;
		cachedHashCode = 0;
	}

	size_t WeightedTerm::hashCode(){
		if ( cachedHashCode == 0 ){
			cachedHashCode = ( CL_NS(util)::Misc::thashCode(this->_term) ^ CL_NS(search)::Similarity::floatToByte(_weight) );
		}

		return cachedHashCode;
	}

	bool WeightedTerm::Compare::operator()( WeightedTerm* t1, WeightedTerm* t2 ) const{
		int r = _tcscmp(t1->getTerm(), t2->getTerm());
		if ( r < 0 )
			return true;
		else if ( r == 0 )
			return t1->getWeight() < t2->getWeight();
		else
			return false;
	}
	size_t WeightedTerm::Compare::operator()( WeightedTerm* t ) const{
		return t->hashCode();
	}

CL_NS_END2
