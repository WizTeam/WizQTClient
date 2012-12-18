/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_TermInfosReader_
#define _lucene_index_TermInfosReader_


//#include "Terms.h"
#include "_SegmentTermEnum.h"
CL_CLASS_DEF(store,Directory)
//CL_CLASS_DEF(store,IndexInput)
#include "CLucene/util/_ThreadLocal.h"
//#include "FieldInfos.h"
//#include "TermInfo.h"
//#include "TermInfosWriter.h"

CL_NS_DEF(index)
/** This stores a monotonically increasing set of <Term, TermInfo> pairs in a
* Directory.  Pairs are accessed either by Term or by ordinal position the
* set.
*
* PORT STATUS: 365707 (jlucene 1.9) -- started port to JLucene 2.3.2
*/
	class TermInfosReader :LUCENE_BASE{
	private:
		CL_NS(store)::Directory* directory;
		const char* segment;
		FieldInfos* fieldInfos;

		CL_NS(util)::ThreadLocal<SegmentTermEnum*, 
			CL_NS(util)::Deletor::Object<SegmentTermEnum> > enumerators;

		SegmentTermEnum* getEnum();
		SegmentTermEnum* origEnum;
		SegmentTermEnum* indexEnum;
		int64_t _size;

		Term* indexTerms; //note: this is a list of objects, not arrays!
    int32_t indexTermsLength;
		TermInfo* indexInfos;
		int64_t* indexPointers;

		int32_t indexDivisor;
		int32_t totalIndexInterval;

		DEFINE_MUTEX(THIS_LOCK)

	public:
		/**
		* Constructor.
        * Reads the TermInfos file (.tis) and eventually the Term Info Index file (.tii)
		*/
		TermInfosReader(CL_NS(store)::Directory* dir, const char* segment, FieldInfos* fis,
			const int32_t readBufferSize = CL_NS(store)::BufferedIndexInput::BUFFER_SIZE);
		~TermInfosReader();

		int32_t getSkipInterval() const;
		int32_t getMaxSkipLevels() const;

		/**
		* <p>Sets the indexDivisor, which subsamples the number
		* of indexed terms loaded into memory.  This has a
		* similar effect as {@link
		* IndexWriter#setTermIndexInterval} except that setting
		* must be done at indexing time while this setting can be
		* set per reader.  When set to N, then one in every
		* N*termIndexInterval terms in the index is loaded into
		* memory.  By setting this to a value > 1 you can reduce
		* memory usage, at the expense of higher latency when
		* loading a TermInfo.  The default value is 1.</p>
		*
		* <b>NOTE:</b> you must call this before the term
		* index is loaded.  If the index is already loaded,
		* an IllegalStateException is thrown.
		*
		* @throws IllegalStateException if the term index has
		* already been loaded into memory.
		*/
		void setIndexDivisor(const int32_t _indexDivisor);

		/** Returns the indexDivisor.
		* @see #setIndexDivisor
		*/
		int32_t getIndexDivisor() const;

		/** Close the enumeration of TermInfos */
		void close();
		
		/** Returns the number of term/value pairs in the set. */
		int64_t size() const;

		/**
		* Returns an enumeration of terms starting at or after the named term.
		* If no term is specified, an enumeration of all the Terms 
		* and TermInfos in the set is returned.
		*/
		SegmentTermEnum* terms(const Term* term=NULL);
		
		/** Returns the TermInfo for a Term in the set, or null. */
		TermInfo* get(const Term* term);
	private:
		/** Reads the term info index file or .tti file. */
		void ensureIndexIsRead();

		/** Returns the offset of the greatest index entry which is less than or equal to term.*/
		int32_t getIndexOffset(const Term* term);

		/** Reposition the current Term and TermInfo to indexOffset */
		void seekEnum(const int32_t indexOffset);  

		/** Scans the Enumeration of terms for term and returns the corresponding TermInfo instance if found.
        * The search is started from the current term.
		*/
		TermInfo* scanEnum(const Term* term);

        /** Scans the enumeration to the requested position and returns the Term located at that position */
		Term* scanEnum(const int32_t position);
		
		/** Returns the position of a Term in the set or -1. */
		int64_t getPosition(const Term* term);

		/** Returns the nth term in the set. synchronized */
		Term* get(const int32_t position);

	};
CL_NS_END
#endif
