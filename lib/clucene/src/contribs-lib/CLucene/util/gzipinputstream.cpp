/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "gzipinputstream.h"
#include "CLucene/util/_bufferedstream.h"
#include <zlib.h>

CL_NS_DEF(util)


class GZipInputStream::Internal{
public:
	class JStreamsBuffer: public BufferedInputStreamImpl{
		z_stream_s* zstream;
		BufferedInputStream* input;
	protected:

		int32_t fillBuffer(signed char* start, int32_t space) {
			 if (zstream == 0) return -1;
			 // make sure there is data to decompress
			 if (zstream->avail_out) {
				  // read data from the input stream
				  const signed char* inStart;
				  int32_t nread = input->read(inStart, 1, 0);
				  if (nread < 1) {
						_CLTHROWA(CL_ERR_IO, "unexpected end of stream");
				  } else {
						zstream->next_in = (Bytef*)inStart;
						zstream->avail_in = nread;
				  }
			 }
			 // make sure we can write into the buffer
			 zstream->avail_out = space;
			 zstream->next_out = (Bytef*)start;
			 // decompress
			 int r = inflate(zstream, Z_SYNC_FLUSH);
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
			 case Z_STREAM_END:
				  if (zstream->avail_in) {
						input->reset(input->position()-zstream->avail_in);
				  }
				  // we are finished decompressing,
				  // (but this stream is not yet finished)
				  dealloc();
			 }
			 return nwritten;
		}
		void dealloc() {
			 if (zstream) {
				  inflateEnd(zstream);
				  free(zstream);
				  zstream = 0;
			 }
		}
		bool checkMagic() {
			 const unsigned char* buf;
			 const signed char* begin;
			 int32_t nread;

			 int64_t pos = input->position();
			 nread = input->read(begin, 2, 2);
			 input->reset(pos);
			 if (nread != 2) {
				  return false;
			 }

			 buf = (const unsigned char*)begin;
			 return buf[0] == 0x1f && buf[1] == 0x8b;
		}

	public:
		int encoding;

		JStreamsBuffer(BufferedInputStream* input, GZipInputStream::ZipFormat format){
		  this->input = input;

		  // check first bytes of stream before allocating buffer
		  if (format == GZipInputStream::GZIPFORMAT && !checkMagic()) {
				_CLTHROWA(CL_ERR_IO, "Magic bytes are wrong.");
		  }

		  // initialize the z_stream
		  zstream = (z_stream_s*)malloc(sizeof(z_stream_s));
		  zstream->zalloc = Z_NULL;
		  zstream->zfree = Z_NULL;
		  zstream->opaque = Z_NULL;
		  zstream->avail_in = 0;
		  zstream->next_in = Z_NULL;
		  // initialize for reading gzip streams
		  // for reading libz streams, you need inflateInit(zstream)
		  int r;
		  switch(format) {
		  case GZipInputStream::ZLIBFORMAT:
				r = inflateInit(zstream);
				break;
		  case GZipInputStream::GZIPFORMAT:
				r = inflateInit2(zstream, 15+16);
				break;
		  case GZipInputStream::ZIPFORMAT:
		  default:
				r = inflateInit2(zstream, -MAX_WBITS);
				break;
		  }
		  if (r != Z_OK) {
				dealloc();
				_CLTHROWA(CL_ERR_IO, "Error initializing GZipInputStream.");
		  }

		  // signal that we need to read into the buffer
		  zstream->avail_out = 1;
		}
		void _setMinBufSize(int32_t bufsize){
			this->setMinBufSize(bufsize);
		}

		~JStreamsBuffer(){
		  dealloc();
		}
	};

	JStreamsBuffer* jsbuffer;

	Internal(BufferedInputStream* input, GZipInputStream::ZipFormat format){
		jsbuffer = new JStreamsBuffer(input, format);
	}
	~Internal(){
		delete jsbuffer;
	}
};

GZipInputStream::GZipInputStream(InputStream* input, ZipFormat format) {
	 internal = new Internal(_CLNEW FilteredBufferedInputStream(input, false), format);
}
GZipInputStream::GZipInputStream(BufferedInputStream* input, ZipFormat format) {
	 internal = new Internal(input, format);
}

GZipInputStream::~GZipInputStream() {
    delete internal;
}


int32_t GZipInputStream::read(const signed char*& start, int32_t min, int32_t max){
	return internal->jsbuffer->read(start,min,max);
}
int32_t GZipInputStream::read(const unsigned char*& _start, int32_t min, int32_t max){
   const signed char* start = 0;
	int32_t ret = internal->jsbuffer->read(start,min,max);
   _start = (const unsigned char*)start;
   return ret;
}
int64_t GZipInputStream::position(){
	return internal->jsbuffer->position();
}
int64_t GZipInputStream::reset(int64_t to){
	return internal->jsbuffer->reset(to);
}
int64_t GZipInputStream::skip(int64_t ntoskip){
	return internal->jsbuffer->skip(ntoskip);
}
void GZipInputStream::setMinBufSize(int32_t minbufsize){
	internal->jsbuffer->_setMinBufSize(minbufsize);
}
size_t GZipInputStream::size(){
	return internal->jsbuffer->size();
}


CL_NS_END
