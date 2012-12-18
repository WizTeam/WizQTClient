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

#ifndef _lucene_search_highlight_simplehtmlformatter_
#define _lucene_search_highlight_simplehtmlformatter_


#include "Formatter.h"

CL_NS_DEF2(search,highlight)

/**
 * Simple {@link Formatter} implementation to highlight terms with a pre and post tag
 *
 */
class CLUCENE_CONTRIBS_EXPORT SimpleHTMLFormatter :public Formatter
{
private:
	TCHAR* _preTag;
	TCHAR* _postTag;

public:
	~SimpleHTMLFormatter(); 


	SimpleHTMLFormatter(const TCHAR* preTag, const TCHAR* postTag);

	/**
	 * Default constructor uses HTML: &lt;B&gt; tags to markup terms
	 * 
	 **/
	SimpleHTMLFormatter();

	
	/**
	* Returns the original text enclosed in _preTag and _postTag, if the score is greater 
	* than 0. Otherwise, it returns the original text.
	* It doesn't use the stemmed text nor the startOffset. 
	* It allocates memory for the returned text, and it has to be freed by the caller.
	*/
	TCHAR* highlightTerm(const TCHAR* originalText, const TokenGroup* tokenGroup);
};

CL_NS_END2

#endif
