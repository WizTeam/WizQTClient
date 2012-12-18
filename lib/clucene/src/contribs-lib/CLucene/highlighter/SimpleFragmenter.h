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

#ifndef _lucene_search_highlight_simplefragmenter_
#define _lucene_search_highlight_simplefragmenter_


#include "Fragmenter.h"

CL_NS_DEF2(search,highlight)

/**
 * {@link Fragmenter} implementation which breaks text up into same-size 
 * fragments with no concerns over spotting sentence boundaries.
 */

class CLUCENE_CONTRIBS_EXPORT SimpleFragmenter:public Fragmenter
{
private:
	LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_FRAGMENT_SIZE =100 );
	int32_t _currentNumFrags;
	int32_t _fragmentSize;

public:
	/**
	 * 
	 * @param fragmentSize size in bytes of each fragment
	 */
	SimpleFragmenter(int32_t fragmentSize = DEFAULT_FRAGMENT_SIZE);

	~SimpleFragmenter();

	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.TextFragmenter#start(const TCHAR*)
	 */
	void start(const TCHAR* originalText);

	/* (non-Javadoc)
	 * @see org.apache.lucene.search.highlight.TextFragmenter#isNewFragment(org.apache.lucene.analysis.Token)
	 */
	bool isNewFragment(const CL_NS(analysis)::Token * token);

	/**
	 * @return size in bytes of each fragment
	 */
	int32_t getFragmentSize() const;

	/**
	 * @param size size in bytes of each fragment
	 */
	void setFragmentSize(int32_t size);

};

CL_NS_END2

#endif
