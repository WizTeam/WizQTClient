/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Jos van den Oever
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef JSTREAM_STREAMBUFFER_H
#define JSTREAM_STREAMBUFFER_H

#include <cstdlib>
#include <cstring>

CL_NS_DEF(util)

/**
 * @internal
 * @brief Provides a buffer for the use of BufferedStream
 */
template <class T>
class StreamBuffer {
private:
public:
    /**
     * @internal
     * @brief Pointer to the start of the buffer.
     */
    T* start;
    /**
     * @internal
     * @brief Size of the buffer.
     *
     * Size of the memory pointed to by @p start,
     * in multiples of sizeof(T)
     */
    int32_t size;
    /**
     * @internal
     * @brief Pointer to the current position the buffer.
     */
    T* readPos;
    /**
     * @internal
     * @brief The amount of data available in the buffer.
     *
     * The size of the used memory in the buffer, starting
     * from @p readPos.  @p readPos + @p avail must be
     * greater than @p start + @p size.
     */
    int32_t avail;

    /**
     * @internal
     * @brief Constructor: initialises members to sane defaults.
     */
    StreamBuffer();
    /**
     * @internal
     * @brief Destructor: frees the memory used by the buffer.
     */
    ~StreamBuffer();
    /**
     * @internal
     * @brief Sets the size of the buffer, allocating the necessary memory
     *
     * @param size the size that the buffer should be, in multiples
     * of sizeof(T)
     */
    void setSize(int32_t size);
    /**
     * @internal
     * @brief Read data from the buffer
     *
     * Sets @p start to point to the data, starting
     * at the item of data following the last item
     * of data read.
     *
     * @param start pointer passed by reference. It will
     * be set to point to the data read from the buffer
     * @param max the maximum amount of data to read from
     * the buffer
     * @return the size of the data pointed to by @p start
     * (always less than or equal to @p max)
     */
    int32_t read(const T*& start, int32_t max=0);

    /**
     * @internal
     * @brief Prepares the buffer for a new write.
     *
     * This function invalidates any pointers
     * previously obtained from read.
     *
     * @return the number of available places
     **/
     int32_t makeSpace(int32_t needed);
};

template <class T>
StreamBuffer<T>::StreamBuffer() {
    readPos = start = 0;
    size = avail = 0;
}
template <class T>
StreamBuffer<T>::~StreamBuffer() {
    std::free(start);
}
template <class T>
void
StreamBuffer<T>::setSize(int32_t size) {
    // store pointer information
    int32_t offset = readPos - start;

    // allocate memory in the buffer
    start = (T*)std::realloc(start, size*sizeof(T));
    this->size = size;

    // restore pointer information
    readPos = start + offset;
}
template <class T>
int32_t
StreamBuffer<T>::makeSpace(int32_t needed) {
    // determine how much space is available for writing
    int32_t space = size - (readPos - start) - avail;
    if (space >= needed) {
        // there's enough space
        return space;
    }

    if (avail) {
        if (readPos != start) {
//            printf("moving\n");
            // move data to the start of the buffer
            std::memmove(start, readPos, avail*sizeof(T));
            space += readPos - start;
            readPos = start;
        }
    } else {
        // we may start writing at the start of the buffer
        readPos = start;
        space = size;
    }
    if (space >= needed) {
        // there's enough space now
        return space;
    }

    // still not enough space, we have to allocate more
//    printf("resize %i %i %i %i %i\n", avail, needed, space, size + needed - space, size);
    setSize(size + needed - space);
    return needed;
}
template <class T>
int32_t
StreamBuffer<T>::read(const T*& start, int32_t max) {
    start = readPos;
    if (max <= 0 || max > avail) {
        max = avail;
    }
    readPos += max;
    avail -= max;
    return max;
}

CL_NS_END

#endif
