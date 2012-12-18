/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_Payload_
#define _lucene_index_Payload_

#include "CLucene/util/Array.h"

CL_NS_DEF(index)

/**
*  A Payload is metadata that can be stored together with each occurrence 
*  of a term. This metadata is stored inline in the posting list of the
*  specific term.  
*  <p>
*  To store payloads in the index a {@link TokenStream} has to be used that
*  produces {@link Token}s containing payload data.
*  <p>
*  Use {@link TermPositions#getPayloadLength()} and {@link TermPositions#getPayload(byte[], int)}
*  to retrieve the payloads from the index.<br>
*
*/
class CLUCENE_EXPORT Payload:LUCENE_REFBASE {
protected:
  CL_NS(util)::ValueArray<uint8_t>& data;

  /** the offset within the byte array */
  int32_t offset;
  
  /** the length of the payload data */
  int32_t _length;

  bool deleteData;
  bool deleteArray;
public:

  /** Creates an empty payload and does not allocate a byte array. */
  Payload();

  /**
   * Creates a new payload with the the given array as data. 
   * A reference to the passed-in array is held, i. e. no 
   * copy is made.
   * 
   * @param data the data of this payload
   * @param length the length of the data
   * @param deleteData delete data when payload is deleted
   */
  Payload(uint8_t* data, const int32_t length, bool deleteData=false);

  /**
  * Creates a new payload with the the given array as data. 
  * A reference to the passed-in array is held, i. e. no 
  * copy is made.
  * 
  * @param data the data of this payload
  * @param deleteData delete data when payload is deleted
  */
  Payload(CL_NS(util)::ValueArray<uint8_t>& data, const int32_t offset=0, const int32_t length=-1, bool deleteData=false);

  /* Desctructor - auto-delete the data container */
  ~Payload();

  /**
  * Sets this payloads data. 
  * A reference to the passed-in array is held, i. e. no 
  * copy is made.
  * @param deleteData delete data when payload is deleted
  */
  void setData(uint8_t* data, const int32_t length, bool deleteData=false);

  /**
  * Sets this payloads data. 
  * A reference to the passed-in array is held, i. e. no 
  * copy is made.
  * @param deleteData delete data when payload is deleted
  */
  void setData(CL_NS(util)::ValueArray<uint8_t>& data, const int32_t offset=0, const int32_t length=-1, bool deleteData=false);

  /**
  * Returns a reference to the underlying byte array
  * that holds this payloads data.
  */
  const CL_NS(util)::ValueArray<uint8_t>& getData() const;

  /**
  * Returns the length of the payload data. 
  */
  int32_t length() const;
  
  /**
  * Returns the offset in the underlying byte array 
  */
  int32_t getOffset() const;

  /**
  * Returns the byte at the given index.
  */
  uint8_t byteAt(int index) const;

  /**
  * Allocates a new byte array, copies the payload data into it and returns it. Caller is responsible
  * for deleting it later.
  * @memory caller is responsible for deleting the returned array
  */
  CL_NS(util)::ValueArray<uint8_t>* toByteArray() const;

  /**
  * Copies the payload data to a byte array.
  * 
  * @param target the target byte array
  * @param targetOffset the offset in the target byte array
  */
  void copyTo(uint8_t* target, const int32_t targetLen) const;

  /**
  * Clones this payload by creating a copy of the underlying
  * byte array.
  */
  Payload* clone() const;

};

CL_NS_END
#endif
