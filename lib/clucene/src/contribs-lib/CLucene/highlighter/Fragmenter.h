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

#ifndef _lucene_search_highlight_fragmenter_
#define _lucene_search_highlight_fragmenter_


CL_CLASS_DEF(analysis, Token)

CL_NS_DEF2(search,highlight)

/**
 * Implements the policy for breaking text into multiple fragments for consideration
 * by the {@link Highlighter} class. A sophisticated implementation may do this on the basis
 * of detecting end of sentences in the text. 
 */
class CLUCENE_CONTRIBS_EXPORT Fragmenter:LUCENE_BASE
{
public:
	/** Virtual destructor */
	virtual ~Fragmenter(){
	}

	/**
	 * Initializes the Fragmenter
	 * @param originalText
	 */
	virtual void start(const TCHAR* originalText) = 0;

	/**
	 * Test to see if this token from the stream should be held in a new TextFragment
	 * @param nextToken
	 */
	virtual bool isNewFragment(const CL_NS(analysis)::Token * nextToken) = 0;
};

CL_NS_END2

#endif
