/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_store_IndexInput_
#define _lucene_store_IndexInput_

#include "CLucene/LuceneThreads.h"
#include "CLucene/util/Equators.h"

CL_NS_DEF(store)

   /** Abstract base class for input from a file in a {@link lucene::store::Directory}.  A
   * random-access input stream.  Used for all Lucene index input operations.
   * @see Directory
   * @see IndexOutput
   */
	class CLUCENE_EXPORT IndexInput: LUCENE_BASE, public CL_NS(util)::NamedObject {
	protected:
		IndexInput();
		IndexInput(const IndexInput& clone);
	public:
	    virtual ~IndexInput();
		virtual IndexInput* clone() const =0;

		DEFINE_MUTEX(THIS_LOCK)

		/** Reads and returns a single byte.
		* @see IndexOutput#writeByte(byte)
		*/
		virtual uint8_t readByte() =0;


  /** Reads a specified number of bytes into an array at the specified offset.
   * @param b the array to read bytes into
   * @param offset the offset in the array to start storing bytes
   * @param len the number of bytes to read
   * @see IndexOutput#writeBytes(byte[],int)
   */
   virtual void readBytes(uint8_t* b, const int32_t len) = 0;

    /** Reads a specified number of bytes into an array at the
    * specified offset with control over whether the read
    * should be buffered (callers who have their own buffer
    * should pass in "false" for useBuffer).  Currently only
    * {@link BufferedIndexInput} respects this parameter.
		* @param b the array to read bytes into
		* @param offset the offset in the array to start storing bytes
		* @param len the number of bytes to read
    * @param useBuffer set to false if the caller will handle
    * buffering.
		* @see IndexOutput#writeBytes(byte[],int32_t)
		*/
		virtual void readBytes(uint8_t* b, const int32_t len, bool useBuffer);

		/** Reads four bytes and returns an int.
		* @see IndexOutput#writeInt(int32_t)
		*/
		int32_t readInt();

		/** Reads an int stored in variable-length format.  Reads between one and
		* five bytes.  Smaller values take fewer bytes.  Negative numbers are not
		* supported.
		* @see IndexOutput#writeVInt(int32_t)
		*/
		virtual int32_t readVInt();

		/** Reads eight bytes and returns a long.
		* @see IndexOutput#writeLong(long)
		*/
		int64_t readLong();

		/** Reads a long stored in variable-length format.  Reads between one and
		* nine bytes.  Smaller values take fewer bytes.  Negative numbers are not
		* supported. */
		int64_t readVLong();

		/** Reads a string
		* @see IndexOutput#writeString(String)
		* maxLength is the amount read into the buffer, the whole string is still read from the stream
		* returns the amount read
		*/
		int32_t readString(TCHAR* buffer, const int32_t maxlength);

		#ifdef _UCS2
		/** Reads a string and converts to ascii.
		* @see IndexOutput#writeString(String)
		* maxLength is the amount read into the buffer, the whole string is still read from the stream
		* returns the amount read
		*/
		int32_t readString(char* buffer, const int32_t maxlength);
		#endif

		/** Reads a string.
		* @see IndexOutput#writeString(String)
		*/
		TCHAR* readString();

		/** Reads UTF-8 encoded characters into an array.
		* @param buffer the array to read characters into
		* @param start the offset in the array to start storing characters
		* @param length the number of characters to read
		* @see IndexOutput#writeChars(String,int32_t,int32_t)
		*/
		void readChars( TCHAR* buffer, const int32_t start, const int32_t len);

		void skipChars( const int32_t count);

		/** Closes the stream to futher operations. */
		virtual void close() =0;

		/** Returns the current position in this file, where the next read will
		* occur.
		* @see #seek(int64_t)
		*/
		virtual int64_t getFilePointer() const =0;

		/** Sets current position in this file, where the next read will occur.
		* @see #getFilePointer()
		*/
		virtual void seek(const int64_t pos) =0;

		/** The number of bytes in the file. */
		virtual int64_t length() const = 0;

		virtual const char* getDirectoryType() const = 0;
		virtual const char* getObjectName() const = 0;
	};

   /** Abstract base class for input from a file in a {@link Directory}.  A
   * random-access input stream.  Used for all Lucene index input operations.
   * @see Directory
   * @see IndexOutput
   */
	class CLUCENE_EXPORT BufferedIndexInput: public IndexInput{
	private:
		uint8_t* buffer; //array of bytes
		void refill();
	protected:
		int32_t bufferSize;				//size of the buffer
		int64_t bufferStart;			  // position in file of buffer
		int32_t bufferLength;			  // end of valid l_byte_ts
		int32_t bufferPosition;		  // next uint8_t to read

      /** Returns a clone of this stream.
      *
      * <p>Clones of a stream access the same data, and are positioned at the same
      * point as the stream they were cloned from.
      *
      * <p>Expert: Subclasses must ensure that clones may be positioned at
      * different points in the input from each other and from the stream they
      * were cloned from.
      */
		BufferedIndexInput(const BufferedIndexInput& clone);
		BufferedIndexInput(int32_t bufferSize = -1);
	public:
		LUCENE_STATIC_CONSTANT(int32_t, BUFFER_SIZE=LUCENE_STREAM_BUFFER_SIZE);

		virtual ~BufferedIndexInput();
		virtual IndexInput* clone() const = 0;
		void close();
		inline uint8_t readByte(){
			if (bufferPosition >= bufferLength)
				refill();

			return buffer[bufferPosition++];
		}
		void readBytes(uint8_t* b, const int32_t len);
		void readBytes(uint8_t* b, const int32_t len, bool useBuffer);
		int64_t getFilePointer() const;
		void seek(const int64_t pos);

		void setBufferSize( int32_t newSize );

		const char* getObjectName();
		static const char* getClassName();

	protected:
      /** Expert: implements buffer refill.  Reads bytes from the current position
      * in the input.
      * @param b the array to read bytes into
      * @param offset the offset in the array to start storing bytes
      * @param length the number of bytes to read
      */
		virtual void readInternal(uint8_t* b, const int32_t len) = 0;

      /** Expert: implements seek.  Sets current position in this file, where the
      * next {@link #readInternal(byte[],int32_t,int32_t)} will occur.
      * @see #readInternal(byte[],int32_t,int32_t)
      */
		virtual void seekInternal(const int64_t pos) = 0;
	};
CL_NS_END
#endif
