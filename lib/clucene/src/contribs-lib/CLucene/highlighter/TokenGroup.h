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

#ifndef _lucene_search_highlight_tokengroup_
#define _lucene_search_highlight_tokengroup_


CL_CLASS_DEF(analysis, Token)

CL_NS_DEF2(search,highlight)

/**
 * One, or several overlapping tokens, along with the score(s) and the
 * scope of the original text
 */
class CLUCENE_CONTRIBS_EXPORT TokenGroup: LUCENE_BASE
{
	LUCENE_STATIC_CONSTANT(int32_t,MAX_NUM_TOKENS_PER_GROUP=50);
	CL_NS(analysis)::Token* tokens;
	float_t scores[MAX_NUM_TOKENS_PER_GROUP];
	int32_t numTokens;
	int32_t startOffset;
	int32_t endOffset;

public:
	TokenGroup(void);
	~TokenGroup(void);

	void addToken(CL_NS(analysis)::Token* token, float_t score);

	/**
	 * 
	 * @param index a value between 0 and numTokens -1
	 * @return the "n"th token
	 */
	CL_NS(analysis)::Token& getToken(int32_t index);

	/**
	 * 
	 * @param index a value between 0 and numTokens -1
	 * @return the "n"th score
	 */
	float_t getScore(int32_t index) const;

	/**
	 * @return the end position in the original text
	 */
	int32_t getEndOffset() const;

	/**
	 * @return the number of tokens in this group
	 */
	int32_t getNumTokens() const;

	/**
	 * @return the start position in the original text
	 */
	int32_t getStartOffset() const;

	/**
	 * @return all tokens' scores summed up
	 */
	float_t getTotalScore() const;

	bool isDistinct(CL_NS(analysis)::Token* token)  const;
	void clear();
};

CL_NS_END2
#endif
