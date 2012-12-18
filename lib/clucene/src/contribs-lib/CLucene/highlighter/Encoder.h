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
#ifndef _lucene_search_highlight_encoder_
#define _lucene_search_highlight_encoder_


CL_NS_DEF2(search,highlight)

/**
 * Encodes original text. The Encoder works with the Formatter to generate the output.
 *
 */
class CLUCENE_CONTRIBS_EXPORT Encoder:LUCENE_BASE
{
public:
	/** Virtual destructor */
	virtual ~Encoder(){
	}

	/**
	 * @param originalText The section of text being output
	 */
	virtual TCHAR* encodeText(TCHAR* originalText) = 0;
};

/**
 * Simple {@link Encoder} implementation that does not modify the output
 * @author Nicko Cadell
 *
 */
class DefaultEncoder: public Encoder
{
public:
	TCHAR* encodeText(TCHAR* originalText)
	{
		return STRDUP_TtoT(originalText);
	}
};


CL_NS_END2

#endif
