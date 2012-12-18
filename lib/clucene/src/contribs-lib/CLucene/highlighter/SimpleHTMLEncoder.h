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

#ifndef _lucene_search_highlight_simplehtmlencoder_
#define _lucene_search_highlight_simplehtmlencoder_


#include "Encoder.h"

CL_NS_DEF2(search,highlight)

/**
 * Simple {@link Encoder} implementation to escape text for HTML output
 *
 */
class CLUCENE_CONTRIBS_EXPORT SimpleHTMLEncoder:public Encoder
{
public:
	SimpleHTMLEncoder(void);
	~SimpleHTMLEncoder(void);
	
	TCHAR* encodeText(TCHAR* originalText);
	
	/**
	 * Encode string into HTML
	 */
	static TCHAR* htmlEncode(TCHAR* plainText) ;
};

CL_NS_END2

#endif
