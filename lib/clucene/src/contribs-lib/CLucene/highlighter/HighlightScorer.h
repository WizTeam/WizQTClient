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

#ifndef _lucene_search_highlight_highlighterscorer_
#define _lucene_search_highlight_highlighterscorer_


CL_CLASS_DEF(analysis, Token)
//#include "TextFragment.h"

CL_NS_DEF2(search,highlight)
class TextFragment;

/**
 * Adds to the score for a fragment based on its tokens
 */
class CLUCENE_CONTRIBS_EXPORT HighlightScorer:LUCENE_BASE
{
public:
	virtual ~HighlightScorer(){
	}

	/**
	 * called when a new fragment is started for consideration
	 * @param newFragment
	 */
	virtual void startFragment(TextFragment* newFragment) = 0;

	/**
	 * Called for each token in the current fragment
	 * @param token The token to be scored
	 * @return a score which is passed to the Highlighter class to influence the mark-up of the text
	 * (this return value is NOT used to score the fragment)
	 */
	virtual float_t getTokenScore(CL_NS(analysis)::Token* token) = 0;
	

	/**
	 * Called when the highlighter has no more tokens for the current fragment - the scorer returns
	 * the weighting it has derived for the most recent fragment, typically based on the tokens
	 * passed to getTokenScore(). 
	 *
	 */	
	virtual float_t getFragmentScore() = 0;
};

CL_NS_END2
#endif


