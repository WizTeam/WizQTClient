/*------------------------------------------------------------------------------
* Copyright (C) 2009 Isidor Zeuner
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef CLUCENE_UTIL_STREAMARRAY_H
#define CLUCENE_UTIL_STREAMARRAY_H

#include <cstring>
#include "CLucene/util/CLStreams.h"
#include "CLucene/util/Array.h"

CL_NS_DEF(util)

template<typename element>
ValueArray<element> streamArray(CL_NS(util)::CLStream<element>* stored) {
	size_t const block_size = 4096;
	element const* retrieved;
	ValueArray<element> result(0);
	size_t available;
	size_t offset = 0;
	do {
		available = stored->read(retrieved, block_size, block_size);
		result.resize(result.length + sizeof(element) * available);
		memcpy(result.values + offset, retrieved, sizeof(element) * available);
		offset += available;
	} while (block_size == available);
	return result;
}
CL_NS_END
#endif
