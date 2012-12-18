/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_Terms_
#define _lucene_index_Terms_

#include "CLucene/util/Equators.h"
CL_NS_DEF(index)

//predefine
class Term;
class TermEnum;
class TermPositions;

/** TermDocs provides an interface for enumerating &lt;document, frequency&gt;
 pairs for a term.  <p> The document portion names each document containing
 the term.  Documents are indicated by number.  The frequency portion gives
 the number of times the term occurred in each document.  <p> The pairs are
 ordered by document number.

 @see IndexReader#termDocs()
 */
class CLUCENE_EXPORT TermDocs {
public:
	virtual ~TermDocs();

	// Sets this to the data for a term.
	// The enumeration is reset to the start of the data for this term.
	virtual void seek(Term* term)=0;

	/** Sets this to the data for the current term in a {@link TermEnum}.
	* This may be optimized in some implementations.
	*/
	virtual void seek(TermEnum* termEnum)=0;

	// Returns the current document number.  <p> This is invalid until {@link
	//	#next()} is called for the first time.
	virtual int32_t doc() const=0;

	// Returns the frequency of the term within the current document.  <p> This
	//	is invalid until {@link #next()} is called for the first time.
	virtual int32_t freq() const=0;

	// Moves to the next pair in the enumeration.  <p> Returns true iff there is
	//	such a next pair in the enumeration.
	virtual bool next() =0;

	// Attempts to read multiple entries from the enumeration, up to length of
	// <i>docs</i>.  Document numbers are stored in <i>docs</i>, and term
	// frequencies are stored in <i>freqs</i>.  The <i>freqs</i> array must be as
	// int64_t as the <i>docs</i> array.
	//
	// <p>Returns the number of entries read.  Zero is only returned when the
	// stream has been exhausted.
	virtual int32_t read(int32_t* docs, int32_t* freqs, int32_t length)=0;

	// Skips entries to the first beyond the current whose document number is
	// greater than or equal to <i>target</i>. <p>Returns true iff there is such
	// an entry.  <p>Behaves as if written: <pre>
	//   bool skipTo(int32_t target) {
	//     do {
	//       if (!next())
	// 	     return false;
	//     } while (target > doc());
	//     return true;
	//   }
	// </pre>
	// Some implementations are considerably more efficient than that.
	virtual bool skipTo(const int32_t target)=0;

	// Frees associated resources.
	virtual void close() = 0;

	
	/** Solve the diamond inheritence problem by providing a reinterpret function.
    *	No dynamic casting is required and no RTTI data is needed to do this
    */
	virtual TermPositions* __asTermPositions()=0;
};


/** Abstract class for enumerating terms.

  <p>Term enumerations are always ordered by Term.compareTo().  Each term in
  the enumeration is greater than all that precede it.
*/
class CLUCENE_EXPORT TermEnum: LUCENE_BASE, public CL_NS(util)::NamedObject {
public:
	/** Increments the enumeration to the next element.  True if one exists.*/ 
	virtual bool next()=0;

	/**
	* Returns the current Term in the enumeration.
	* @param pointer if true, then increment the reference count before returning
	*/
	virtual Term* term(bool pointer=true)=0;

	/** Returns the docFreq of the current Term in the enumeration.*/
	virtual int32_t docFreq() const=0;

	/** Closes the enumeration to further activity, freeing resources. */
	virtual void close() =0;

	virtual ~TermEnum();
	
	// Term Vector support
	/** Skips terms to the first beyond the current whose value is
	* greater or equal to <i>target</i>. <p>Returns true iff there is such
	* an entry.  <p>Behaves as if written: <pre>
	*   public boolean skipTo(Term target) {
	*     do {
	*       if (!next())
	* 	     return false;
	*     } while (target > term());
	*     return true;
	*   }
	* </pre>
	* Some implementations are considerably more efficient than that.
	*/
	virtual bool skipTo(Term* target);
};



/**
 * TermPositions provides an interface for enumerating the &lt;document,
 * frequency, &lt;position&gt;* &gt; tuples for a term.  <p> The document and
 * frequency are the same as for a TermDocs.  The positions portion lists the ordinal
 * positions of each occurrence of a term in a document.
 *
 * @see IndexReader#termPositions()
 */
class CLUCENE_EXPORT TermPositions: public virtual TermDocs {
public:
    /** Returns next position in the current document.  It is an error to call
    this more than {@link #freq()} times
    without calling {@link #next()}<p> This is
    invalid until {@link #next()} is called for
    the first time.
    */
	virtual int32_t nextPosition() = 0;

	virtual ~TermPositions();

    /** 
     * Returns the length of the payload at the current term position.
     * This is invalid until {@link #nextPosition()} is called for
     * the first time.<br>
     * @return length of the current payload in number of bytes
     */
    virtual int32_t getPayloadLength() const = 0;
    
    /** 
     * Returns the payload data at the current term position.
     * This is invalid until {@link #nextPosition()} is called for
     * the first time.
     * This method must not be called more than once after each call
     * of {@link #nextPosition()}. However, payloads are loaded lazily,
     * so if the payload data for the current position is not needed,
     * this method may not be called at all for performance reasons.<br>
     * 
     * @param data the array into which the data of this payload is to be
     *             stored, if it is big enough; otherwise, a new byte[] array
     *             is allocated for this purpose. 
     * @return a byte[] array containing the data of this payload
     */
    virtual uint8_t* getPayload(uint8_t* data) = 0;

	/**
	* Checks if a payload can be loaded at this position.
	* <p>
	* Payloads can only be loaded once per call to 
	* {@link #nextPosition()}.
	* 
	* @return true if there is a payload available at this position that can be loaded
	*/
	virtual bool isPayloadAvailable() const = 0;

	/** Solve the diamond inheritence problem by providing a reinterpret function.
	  *	No dynamic casting is required and no RTTI data is needed to do this
	  */
	virtual TermDocs* __asTermDocs()=0;
	virtual TermPositions* __asTermPositions()=0;
};
CL_NS_END
#endif
