/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_document_NumberTools_
#define _lucene_document_NumberTools_

CL_NS_DEF(document)

/**
 * Provides support for converting longs to Strings, and back again. The strings
 * are structured so that lexicographic sorting order is preserved.
 * 
 * <p>
 * That is, if l1 is less than l2 for any two longs l1 and l2, then
 * NumberTools.longToString(l1) is lexicographically less than
 * NumberTools.longToString(l2). (Similarly for "greater than" and "equals".)
 * 
 * <p>
 * This class handles <b>all</b> long values (unlike
 * {@link org.apache.lucene.document.DateField}).
 * 
 * 
 */
class CLUCENE_EXPORT NumberTools :LUCENE_BASE {

	#define NUMBERTOOLS_RADIX 36

	#define NEGATIVE_PREFIX _T("-")
	// NB: NEGATIVE_PREFIX must be < POSITIVE_PREFIX
	#define POSITIVE_PREFIX _T("0")

public:
	//NB: this must be less than
    /**
     * Equivalent to longToString(Long.MIN_VALUE); STR_SIZE is depandant on the length of it
     */
	static const TCHAR* MIN_STRING_VALUE;

	/**
     * Equivalent to longToString(Long.MAX_VALUE)
     */
	static const TCHAR* MAX_STRING_VALUE;

	/**
     * The length of (all) strings returned by {@link #longToString}
     */
    LUCENE_STATIC_CONSTANT (size_t, STR_SIZE = 14);

	/**
     * Converts a long to a String suitable for indexing.
	 *
	 * @memory Caller should free the returned buffer
     */
    static TCHAR* longToString(int64_t l);

    /**
     * Converts a String that was returned by {@link #longToString} back to a
     * long.
     * 
     * @throws IllegalArgumentException
     *             if the input is null
     * @throws NumberFormatException
     *             if the input does not parse (it was not a String returned by
     *             longToString()).
     */
    static int64_t stringToLong(const TCHAR* str);

	~NumberTools();

};
CL_NS_END
#endif
