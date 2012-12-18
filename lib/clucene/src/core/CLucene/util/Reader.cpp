/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLStreams.h"
#include "CLucene/util/Misc.h"

#include <fcntl.h>
#ifdef _CL_HAVE_IO_H
	#include <io.h>
#endif
#ifdef _CL_HAVE_SYS_STAT_H
	#include <sys/stat.h>
#endif
#ifdef _CL_HAVE_UNISTD_H
	#include <unistd.h>
#endif
#ifdef _CL_HAVE_DIRECT_H
	#include <direct.h>
#endif
#include <errno.h>

#include "_bufferedstream.h"

CL_NS_DEF(util)

StringReader::StringReader ( const TCHAR* _value, const int32_t _length, bool copyData )
{
  this->m_size = 0;
  this->value = NULL;
  this->ownValue = true;
  this->buffer_size = 0;
	this->init(_value,_length,copyData);
}

void StringReader::init ( const TCHAR* _value, const int32_t _length, bool copyData ){
  const size_t length = ( _length < 0 ? _tcslen(_value) : _length );
	this->pos = 0;
	if ( copyData ){
    TCHAR* tmp = (TCHAR*)this->value;
    if ( tmp == NULL || !this->ownValue ){
      tmp = _CL_NEWARRAY(TCHAR, length+1);
      this->buffer_size = length;
    }else if ( length > this->buffer_size || length < (this->buffer_size/2) ){ //expand, or shrink
      tmp = (TCHAR*)realloc(tmp, sizeof(TCHAR) * (length+1));
      this->buffer_size = length;
    }
		_tcsncpy(tmp, _value, length+1);
    this->value = tmp;
	}else{
    if ( ownValue && this->value != NULL)
      _CLDELETE_LARRAY( (TCHAR*)this->value);
		this->value = _value;
    this->buffer_size = 0;
	}
  this->m_size = length;
	this->ownValue = copyData;
}

StringReader::~StringReader(){
	if ( ownValue && this->value != NULL){
		TCHAR* value = (TCHAR*) this->value;
		_CLDELETE_LARRAY(value);
		this->value = NULL;
	}
}

size_t StringReader::size(){
	return m_size;
}
int32_t StringReader::read(const TCHAR*& start, int32_t min, int32_t max){
	if ( m_size == pos )
		return -1;
	start = this->value + pos;
	int32_t r = (int32_t)cl_min(cl_max(min,max),m_size-pos);
	pos += r;
	return r;
}
int64_t StringReader::position(){
	return pos;
}
void StringReader::setMinBufSize(int32_t /*s*/){
}
int64_t StringReader::reset(int64_t pos){
	if ( pos >= 0 && pos < this->m_size )
		this->pos = pos;
	return this->pos;
}
int64_t StringReader::skip(int64_t ntoskip){
	int64_t s = cl_min(ntoskip, m_size-pos);
	this->pos += s;
	return s;
}




AStringReader::AStringReader ( char* value, const int32_t length, bool copyData )
{
	this->m_size = length;
	this->pos = 0;
	if ( copyData ){
		this->value = _CL_NEWARRAY(signed char, this->m_size);
		strncpy((char*)this->value, value, this->m_size);
	}else{
		this->value = (signed char*)value;
	}
	this->ownValue = copyData;
}

AStringReader::AStringReader ( const char* value, const int32_t length ){
	if ( length >= 0 )
		this->m_size = length;
	else
		this->m_size = strlen(value);
	this->pos = 0;
	this->value = _CL_NEWARRAY(signed char, this->m_size);
	strncpy((char*)this->value, value, this->m_size);
	this->ownValue = true;
}
AStringReader::~AStringReader(){
	if ( ownValue )
		_CLDELETE_ARRAY(this->value);
}

size_t AStringReader::size(){
	return m_size;
}
int32_t AStringReader::read(const signed char*& start, int32_t min, int32_t max){
	if ( m_size == pos )
		return -1;
	start = this->value + pos;
	int32_t r = (int32_t)cl_min(cl_max(min,max),m_size-pos);
	pos += r;
	return r;
}
int32_t AStringReader::read(const unsigned char*& start, int32_t min, int32_t max){
	if ( m_size == pos )
		return -1;
	start = (unsigned char*)(this->value + pos);
	int32_t r = (int32_t)cl_min(cl_max(min,max),m_size-pos);
	pos += r;
	return r;
}
int64_t AStringReader::position(){
	return pos;
}
void AStringReader::setMinBufSize(int32_t /*s*/){
}
int64_t AStringReader::reset(int64_t pos){
	if ( pos >= 0 && pos < this->m_size )
		this->pos = pos;
	return this->pos;
}
int64_t AStringReader::skip(int64_t ntoskip){
	int64_t s = cl_min(ntoskip, m_size-pos);
	this->pos += s;
	return s;
}

class FileInputStream::Internal{
public:
	class JStreamsBuffer: public BufferedInputStreamImpl{
		int32_t fhandle;
	protected:
		int32_t fillBuffer(signed char* start, int32_t space){
			if (fhandle == 0) return -1;
	    // read into the buffer
	    int32_t nwritten = ::_read(fhandle, start, space);

	    // check the file stream status
	    if (nwritten == -1 ) {
	        m_error = "Could not read from file";
			m_status = CL_NS(util)::Error;
			if ( fhandle > 0 ){
				::_close(fhandle);
				fhandle = 0;
			}
	        return -1;
	    }else if ( nwritten == 0 ) {
	        ::_close(fhandle);
	        fhandle = 0;
	    }
	    return nwritten;
		}
	public:
		int encoding;

		JStreamsBuffer(int32_t fhandle, int32_t buffersize){
			this->fhandle = fhandle;

			m_size = fileSize(fhandle); // no need to know the file length...

			// allocate memory in the buffer
			int32_t bufsize = (int32_t)((m_size <= buffersize) ?m_size+1 :buffersize);
			setMinBufSize(bufsize);
		}
		void _setMinBufSize(int32_t bufsize){
			this->setMinBufSize(bufsize);
		}

		~JStreamsBuffer(){
			if ( fhandle > 0 ){
				if ( ::_close(fhandle) != 0 )
					_CLTHROWA(CL_ERR_IO, "File IO Close error");
			}
		}
	};

	JStreamsBuffer* jsbuffer;

	Internal(const char* path, int32_t buffersize){
		int32_t fhandle = _cl_open(path, _O_BINARY | O_RDONLY | _O_RANDOM, _S_IREAD );

		//Check if a valid handle was retrieved
	   if (fhandle < 0){
			int err = errno;
			if ( err == ENOENT )
		    		_CLTHROWA(CL_ERR_IO, "File does not exist");
			else if ( err == EACCES )
				_CLTHROWA(CL_ERR_IO, "File Access denied");
			else if ( err == EMFILE )
				_CLTHROWA(CL_ERR_IO, "Too many open files");
			else
	    		_CLTHROWA(CL_ERR_IO, "Could not open file");
	   }
		jsbuffer = new JStreamsBuffer(fhandle, buffersize);

	}
	~Internal(){
		delete jsbuffer;
	}
};


FileInputStream::FileInputStream ( const char* path, int32_t buflen  )
{
	if ( buflen == -1 )
		buflen = DEFAULT_BUFFER_SIZE;
	_internal = new Internal(path, buflen);
}

size_t FileInputStream::size(){
	return (size_t)_internal->jsbuffer->size();
}

FileInputStream::~FileInputStream ()
{
	delete _internal;
}

int32_t FileInputStream::read(const signed char*& start, int32_t min, int32_t max){
	return _internal->jsbuffer->read(start,min,max);
}
int64_t FileInputStream::position(){
	return _internal->jsbuffer->position();
}
int64_t FileInputStream::reset(int64_t to){
	return _internal->jsbuffer->reset(to);
}
int64_t FileInputStream::skip(int64_t ntoskip){
	return _internal->jsbuffer->skip(ntoskip);
}
void FileInputStream::setMinBufSize(int32_t minbufsize){
	_internal->jsbuffer->_setMinBufSize(minbufsize);
}


FileReader::FileReader(const char *path, const char *enc, int32_t buflen)
{
	int encoding;
	if ( strcmp(enc,"ASCII")==0 )
		encoding = ASCII;
#ifdef _UCS2
	else if ( strcmp(enc,"UTF-8")==0 )
		encoding = UTF8;
	else if ( strcmp(enc,"UCS-2LE")==0 )
		encoding = UCS2_LE;
#endif
	else
		_CLTHROWA(CL_ERR_IllegalArgument,"Unsupported encoding, use jstreams iconv based instead");
	init( _CLNEW FileInputStream(path, buflen), encoding);
}
FileReader::FileReader(const char *path, int encoding, int32_t buflen)
{
	init(_CLNEW FileInputStream(path, buflen), encoding);
}
FileReader::~FileReader(){
}

class SimpleInputStreamReader::Internal{
public:

	class JStreamsBuffer: public BufferedReaderImpl{
		InputStream* input;
		char utf8buf[6]; //< buffer used for converting utf8 characters
	protected:
		int readChar(){
			const signed char* buf;
			if ( encoding == ASCII ){
				int32_t ret = this->input->read(buf, 1, 1) ;
				if ( ret == 1 ){
					return buf[0];
				}else
					return -1;

			}else if ( encoding == UCS2_LE ){
				int32_t ret = this->input->read(buf, 2, 2);
				if ( ret < 0 )
					return -1;
				else if ( ret == 1 ){
					return buf[0];
				}else{
					uint8_t c1 = *buf;
					uint8_t c2 = *(buf+1);
					return c1 | (c2<<8);
				}
			}else if ( encoding == UTF8 ){
				int32_t ret = this->input->read(buf, 1, 1);

				if ( ret == 1 ){
					int len = lucene_utf8charlen(buf[0]);
					if ( len > 1 ){
						*utf8buf = buf[0];
						ret = this->input->read(buf, len-1, len-1);
					}else
						return buf[0];

					if ( ret >= 0 ){
						if ( ret == len-1 ){
							memcpy(utf8buf+1,buf,ret);
							wchar_t wcbuf=0;
							lucene_utf8towc(wcbuf, utf8buf);
							return wcbuf;
						}
					}
				}else if ( ret == -1 )
					return -1;
				this->m_error = "Invalid multibyte sequence.";
				this->m_status = CL_NS(util)::Error;
			}else{
				this->m_error = "Unexpected encoding";
				this->m_status = CL_NS(util)::Error;
			}
			return -1;
		}
		int32_t fillBuffer(TCHAR* start, int32_t space){
			if ( input == NULL ) return -1;

			int c;
			int32_t i;
			for(i=0;i<space;i++){
				c = readChar();
				if ( c == -1 ){
					if ( this->m_status == CL_NS(util)::Ok ){
						if ( i == 0 )
							return -1;
						break;
					}
					return -1;
				}
				start[i] = c;
			}
			return i;
		}
	public:
		int encoding;

		JStreamsBuffer(InputStream* input, int encoding){
			this->input = input;
			this->encoding = encoding;
		   setMinBufSize(1024);
		}
		virtual ~JStreamsBuffer(){
			_CLDELETE(input);
		}
		void _setMinBufSize(int32_t min){
			this->setMinBufSize(min);
		}
	};

	JStreamsBuffer* jsbuffer;

	Internal(InputStream* input, int encoding){
		jsbuffer = new JStreamsBuffer(input, encoding);
	}
	~Internal(){
		delete jsbuffer;
	}
};

SimpleInputStreamReader::SimpleInputStreamReader(){
	_internal = NULL;
}
SimpleInputStreamReader::SimpleInputStreamReader(InputStream *i, int encoding){
	_internal = new Internal(i, encoding);
}
void SimpleInputStreamReader::init(InputStream *i, int encoding){
	_internal = new Internal(i, encoding);
}
SimpleInputStreamReader::~SimpleInputStreamReader(){
	delete _internal;
}

int32_t SimpleInputStreamReader::read(const TCHAR*& start, int32_t min, int32_t max){
	return _internal->jsbuffer->read(start, min, max);
}
int64_t SimpleInputStreamReader::position(){
	return _internal->jsbuffer->position();
}
int64_t SimpleInputStreamReader::reset(int64_t to){
	return _internal->jsbuffer->reset(to);
}
int64_t SimpleInputStreamReader::skip(int64_t ntoskip){
	return _internal->jsbuffer->skip(ntoskip);
}
size_t SimpleInputStreamReader::size(){
	return (size_t)_internal->jsbuffer->size();
}
void SimpleInputStreamReader::setMinBufSize(int32_t minbufsize){
	_internal->jsbuffer->_setMinBufSize(minbufsize);
}

class FilteredBufferedReader::Internal{
public:
	class JStreamsFilteredBuffer: public BufferedReaderImpl{
		Reader* input;
		bool deleteInput;
	protected:
		int32_t fillBuffer(TCHAR* start, int32_t space){
			const TCHAR* buffer;
			int32_t r = input->read(buffer, 1, space);
			if ( r > 0 )
				_tcsncpy(start, buffer, r);
			return r;
		}
	public:
		JStreamsFilteredBuffer(Reader* input, bool deleteInput){
			this->input = input;
			this->deleteInput = deleteInput;
		}
		~JStreamsFilteredBuffer(){
			if ( deleteInput )
				_CLDELETE(input);
		}
		void _setMinBufSize(int32_t min){
			this->setMinBufSize(min);
		}
	};
	JStreamsFilteredBuffer* jsbuffer;

	Internal(Reader* reader, bool deleteReader){
		this->jsbuffer = new JStreamsFilteredBuffer(reader, deleteReader);
	}
	~Internal(){
		delete jsbuffer;
	}
};
FilteredBufferedReader::FilteredBufferedReader(Reader* reader, bool deleteReader){
	_internal = new Internal(reader, deleteReader);
}
FilteredBufferedReader::~FilteredBufferedReader(){
	delete _internal;
}
int32_t FilteredBufferedReader::read(const TCHAR*& start, int32_t min, int32_t max){
	return _internal->jsbuffer->read(start,min,max);
}
int64_t FilteredBufferedReader::position(){
	return _internal->jsbuffer->position();
}
int64_t FilteredBufferedReader::reset(int64_t p){
	return _internal->jsbuffer->reset(p);
}
int64_t FilteredBufferedReader::skip(int64_t ntoskip){
	return _internal->jsbuffer->skip(ntoskip);
}
size_t FilteredBufferedReader::size(){
	return (size_t)_internal->jsbuffer->size();
}
void FilteredBufferedReader::setMinBufSize(int32_t minbufsize){
	return _internal->jsbuffer->_setMinBufSize(minbufsize);
}




class FilteredBufferedInputStream::Internal{
public:
	class JStreamsFilteredBuffer: public BufferedInputStreamImpl{
		InputStream* input;
		bool deleteInput;
	protected:
		int32_t fillBuffer(signed char* start, int32_t space){
			const signed char* buffer;
			int32_t r = input->read(buffer, 1, space);
			if ( r > 0 )
				memcpy(start, buffer, r);
			return r;
		}
	public:
		JStreamsFilteredBuffer(InputStream* input, bool deleteInput){
			this->input = input;
			this->deleteInput = deleteInput;
		}
		~JStreamsFilteredBuffer(){
			if ( deleteInput )
				_CLDELETE(input);
		}
		void _setMinBufSize(int32_t min){
			this->setMinBufSize(min);
		}
	};
	JStreamsFilteredBuffer* jsbuffer;

	Internal(InputStream* input, bool deleteInput){
		this->jsbuffer = new JStreamsFilteredBuffer(input, deleteInput);
	}
	~Internal(){
		delete jsbuffer;
	}
};
FilteredBufferedInputStream::FilteredBufferedInputStream(InputStream* input, bool deleteInput){
	_internal = new Internal(input, deleteInput);
}
FilteredBufferedInputStream::~FilteredBufferedInputStream(){
	delete _internal;
}
int32_t FilteredBufferedInputStream::read(const signed char*& start, int32_t min, int32_t max){
	return _internal->jsbuffer->read(start,min,max);
}
int64_t FilteredBufferedInputStream::position(){
	return _internal->jsbuffer->position();
}
int64_t FilteredBufferedInputStream::reset(int64_t p){
	return _internal->jsbuffer->reset(p);
}
int64_t FilteredBufferedInputStream::skip(int64_t ntoskip){
	return _internal->jsbuffer->skip(ntoskip);
}
size_t FilteredBufferedInputStream::size(){
	return (size_t)_internal->jsbuffer->size();
}
void FilteredBufferedInputStream::setMinBufSize(int32_t minbufsize){
	return _internal->jsbuffer->_setMinBufSize(minbufsize);
}

CL_NS_END
