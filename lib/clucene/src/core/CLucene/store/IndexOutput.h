/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_IndexOutput_
#define _lucene_store_IndexOutput_

CL_NS_DEF(store)

class IndexInput;

/** Abstract class for output to a file in a Directory.  A random-access output
* stream.  Used for all Lucene index output operations.
* @see Directory
* @see IndexInput
*/
class CLUCENE_EXPORT IndexOutput:LUCENE_BASE{
	bool isclosed;
public:
	IndexOutput();
	virtual ~IndexOutput();

	/** Writes a single byte.
	* @see IndexInput#readByte()
	*/
	virtual void writeByte(const uint8_t b) = 0;

	/** Writes an array of bytes.
	* @param b the bytes to write
	* @param length the number of bytes to write
	* @see IndexInput#readBytes(uint8_t*,int32_t)
	*/
	virtual void writeBytes(const uint8_t* b, const int32_t length) = 0;

	/** Writes an int as four bytes.
	* @see IndexInput#readInt()
	*/
	void writeInt(const int32_t i);

	/** Writes an int in a variable-length format.  Writes between one and
	* five bytes.  Smaller values take fewer bytes.  Negative numbers are not
	* supported.
	* @see IndexInput#readVInt()
	*/
	void writeVInt(const int32_t vi);

	/** Writes a long as eight bytes.
	* @see IndexInput#readLong()
	*/
	void writeLong(const int64_t i);

	/** Writes an long in a variable-length format.  Writes between one and five
	* bytes.  Smaller values take fewer bytes.  Negative numbers are not
	* supported.
	* @see IndexInput#readVLong()
	*/
	void writeVLong(const int64_t vi);

	/** Writes a string.
	* @see IndexInput#readString()
	*/
	void writeString(const TCHAR* s, const int32_t length);
    void writeString(const std::string& s);
	
	#ifdef _UCS2
	/** Writes an ascii string. converts to TCHAR* before writing
	* @see IndexInput#readString()
	*/
	void writeString(const char* s, const int32_t length);
	#endif

	/** Writes a sequence of UTF-8 encoded characters from a string.
	* @param s the source of the characters
	* @param start the first character in the sequence
	* @param length the number of characters in the sequence
	* @see IndexInput#readChars(char[],int32_t,int32_t)
	*/
	void writeChars(const TCHAR* s, const int32_t length);

	/** Closes this stream to further operations. */
	virtual void close() = 0;

	/** Returns the current position in this file, where the next write will
	* occur.
	* @see #seek(long)
	*/
	virtual int64_t getFilePointer() const = 0;

	/** Sets current position in this file, where the next write will occur.
	* @see #getFilePointer()
	*/
	virtual void seek(const int64_t pos) = 0;

	/** The number of bytes in the file. */
	virtual int64_t length() const = 0;

	/** Forces any buffered output to be written. */
	virtual void flush() = 0;

private:
	LUCENE_STATIC_CONSTANT(int32_t, COPY_BUFFER_SIZE = 16384);
	uint8_t* copyBuffer;

public:
	/** Copy numBytes bytes from input to ourself. */
	void copyBytes(CL_NS(store)::IndexInput* input, int64_t numBytes);
};

/** Base implementation class for buffered {@link IndexOutput}. */
class CLUCENE_EXPORT BufferedIndexOutput : public IndexOutput{
public:
	LUCENE_STATIC_CONSTANT(int32_t, BUFFER_SIZE=16384);
private:
	uint8_t* buffer;
	int64_t bufferStart;			  // position in file of buffer
	int32_t bufferPosition;		  // position in buffer

public:
	BufferedIndexOutput();
	virtual ~BufferedIndexOutput();

	/** Writes a single byte.
	* @see IndexInput#readByte()
	*/
	virtual void writeByte(const uint8_t b);

	/** Writes an array of bytes.
	* @param b the bytes to write
	* @param length the number of bytes to write
	* @see IndexInput#readBytes(byte[],int32_t,int32_t)
	*/
	virtual void writeBytes(const uint8_t* b, const int32_t length);

	/** Closes this stream to further operations. */
	virtual void close();

	/** Returns the current position in this file, where the next write will
	* occur.
	* @see #seek(long)
	*/
	int64_t getFilePointer() const;

	/** Sets current position in this file, where the next write will occur.
	* @see #getFilePointer()
	*/
	virtual void seek(const int64_t pos);

	/** The number of bytes in the file. */
	virtual int64_t length() const = 0;

	/** Forces any buffered output to be written. */
	void flush();

protected:
	/** Expert: implements buffer write.  Writes bytes at the current position in
	* the output.
	* @param b the bytes to write
	* @param len the number of bytes to write
	*/
	virtual void flushBuffer(const uint8_t* b, const int32_t len) = 0;
};

CL_NS_END
#endif
