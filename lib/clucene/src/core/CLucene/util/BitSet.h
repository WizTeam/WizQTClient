/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_BitSet_
#define _lucene_util_BitSet_


CL_CLASS_DEF(store,Directory)
CL_CLASS_DEF(store,IndexInput)
CL_CLASS_DEF(store,IndexOutput)

CL_NS_DEF(util)


/** Optimized implementation of a vector of bits.  This is more-or-less like
  java.util.BitSet, but also includes the following:
  <ul>
  <li>a count() method, which efficiently computes the number of one bits;</li>
  <li>optimized read from and write to disk;</li>
  <li>inlinable get() method;</li>
  <li>store and load, as bit set or d-gaps, depending on sparseness;</li> 
  </ul>
  */
class CLUCENE_EXPORT BitSet:LUCENE_BASE {
	int32_t _size;
	int32_t _count;
	uint8_t *bits;

  void readBits(CL_NS(store)::IndexInput* input);
  /** read as a d-gaps list */
  void readDgaps(CL_NS(store)::IndexInput* input);
  /** Write as a bit set */
  void writeBits(CL_NS(store)::IndexOutput* output);
  /** Write as a d-gaps list */
  void writeDgaps(CL_NS(store)::IndexOutput* output);
  /** Indicates if the bit vector is sparse and should be saved as a d-gaps list, or dense, and should be saved as a bit set. */
  bool isSparse();
  static const uint8_t BYTE_COUNTS[256];
protected:
	BitSet( const BitSet& copy );

public:
	///Create a bitset with the specified size
	BitSet ( int32_t size );
	BitSet(CL_NS(store)::Directory* d, const char* name);
	void write(CL_NS(store)::Directory* d, const char* name);
	
	///Destructor for the bit set
	~BitSet();
	
	///get the value of the specified bit
	///get the value of the specified bit
    inline bool get(const int32_t bit) const{
        if (bit >= _size) {
            _CLTHROWA(CL_ERR_IndexOutOfBounds, "bit out of range");
        }
        return (bits[bit >> 3] & (1 << (bit & 7))) != 0;
    }

    /**
    * Returns the index of the first bit that is set to {@code true}
    * that occurs on or after the specified starting index. If no such
    * bit exists then {@code -1} is returned.
    *
    * <p>To iterate over the {@code true} bits in a {@code BitSet},
    * use the following loop:
    *
    *  <pre> {@code
    * for (int i = bs.nextSetBit(0); i >= 0; i = bs.nextSetBit(i+1)) {
    *     // operate on index i here
    * }}</pre>
    *
    * @param  fromIndex the index to start checking from (inclusive)
    * @return the index of the next set bit, or {@code -1} if there
    *         is no such bit
    * @throws IndexOutOfBounds if the specified index is negative
    *
    */
    int32_t nextSetBit(int32_t fromIndex) const;
	
	///set the value of the specified bit
	void set(const int32_t bit, bool val=true);
	
	///returns the size of the bitset
	int32_t size() const;
	
	/// Returns the total number of one bits in this BitSet.  This is efficiently
	///	computed and cached, so that, if the BitSet is not changed, no
	///	recomputation is done for repeated calls. 
	int32_t count();
	BitSet *clone() const;
};
typedef BitSet BitVector; //Lucene now calls the BitSet a BitVector...

CL_NS_END
#endif
