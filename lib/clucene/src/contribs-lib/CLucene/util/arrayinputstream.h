/*------------------------------------------------------------------------------
* Copyright (C) 2009 Isidor Zeuner
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef CLUCENE_UTIL_ARRAYINPUTSTREAM_H
#define CLUCENE_UTIL_ARRAYINPUTSTREAM_H

#include "CLucene/_ApiHeader.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/util/Array.h"

CL_NS_DEF(util)

template<typename element>
class CLUCENE_CONTRIBS_EXPORT ArrayInputStream : public CL_NS(util)::CLStream<element> {
public:
	_CL_DEPRECATED(Use compressed field) ArrayInputStream(ArrayBase<element> const* data);
	int32_t read(const element*& start, int32_t min, int32_t max);
	int64_t skip(int64_t ntoskip);
	int64_t position();
	size_t size();
private:
	ArrayBase<element> const* data;
	int64_t current_position;
};

template<typename element>
ArrayInputStream<element>::ArrayInputStream(ArrayBase<element> const* data) :
data(data),
current_position(0) {
}

template<typename element>
int32_t ArrayInputStream<element>::read(const element*& start, int32_t min, int32_t max) {
	int32_t to_read = min;
	int32_t readable = data->length - current_position;
	if (readable < to_read) {
		to_read = readable;
	}
	start = data->values + current_position;
	current_position += to_read;
	return to_read;
}
	
template<typename element>
int64_t ArrayInputStream<element>::skip(int64_t ntoskip) {
	int64_t to_skip = ntoskip;
	int64_t skippable = data->length - current_position;
	if (skippable < to_skip) {
		to_skip = skippable;
	}
	current_position += to_skip;
	return to_skip;
}

template<typename element>
int64_t ArrayInputStream<element>::position() {
	return current_position;
}
	
template<typename element>
size_t ArrayInputStream<element>::size() {
	return data->length;
}
CL_NS_END
#endif
