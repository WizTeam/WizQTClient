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

#ifndef _lucene_search_highlight_textfragment_
#define _lucene_search_highlight_textfragment_


CL_CLASS_DEF(util, StringBuffer)
//#include "CLucene/util/VoidList.h"

CL_NS_DEF2(search,highlight)

/**
 * Low-level class used to record information about a section of a document 
 * with a score.
 */
class CLUCENE_CONTRIBS_EXPORT TextFragment:LUCENE_BASE
{
	int32_t _fragNum;
	int32_t _textStartPos;
	int32_t _textEndPos;
	float_t _score;

public:
	TextFragment(int32_t textStartPos, int32_t fragNum);
	~TextFragment();

	void setScore(float_t score);
	float_t getScore() const;

	int32_t textEndPos(){ return _textEndPos; }
	void setTextEndPos(int32_t val){ _textEndPos = val; }

	/**
	 * @param frag2 Fragment to be merged into this one
	 */
	void merge(const TextFragment * frag2);

	/**
	 * @param fragment 
	 * @return true if this fragment follows the one passed
	 */
	bool follows(const TextFragment * fragment) const;

	/**
	 * @return the fragment sequence number
	 */
	int32_t getFragNum() const;

	/* Returns the marked-up text for this text fragment 
	 */
	TCHAR* toString(CL_NS(util)::StringBuffer* buffer);

	/**
	 * Compare weighted terms, according to the term text.
	 * @todo Do we have to take boost factors into account
	 */
	class Compare:LUCENE_BASE, public CL_NS(util)::Compare::_base //<TextFragment*>
	{
	public:
	//todo: this should be more efficient, but will be using a hash table soon, anyway
		bool operator()( TextFragment* t1, TextFragment* t2 ) const;
		size_t operator()( TextFragment* t ) const;
	};
};

/**
 * Text fragment list.
 */
//typedef CL_NS(util)::CLSetList<TextFragment*,TextFragment::Compare,CL_NS(util)::Deletor::Object<TextFragment> > TextFragmentList;

CL_NS_END2

#endif
