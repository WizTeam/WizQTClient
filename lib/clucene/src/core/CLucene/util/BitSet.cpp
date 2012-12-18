/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "BitSet.h"
#include "CLucene/store/Directory.h"
#include "CLucene/store/IndexInput.h"
#include "CLucene/store/IndexOutput.h"

CL_NS_USE(store)
CL_NS_DEF(util)


const uint8_t BitSet::BYTE_COUNTS[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

BitSet::BitSet( const BitSet& copy ) :
	_size( copy._size ),
	_count(-1)
{
	int32_t len = (_size >> 3) + 1;
	bits = _CL_NEWARRAY(uint8_t, len);
	memcpy( bits, copy.bits, len );
}

BitSet::BitSet ( int32_t size ):
  _size(size),
  _count(-1)
{
	int32_t len = (_size >> 3) + 1;
	bits = _CL_NEWARRAY(uint8_t, len);
	memset(bits,0,len);
}

BitSet::BitSet(CL_NS(store)::Directory* d, const char* name)
{
	_count=-1;
	CL_NS(store)::IndexInput* input = d->openInput( name );
	try {
		 _size = input->readInt();			  // read size
     if (_size == -1) {
      readDgaps(input);
     } else {
      readBits(input);
     }
	} _CLFINALLY (
	    input->close();
	    _CLDELETE(input );
	);
}
	
void BitSet::write(CL_NS(store)::Directory* d, const char* name) {
	CL_NS(store)::IndexOutput* output = d->createOutput(name);
	try {
    if (isSparse()) {
      writeDgaps(output); // sparse bit-set more efficiently saved as d-gaps.
    } else {
      writeBits(output);
    }
	} _CLFINALLY (
	    output->close();
	    _CLDELETE(output);
	);
}
BitSet::~BitSet(){
	_CLDELETE_ARRAY(bits);
}


void BitSet::set(const int32_t bit, bool val){
    if (bit >= _size) {
	      _CLTHROWA(CL_ERR_IndexOutOfBounds, "bit out of range");
    }

	_count = -1;

	if (val)
		bits[bit >> 3] |= 1 << (bit & 7);
	else
		bits[bit >> 3] &= ~(1 << (bit & 7));
}

int32_t BitSet::size() const {
  return _size;
}
int32_t BitSet::count(){
	// if the BitSet has been modified
    if (_count == -1) {

      int32_t c = 0;
      int32_t end = (_size >> 3) + 1;
      for (int32_t i = 0; i < end; i++)
        c += BYTE_COUNTS[bits[i]];	  // sum bits per uint8_t
      _count = c;
    }
    return _count;
}
BitSet* BitSet::clone() const {
	return _CLNEW BitSet( *this );
}

  /** Read as a bit set */
  void BitSet::readBits(IndexInput* input) {
    _count = input->readInt();        // read count
    bits = _CL_NEWARRAY(uint8_t,(_size >> 3) + 1);      // allocate bits
    input->readBytes(bits, (_size >> 3) + 1);   // read bits
  }

  /** read as a d-gaps list */
  void BitSet::readDgaps(IndexInput* input) {
    _size = input->readInt();       // (re)read size
    _count = input->readInt();        // read count
    bits = _CL_NEWARRAY(uint8_t,(_size >> 3) + 1);     // allocate bits
    int32_t last=0;
    int32_t n = count();
    while (n>0) {
      last += input->readVInt();
      bits[last] = input->readByte();
      n -= BYTE_COUNTS[bits[last] & 0xFF];
    }
  }

  /** Write as a bit set */
   void BitSet::writeBits(IndexOutput* output) {
    output->writeInt(size());       // write size
    output->writeInt(count());        // write count
    output->writeBytes(bits, (_size >> 3) + 1);   // write bits
  }

  /** Write as a d-gaps list */
  void BitSet::writeDgaps(IndexOutput* output) {
    output->writeInt(-1);            // mark using d-gaps
    output->writeInt(size());        // write size
    output->writeInt(count());       // write count
    int32_t last=0;
    int32_t n = count();
    int32_t m = (_size >> 3) + 1;
    for (int32_t i=0; i<m && n>0; i++) {
      if (bits[i]!=0) {
        output->writeVInt(i-last);
        output->writeByte(bits[i]);
        last = i;
        n -= BYTE_COUNTS[bits[i] & 0xFF];
      }
    }
  }

  /** Indicates if the bit vector is sparse and should be saved as a d-gaps list, or dense, and should be saved as a bit set. */
  bool BitSet::isSparse() {
    // note: order of comparisons below set to favor smaller values (no binary range search.)
    // note: adding 4 because we start with ((int) -1) to indicate d-gaps format.
    // note: we write the d-gap for the byte number, and the byte (bits[i]) itself, therefore
    //       multiplying count by (8+8) or (8+16) or (8+24) etc.:
    //       - first 8 for writing bits[i] (1 byte vs. 1 bit), and
    //       - second part for writing the byte-number d-gap as vint.
    // note: factor is for read/write of byte-arrays being faster than vints.
    int32_t factor = 10;
    if ((_size >> 3) < (1<< 7)) return factor * (4 + (8+ 8)*count()) < size();
    if ((_size >> 3) < (1<<14)) return factor * (4 + (8+16)*count()) < size();
    if ((_size >> 3) < (1<<21)) return factor * (4 + (8+24)*count()) < size();
    if ((_size >> 3) < (1<<28)) return factor * (4 + (8+32)*count()) < size();
    return                            factor * (4 + (8+40)*count()) < size();
  }

  int32_t BitSet::nextSetBit(int32_t fromIndex) const {
      if (fromIndex < 0)
          _CLTHROWT(CL_ERR_IndexOutOfBounds, _T("fromIndex < 0"));

      if (fromIndex >= _size)
          return -1;

      while (true) {
          if ((bits[fromIndex >> 3] & (1 << (fromIndex & 7))) != 0)
              return fromIndex;
          if (++fromIndex == _size)
              return -1;
      }
  }

CL_NS_END
