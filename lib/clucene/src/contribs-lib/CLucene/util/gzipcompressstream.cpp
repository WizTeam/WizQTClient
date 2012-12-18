/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "gzipcompressstream.h"
#include "CLucene/util/_bufferedstream.h"
#include <zlib.h>

CL_NS_DEF(util)

class GZipCompressInputStream::Internal: public CL_NS(util)::BufferedInputStreamImpl{
  z_stream_s* zstream;
  InputStream* input;
protected:
  int32_t fillBuffer(signed char* start, int32_t space){
	 if (zstream == 0) return -1;
	 // make sure there is data to decompress
	 if (zstream->avail_in==0) {
		  // read data from the input stream
		  const signed char* inStart;
		  int32_t nread = input->read(inStart, 1, 0);
		  if (nread < 1) {
			 zstream->avail_in = 0; //bail...
			 if (deflate(zstream, Z_FINISH) != Z_STREAM_END) {
				  _CLTHROWA(CL_ERR_IO, "deflate should report Z_STREAM_END\n");
			 }
			 int32_t nwritten = space - zstream->avail_out;
			 dealloc();
			 return nwritten;
		  }
		  zstream->next_in = (Bytef*)inStart;
		  zstream->avail_in = nread;
	 }

	 // make sure we can write into the buffer
	 zstream->avail_out = space;
	 zstream->next_out = (Bytef*)start;

	 int r = deflate(zstream, Z_NO_FLUSH);
	 // inform the buffer of the number of bytes that was read
	 int32_t nwritten = space - zstream->avail_out;
	 switch (r) {
	 case Z_NEED_DICT:
		  _CLTHROWA(CL_ERR_IO, "Z_NEED_DICT while inflating stream.");
		  break;
	 case Z_DATA_ERROR:
		   _CLTHROWA(CL_ERR_IO, "Z_DATA_ERROR while inflating stream.");
		  break;
	 case Z_MEM_ERROR:
		   _CLTHROWA(CL_ERR_IO, "Z_MEM_ERROR while inflating stream.");
		  break;
	 }
	 return nwritten;
  }
public:
  void dealloc(){
	 if (zstream) {
		  deflateEnd(zstream);
		  free(zstream);
		  zstream = 0;
	 }
  }
  void _setMinBufSize(int buf){
	 this->setMinBufSize(buf);
  }
  Internal(InputStream* input, int level){
	 if ( level < 0 || level > 9 )
		  level = Z_DEFAULT_COMPRESSION;

	 this->input = input;

	 // initialize the z_stream
	 zstream = (z_stream_s*)malloc(sizeof(z_stream_s));
	 zstream->zalloc = Z_NULL;
	 zstream->zfree = Z_NULL;
	 zstream->opaque = Z_NULL;
	 zstream->avail_in = 0;
	 
	 // initialize for writing gzip streams
	 int r = deflateInit(zstream, level);
	 if (r != Z_OK) {
		  dealloc();
		  _CLTHROWA(CL_ERR_IO, "Error initializing GZipCompressInputStream.");
	 }

	 // signal that we need to read into the buffer
	 zstream->avail_out = 1;
  }

  ~Internal(){
	 dealloc();
  }
};


GZipCompressInputStream::GZipCompressInputStream ( InputStream* input, int level)
{
	internal = new Internal(input,level);
}
size_t GZipCompressInputStream::size(){
	return internal->size();
}
GZipCompressInputStream::~GZipCompressInputStream ()
{
	delete internal;
}
int32_t GZipCompressInputStream::read(const signed char*& start, int32_t min, int32_t max){
	return internal->read(start,min,max);
}
int64_t GZipCompressInputStream::position(){
	return internal->position();
}
int64_t GZipCompressInputStream::reset(int64_t to){
	return internal->reset(to);
}
int64_t GZipCompressInputStream::skip(int64_t ntoskip){
	return internal->skip(ntoskip);
}
void GZipCompressInputStream::setMinBufSize(int32_t minbufsize){
	internal->_setMinBufSize(minbufsize);
}

CL_NS_END
