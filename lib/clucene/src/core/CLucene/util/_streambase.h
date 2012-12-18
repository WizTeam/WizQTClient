/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef JSTREAMS_STREAMBASE_H
#define JSTREAMS_STREAMBASE_H

#include <stdio.h>
#include <string>

#define INT32MAX 0x7FFFFFFFL

CL_NS_DEF(util)

/** Used to indicate the current status of a Stream */
enum StreamStatus {
    Ok /**< Stream is capable of being read from */,
    Eof /**< The end of the Stream has been reached */,
    Error /**< An error occurred. Use error() to find out more information */
};

// java mapping: long=int64, int=int32, byte=uint8_t
/**
 * The base of all Streams. Do not inherit directly from this class,
 * but from (an instance of) StreamBase
 *
 * This class contains all the non-virtual StreamBase methods
 * that don't depend on a specific Stream type
 *
 * Developer comment: This is needed because of win32 compilation.
 * When we want to access a function outside a lib, we have to export them,
 * but we can't export the template class because this would be somewhat
 * stupid / does not work by design :)
 * Because of this I've introduced this StreamBaseBase class
 */
class StreamBaseBase { //krazy:exclude=dpointer
protected:
    /** The size of the stream (-1 if unknown) */
    int64_t m_size;
    /** The position of the stream */
    int64_t m_position;
    /**
     * @brief String representation of the last error, or
     * an empty string otherwise
     */
    std::string m_error;
    /** The status of the stream - see StreamStatus */
    StreamStatus m_status;
public:
    /**
     * @brief  Constructor: initialises everything to sane defaults
     **/
    StreamBaseBase() :m_size(-1), m_position(0), m_status(Ok) {}
    /**
     * @brief Destructor
     **/
    virtual ~StreamBaseBase() {}
    /**
     * @brief  Return a string representation of the last error.
     * If no error has occurred, an empty string is returned.
     **/
    const char* error() const { return m_error.c_str(); }
    /**
     * @brief  Return the status of the stream.
     **/
    StreamStatus status() const { return m_status; }
    /**
     * @brief Get the current position in the stream.
     * The value obtained from this function can be used to reset the stream.
     **/
    int64_t position() const { return m_position; }
    /**
     * @brief Return the size of the stream.
     *
     * The size of the stream is always known if the end of the stream
     * has been reached.  In all other cases, this may return -1 to
     * indicate the size of the stream is unknown.
     *
     * @return the size of the stream, if it is known, or -1 if the size
     * of the stream is unknown
     **/
    int64_t size() const { return m_size; }
};

/**
 * @brief Base class for stream read access to a data source.
 *
 * This class is based on the interface java.io.InputStream. It provides
 * a uniform interface for accessing streamed resources.
 *
 * The main difference with the Java equivalent is a performance improvement.
 * When reading data, data is not copied into a buffer provided by the caller,
 * but a pointer to the read data is provided. This makes this interface
 * especially useful for deriving from it and implementing filters or
 * transformers.
 */
template <class T>
class StreamBase : public StreamBaseBase {
public:
    StreamBase() { }
    virtual ~StreamBase(){}
    /**
     * @brief Reads items from the stream and sets @p start to point to
     * the first item that was read.
     *
     * Note: unless stated otherwise in the documentation for that method,
     * this pointer will no longer be valid after calling another method of
     * this class. The pointer will also no longer be valid after the class
     * is destroyed.
     *
     * The functions inherited from StreamBaseBase do not invalidate the pointer.
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
     *         end of the stream has already been reached. @c -2 is returned
     *         if an error has occurred
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
    virtual int64_t skip(int64_t ntoskip);
    /**
     * @brief Repositions this stream to a given position.
     *
     * A call to StreamBase::reset is only guaranteed to be successful when
     * the requested position lies within the segment of a stream
     * corresponding to a valid pointer obtained from StreamBase::read.
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
    virtual int64_t reset(int64_t pos) = 0;
};


template <class T>
int64_t
StreamBase<T>::skip(int64_t ntoskip) {
    const T* begin;
    int32_t nread;
    int64_t skipped = 0;
    while (ntoskip > 0) {
        // make sure we do not overflow uint32_t
        int32_t maxstep = (int32_t)((ntoskip > 10000000)
                       ?10000000 :ntoskip);
        // the default implementation is to simply read the data that we want
        // to skip
        nread = read(begin, 1, maxstep);
        if (nread < -1 ) {
            // an error occurred
            return nread;
        } else if (nread < 1) {
            // the end of the stream was encountered
            ntoskip = 0;
        } else {
            skipped += nread;
            ntoskip -= nread;
        }
    }
    return skipped;
}

CL_NS_END

#endif
