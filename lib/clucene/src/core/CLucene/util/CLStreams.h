/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_CLStreams_
#define _lucene_util_CLStreams_

CL_NS_DEF(util)

template <typename T>
class CLUCENE_EXPORT CLStream{
public:
	virtual ~CLStream(){}

	inline int read(){
		const T* buffer;
		const int32_t nread = read(buffer,1, 1);
		if ( nread < 0 )
			return -1;
		else
			return buffer[0];
	}

	/** Read one line, return the length of the line read.
	* If the string is longer than len, only len of that line will be copied
	*/
	inline int32_t readLine(T* buffer, size_t len){
		size_t i = 0;
		while (true && i<len-1) {
			int32_t b = read();
			if (b < 1)
				break;
			if (b == '\n' || b == '\r') {
				if (i > 0)
					break;
				else
					continue;
			}
			buffer[i++] = b;
		}
		buffer[i] = 0;
		return i;
	}
	
    /**
     * @brief Reads items from the stream and sets @p start to point to
     * the first item that was read.
     *
     * Note: unless stated otherwise in the documentation for that method,
     * this pointer will no longer be valid after calling another method of
     * this class. The pointer will also no longer be valid after the class
     * is destroyed.
     *
     * At least @p min items will be read from the stream, unless an error occurs
     * or the end of the stream is reached.  Under no circumstances will more than
     * @p max items be read.
     *
     * If the end of the stream is reached before @p min items are read, the
     * read is still considered successful and the number of items read will
     * be returned.
     *
     * @param start pointer passed by reference that will be set to point to
     *              the retrieved array of items. If the end of the stream
     *              is encountered or an error occurs, the value of @p start
     *              is undefined
     * @param min   the minimal number of items to read from the stream. This
     *              value should be larger than 0. If it is 0 or smaller, the
     *              result is undefined
     * @param max   the maximal number of items to read from the stream.
     *              If this value is smaller than @p min, there is no limit on
     *              the number of items that can be read
     * @return the number of items that were read. @c -1 is returned if
     *         end of the stream has already been reached. An error is thrown
	 *			if an error occurs.
     **/
	virtual int32_t read(const T*& start, int32_t min, int32_t max) = 0;
    /**
     * @brief Skip @p ntoskip items.
     *
     * If an error occurs, or the end of the stream is encountered, fewer
     * than @p ntoskip items may be skipped.  This can be checked by comparing
     * the return value to @p ntoskip.
     *
     * Calling this function invalidates the data pointer that was obtained from
     * StreamBase::read.
     *
     * @param ntoskip the number of items that should be skipped
     * @return the number of items skipped
     **/
	virtual int64_t skip(int64_t ntoskip) = 0;
    /**
     * @brief Get the current position in the stream.
     * The value obtained from this function can be used to reset the stream.
     **/
	virtual int64_t position() = 0;
	int64_t getPosition(){ return this->position(); }

	virtual size_t size() = 0;
};

template <class T> 
class CLUCENE_EXPORT BufferedStream{
public:
		virtual ~BufferedStream(){}
    /**
     * @brief Repositions this stream to a given position.
     *
     * A call to reset is only guaranteed to be successful when
     * the requested position lies within the segment of a stream
     * corresponding to a valid pointer obtained from read.
     * In this case, the pointer will not be invalidated.
     *
     * Calling this function invalidates the data pointer that was obtained from
     * StreamBase::read unless the conditions outlined above apply.
     *
     * To read n items, leaving the stream at the same position as before, you
     * can do the following:
     * @code
     * int64_t start = stream.position();
     * if ( stream.read(data, min, max) > 0 ) {
     *     stream.reset(start);
     *     // The data pointer is still valid here
     * }
     * @endcode
     *
     * @param pos the position in the stream you want to go to, relative to
     * the start of the stream
     * @return the new position in the stream
     **/
    virtual int64_t reset(int64_t) = 0;
    /**
     * @brief Sets the minimum size of the buffer
     */
	virtual void setMinBufSize(int32_t s) = 0;
};

class BufferedReader;
class CLUCENE_EXPORT Reader: public CLStream<TCHAR>{
public:
	~Reader(){}
	virtual BufferedReader* __asBufferedReader(){ return NULL; }
};
class CLUCENE_EXPORT BufferedReader: public Reader, public BufferedStream<TCHAR>{
public:
	_CL_DEPRECATED( setMinBufSize ) int64_t mark(int32_t readAheadlimit){
		this->setMinBufSize(readAheadlimit);
		return this->position();
	}
	~BufferedReader(){}
	BufferedReader* __asBufferedReader(){ return this; }
};
typedef CLStream<signed char> InputStream;
class CLUCENE_EXPORT BufferedInputStream: public InputStream, public BufferedStream<signed char>{
public:
	virtual ~BufferedInputStream(){}
};
	

class CLUCENE_EXPORT FilteredBufferedReader: public BufferedReader{
	class Internal;
	Internal* _internal;
public:
	FilteredBufferedReader(Reader* reader, bool deleteReader);
	virtual ~FilteredBufferedReader();
	
	int32_t read(const TCHAR*& start, int32_t min, int32_t max);
	int64_t position();
	int64_t reset(int64_t);
	int64_t skip(int64_t ntoskip);
	size_t size();
	void setMinBufSize(int32_t minbufsize);
};

class CLUCENE_EXPORT FilteredBufferedInputStream: public BufferedInputStream{
	class Internal;
	Internal* _internal;
public:
	FilteredBufferedInputStream(InputStream* input, bool deleteInput);
	virtual ~FilteredBufferedInputStream();
	
	int32_t read(const signed char*& start, int32_t min, int32_t max);
	int64_t position();
	int64_t reset(int64_t);
	int64_t skip(int64_t ntoskip);
	size_t size();
	void setMinBufSize(int32_t minbufsize);
};


class CLUCENE_EXPORT StringReader: public BufferedReader{
protected:
	const TCHAR* value;
	bool ownValue;
	int64_t pos;
	size_t m_size;
  size_t buffer_size;
public:
  StringReader ( const TCHAR* value, const int32_t length = -1, bool copyData = true );
  void init ( const TCHAR* value, const int32_t length, bool copyData = true );
	virtual ~StringReader();

  int32_t read(const TCHAR*& start, int32_t min, int32_t max);
  int64_t position();
  int64_t reset(int64_t);
	int64_t skip(int64_t ntoskip);
	void setMinBufSize(int32_t s);
	size_t size();
};
class CLUCENE_EXPORT AStringReader: public BufferedInputStream{
	signed char* value;
	bool ownValue;
	int64_t pos;
protected:
	size_t m_size;
public:
    AStringReader ( const char* value, const int32_t length = -1 );
    AStringReader ( char* value, const int32_t length, bool copyData = true );
	 virtual ~AStringReader();
	
    int32_t read(const signed char*& start, int32_t min, int32_t max);
    int32_t read(const unsigned char*& start, int32_t min, int32_t max);
    int64_t position();
    int64_t reset(int64_t);
	int64_t skip(int64_t ntoskip);
	void setMinBufSize(int32_t s);
	size_t size();
};

/**
* A helper class which constructs a FileReader with a specified
* simple encodings, or a given inputstreamreader
*/
class CLUCENE_EXPORT FileInputStream: public BufferedInputStream {
	class Internal;
	Internal* _internal;
protected:
	void init(InputStream *i, int encoding);
public:
	LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_BUFFER_SIZE=4096);
	FileInputStream ( const char* path, int32_t buflen = -1 );
	virtual ~FileInputStream ();
	
	int32_t read(const signed char*& start, int32_t min, int32_t max);
	int64_t position();
	int64_t reset(int64_t);
	int64_t skip(int64_t ntoskip);
	size_t size();
	void setMinBufSize(int32_t minbufsize);
};

class CLUCENE_EXPORT SimpleInputStreamReader: public BufferedReader{
	class Internal;
	Internal* _internal;
protected:
	void init(InputStream *i, int encoding);
public:	
	enum{
		ASCII=1,
		UTF8=2,
		UCS2_LE=3
	};
	
	SimpleInputStreamReader();
   SimpleInputStreamReader(InputStream *i, int encoding);
	virtual ~SimpleInputStreamReader();
	
  int32_t read(const TCHAR*& start, int32_t min, int32_t max);
  int64_t position();
  int64_t reset(int64_t);
  int64_t skip(int64_t ntoskip);
  void setMinBufSize(int32_t s);
  size_t size();
};

/**
* A helper class which constructs a FileReader with a specified
* simple encodings, or a given inputstreamreader.
* It is recommended that you use the contribs package for proper 
* decoding using iconv. This class is provided only as a dependency-less
* replacement.
*/
class CLUCENE_EXPORT FileReader: public SimpleInputStreamReader{
public:
	FileReader(const char* path, int encoding, int32_t buflen = -1);
	FileReader(const char* path, const char* encoding, int32_t buflen = -1);
	virtual ~FileReader();
};

CL_NS_END

#define jstreams CL_NS(util)

#endif
