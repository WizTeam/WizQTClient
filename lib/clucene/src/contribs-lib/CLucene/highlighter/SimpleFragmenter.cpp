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
#include "SimpleFragmenter.h"
#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(analysis)

SimpleFragmenter::SimpleFragmenter(int32_t fragmentSize)
	: _fragmentSize(fragmentSize), _currentNumFrags(0)
{
}
SimpleFragmenter::~SimpleFragmenter(){
}

void SimpleFragmenter::start(const TCHAR*)
{
	_currentNumFrags=1;
}

bool SimpleFragmenter::isNewFragment(const Token * token)
{
	bool isNewFrag= token->endOffset()>=(_fragmentSize*_currentNumFrags);
	if (isNewFrag) {
		_currentNumFrags++;
	}
	return isNewFrag;
}

int32_t SimpleFragmenter::getFragmentSize() const
{
	return _fragmentSize;
}

void SimpleFragmenter::setFragmentSize(int32_t size)
{
	_fragmentSize = size;
}

CL_NS_END2
