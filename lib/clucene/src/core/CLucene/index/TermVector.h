/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_termvector_h
#define _lucene_index_termvector_h


//#include "FieldInfos.h"
#include "CLucene/util/Array.h"

CL_NS_DEF(index)

class TermPositionVector;


/** Provides access to stored term vector of
 *  a document field.  The vector consists of the name of the field, an array of the terms tha occur in the field of the
 * {@link org.apache.lucene.document.Document} and a parallel array of frequencies.  Thus, getTermFrequencies()[5] corresponds with the
 * frequency of getTerms()[5], assuming there are at least 5 terms in the Document.
 */
class CLUCENE_EXPORT TermFreqVector:LUCENE_BASE {
public:
	virtual ~TermFreqVector(){
	}

	/**
	* The Field name.
	* @return The name of the field this vector is associated with.
	*
	*/
	virtual const TCHAR* getField() = 0;

	/**
	* @return The number of terms in the term vector.
	*/
	virtual int32_t size() = 0;

	/**
	* @return An Array of term texts in ascending order.
	*/
	virtual const CL_NS(util)::ArrayBase<const TCHAR*>* getTerms() = 0;


	/** Array of term frequencies. Locations of the array correspond one to one
	*  to the terms in the array obtained from <code>getTerms</code>
	*  method. Each location in the array contains the number of times this
	*  term occurs in the document or the document field.
	*
	*  The size of the returned array is size()
	*  @memory Returning a pointer to internal data. Do not delete.
	*/
	virtual const CL_NS(util)::ArrayBase<int32_t>* getTermFrequencies() = 0;


	/** Return an index in the term numbers array returned from
	*  <code>getTerms</code> at which the term with the specified
	*  <code>term</code> appears. If this term does not appear in the array,
	*  return -1.
	*/
	virtual int32_t indexOf(const TCHAR* term) = 0;

	/** Just like <code>indexOf(int32_t)</code> but searches for a number of terms
	*  at the same time. Returns an array that has the same size as the number
	*  of terms searched for, each slot containing the result of searching for
	*  that term number.
	*
	*  @param terms array containing terms to look for
	*  @param start index in the array where the list of terms starts
	*  @param len the number of terms in the list
	*/
	virtual CL_NS(util)::ArrayBase<int32_t>* indexesOf(const CL_NS(util)::ArrayBase<TCHAR*>& terms, const int32_t start, const int32_t len) = 0;

	/** Solve the diamond inheritence problem by providing a reinterpret function.
    *	No dynamic casting is required and no RTTI data is needed to do this
    */
	virtual TermPositionVector* __asTermPositionVector()=0;
};


/**
* The TermVectorOffsetInfo class holds information pertaining to a Term in a {@link TermPositionVector}'s
* offset information.  This offset information is the character offset as set during the Analysis phase (and thus may not be the actual offset in the
* original content).
*/
struct CLUCENE_EXPORT TermVectorOffsetInfo {
private:
    int32_t startOffset;
    int32_t endOffset;
public: // TODO: Remove after TermVectorWriter has been ported;
    TermVectorOffsetInfo();
    ~TermVectorOffsetInfo();
    TermVectorOffsetInfo(int32_t startOffset, int32_t endOffset);

	/**
	* The accessor for the ending offset for the term
	* @return The offset
	*/
    int32_t getEndOffset() const;
    void setEndOffset(const int32_t _endOffset);

	/**
	* The accessor for the starting offset of the term.
	*
	* @return The offset
	*/
    int32_t getStartOffset() const;
    void setStartOffset(const int32_t _startOffset);

	/**
	* Two TermVectorOffsetInfos are equals if both the start and end offsets are the same
	* @param o The comparison Object
	* @return true if both {@link #getStartOffset()} and {@link #getEndOffset()} are the same for both objects.
	*/
    bool equals(TermVectorOffsetInfo* o);
    size_t hashCode() const;
};


/**
* Convenience declaration when creating a {@link org.apache.lucene.index.TermPositionVector} that stores only position information.
*/
extern CL_NS(util)::ObjectArray<TermVectorOffsetInfo>* TermVectorOffsetInfo_EMPTY_OFFSET_INFO;

/** Extends <code>TermFreqVector</code> to provide additional information about
 *  positions in which each of the terms is found. A TermPositionVector not necessarily
 * contains both positions and offsets, but at least one of these arrays exists.
 */
class CLUCENE_EXPORT TermPositionVector: public virtual TermFreqVector {
public:

    /** Returns an array of positions in which the term is found.
     *  Terms are identified by the index at which its number appears in the
     *  term String array obtained from the <code>indexOf</code> method.
     *  May return null if positions have not been stored.
     */
    virtual const CL_NS(util)::ArrayBase<int32_t>* getTermPositions(const size_t index) = 0;

    /**
     * Returns an array of TermVectorOffsetInfo in which the term is found.
     * May return null if offsets have not been stored.
     *
     * @see org.apache.lucene.analysis.Token
     *
     * @param index The position in the array to get the offsets from
     * @return An array of TermVectorOffsetInfo objects or the empty list
     */
     virtual const CL_NS(util)::ArrayBase<TermVectorOffsetInfo*>* getOffsets(const size_t index) = 0;

     virtual ~TermPositionVector(){
	 }
};



CL_NS_END
#endif
