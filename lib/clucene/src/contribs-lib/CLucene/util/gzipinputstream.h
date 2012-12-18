/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef CLUENE_UTIL_GZIPINPUTSTREAM_H
#define CLUENE_UTIL_GZIPINPUTSTREAM_H

#include "CLucene/util/CLStreams.h"

struct z_stream_s;

CL_NS_DEF(util)

class CLUCENE_CONTRIBS_EXPORT GZipInputStream : public CL_NS(util)::BufferedInputStream {
public:
    enum ZipFormat { ZLIBFORMAT, GZIPFORMAT, ZIPFORMAT};
private:
	 class Internal;
	 Internal* internal;
public:
    _CL_DEPRECATED(Use compressed field) explicit GZipInputStream(BufferedInputStream* input,
        ZipFormat format=GZIPFORMAT);
    _CL_DEPRECATED(Use compressed field) explicit GZipInputStream(InputStream* input,
        ZipFormat format=GZIPFORMAT);
    ~GZipInputStream();

	int32_t read(const signed char*& start, int32_t min, int32_t max);
	int32_t read(const unsigned char*& start, int32_t min, int32_t max);
	int64_t position();
	int64_t reset(int64_t);
	int64_t skip(int64_t ntoskip);
	size_t size();
	void setMinBufSize(int32_t minbufsize);
};

CL_NS_END
#endif
