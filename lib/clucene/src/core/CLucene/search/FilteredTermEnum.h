/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_FilteredTermEnum_
#define _lucene_search_FilteredTermEnum_


CL_CLASS_DEF(index,Term)
#include "CLucene/index/Terms.h"

CL_NS_DEF(search)
/** Abstract class for enumerating a subset of all terms. 

<p>Term enumerations are always ordered by Term.compareTo().  Each term in
the enumeration is greater than all that precede it.  */
class CLUCENE_EXPORT FilteredTermEnum: public CL_NS(index)::TermEnum {
public:
	FilteredTermEnum();
	virtual ~FilteredTermEnum();

	/** Equality measure on the term */
	virtual float_t difference() = 0;

	/** 
	* Returns the docFreq of the current Term in the enumeration.
	* Returns -1 if no Term matches or all terms have been enumerated.
	*/
	int32_t docFreq() const;

	/** Increments the enumeration to the next element.  True if one exists. */
	bool next() ;

	/** Returns the current Term in the enumeration.
	* Returns null if no Term matches or all terms have been enumerated. */
	CL_NS(index)::Term* term(bool pointer=true);

	/** Closes the enumeration to further activity, freeing resources.  */
	void close();

protected:
	/** Equality compare on the term */
	virtual bool termCompare(CL_NS(index)::Term* term) = 0;

	/** Indicates the end of the enumeration has been reached */
	virtual bool endEnum() = 0;

	void setEnum(CL_NS(index)::TermEnum* actualEnum) ;

private:
	CL_NS(index)::Term* currentTerm;
	CL_NS(index)::TermEnum* actualEnum;

};
CL_NS_END
#endif
