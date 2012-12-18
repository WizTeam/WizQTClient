/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_WildcardTermEnum_
#define _lucene_search_WildcardTermEnum_

//#include "CLucene/index/IndexReader.h"
CL_CLASS_DEF(index,Term)
CL_CLASS_DEF(index,IndexReader)
//#include "CLucene/index/Terms.h"
#include "FilteredTermEnum.h"

CL_NS_DEF(search)
    /**
     * Subclass of FilteredTermEnum for enumerating all terms that match the
     * specified wildcard filter term->
     * <p>
     * Term enumerations are always ordered by term->compareTo().  Each term in
     * the enumeration is greater than all that precede it.
     */
	class CLUCENE_EXPORT WildcardTermEnum: public FilteredTermEnum {
    private:
        CL_NS(index)::Term* __term;
        TCHAR* pre;
        int32_t preLen;
        bool fieldMatch;
        bool _endEnum;

        /********************************************
        * const TCHAR* equality with support for wildcards
        ********************************************/

        protected:
        bool termCompare(CL_NS(index)::Term* term) ;

        public:

        /**
		* Creates a new <code>WildcardTermEnum</code>.  Passing in a
		* {@link Term Term} that does not contain a
		* <code>LUCENE_WILDCARDTERMENUM_WILDCARD_STRING</code> or
		* <code>LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR</code> will cause an exception to be thrown.
		*/
        WildcardTermEnum(CL_NS(index)::IndexReader* reader, CL_NS(index)::Term* term);
        ~WildcardTermEnum();

        float_t difference() ;

        bool endEnum() ;

        /**
         * Determines if a word matches a wildcard pattern.
         */
        static bool wildcardEquals(const TCHAR* pattern, int32_t patternLen, int32_t patternIdx, const TCHAR* str, int32_t strLen, int32_t stringIdx);

        void close();

		    const char* getObjectName() const;
		    static const char* getClassName();
    };
CL_NS_END
#endif
