/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef CLUENE_UTIL_GZIPCOMPRESSSTREAM_H
#define CLUENE_UTIL_GZIPCOMPRESSSTREAM_H

#include "CLucene/util/CLStreams.h"

struct z_stream_s;

CL_NS_DEF(util)

class CLUCENE_CONTRIBS_EXPORT GZipCompressInputStream : public InputStream{
private:
	 class Internal;
	 Internal* internal;
public:
	 LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_BUFFER_SIZE=4096);
   _CL_DEPRECATED(Use compressed field)  explicit GZipCompressInputStream(InputStream* input, int level=-1);
   _CL_DEPRECATED(Use compressed field)  virtual ~GZipCompressInputStream();
	
	 int32_t read(const signed char*& start, int32_t min, int32_t max);
	 int64_t position();
	 int64_t reset(int64_t);
	 int64_t skip(int64_t ntoskip);
	 size_t size();
	 void setMinBufSize(int32_t minbufsize);
};

CL_NS_END
#endif
