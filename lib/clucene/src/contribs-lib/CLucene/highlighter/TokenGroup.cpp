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
#include "TokenGroup.h"
#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(analysis)

TokenGroup::TokenGroup(void)
{
	numTokens=0;
	startOffset=0;
	endOffset=0;
	tokens = new Token[MAX_NUM_TOKENS_PER_GROUP];
}

TokenGroup::~TokenGroup(void)
{
	delete[] tokens;
}

void TokenGroup::addToken(Token* token, float_t score)
{
	if(numTokens < MAX_NUM_TOKENS_PER_GROUP)
    {	    
		if(numTokens==0)
		{
			startOffset=token->startOffset();		
			endOffset=token->endOffset();		
		}
		else
		{
			startOffset=cl_min(startOffset,token->startOffset());		
			endOffset=cl_max(endOffset,token->endOffset());		
		}
		tokens[numTokens].set(token->termBuffer(),token->startOffset(),token->endOffset(),token->type());;
		scores[numTokens]=score;
		numTokens++;
    }
}

CL_NS(analysis)::Token& TokenGroup::getToken(int32_t index)
{
	return tokens[index];
}

float_t TokenGroup::getScore(int32_t index) const
{
	return scores[index];
}

int32_t TokenGroup::getEndOffset() const
{
	return endOffset;
}

int32_t TokenGroup::getNumTokens() const
{
	return numTokens;
}

int32_t TokenGroup::getStartOffset() const
{
	return startOffset;
}

float_t TokenGroup::getTotalScore() const
{
	float_t total=0;
	for (int32_t i = 0; i < numTokens; i++)
	{
		total+=scores[i];
	}
	return total;
}

/*void addToken(CL_NS(analysis)::Token* token, float_t score)
{
	if(numTokens < MAX_NUM_TOKENS_PER_GROUP)
  		{
		if(numTokens==0)
		{
			startOffset=token->startOffset();		
			endOffset=token->endOffset();		
		}
		else
		{
			startOffset=min(startOffset,token->startOffset());		
			endOffset=max(endOffset,token->endOffset());		
		}
		tokens[numTokens]=token;
		scores[numTokens]=score;
		numTokens++;
	}
}*/

bool TokenGroup::isDistinct(CL_NS(analysis)::Token* token) const
{
	return token->startOffset() > endOffset;
}


void TokenGroup::clear()
{
	numTokens=0;
}

CL_NS_END2
