/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_TermInfosWriter_
#define _lucene_index_TermInfosWriter_

#include "CLucene/util/Array.h"

CL_CLASS_DEF(store,Directory)
//#include "FieldInfos.h"
//#include "TermInfo.h"
//#include "Term.h"

CL_NS_DEF(index)
class FieldInfos;
class TermInfo;

	// This stores a monotonically increasing set of <Term, TermInfo> pairs in a
	// Directory.  A TermInfos can be written once, in order.
	class TermInfosWriter :LUCENE_BASE{
	private:
		FieldInfos* fieldInfos;
		CL_NS(store)::IndexOutput* output;
		TermInfo* lastTi;
		int64_t size;

    int64_t lastIndexPointer;
    bool isIndex;
    CL_NS(util)::ValueArray<TCHAR> lastTermText;
    int32_t lastTermTextLength;
    int32_t lastFieldNumber;

    CL_NS(util)::ValueArray<TCHAR> termTextBuffer;

		TermInfosWriter* other;

		//inititalize
		TermInfosWriter(CL_NS(store)::Directory* directory, const char* segment, FieldInfos* fis, int32_t interval, bool isIndex);

    int32_t compareToLastTerm(int32_t fieldNumber, const TCHAR* termText, int32_t length);
	public:
    /** Expert: The maximum number of skip levels. Smaller values result in
    * slightly smaller indexes, but slower skipping in big posting lists.
    */
    int32_t maxSkipLevels;

		/** The file format version, a negative number. */
		LUCENE_STATIC_CONSTANT(int32_t,FORMAT=-3);

    //Expert: The fraction of {@link TermDocs} entries stored in skip tables,
    //used to accellerate {@link TermDocs#skipTo(int)}.  Larger values result in
    //smaller indices, greater acceleration, but fewer accelerable cases, while
    //smaller values result in bigger indices, less acceleration and more
    //accelerable cases. More detailed experiments would be useful here. */
    LUCENE_STATIC_CONSTANT(int32_t, DEFAULT_TERMDOCS_SKIP_INTERVAL=16);


		/**
		* Expert: The fraction of terms in the "dictionary" which should be stored
		* in RAM.  Smaller values use more memory, but make searching slightly
		* faster, while larger values use less memory and make searching slightly
		* slower.  Searching is typically not dominated by dictionary lookup, so
		* tweaking this is rarely useful.
		*/
		int32_t indexInterval;// = 128

		/**
		* Expert: The fraction of {@link TermDocs} entries stored in skip tables,
		* used to accellerate {@link TermDocs#SkipTo(int32_t)}.  Larger values result in
		* smaller indexes, greater acceleration, but fewer accelerable cases, while
		* smaller values result in bigger indexes, less acceleration and more
		* accelerable cases. More detailed experiments would be useful here.
		*/
		int32_t skipInterval;// = 16

		TermInfosWriter(CL_NS(store)::Directory* directory, const char* segment, FieldInfos* fis, int32_t interval);

		~TermInfosWriter();


    void add(Term* term, TermInfo* ti);

    /** Adds a new <<fieldNumber, termText>, TermInfo> pair to the set.
    Term must be lexicographically greater than all previous Terms added.
    TermInfo pointers must be positive and greater than all previous.*/
		void add(int32_t fieldNumber, const TCHAR* termText, int32_t termTextLength, const TermInfo* ti);

		/** Called to complete TermInfos creation. */
		void close();

	private:
        /** Helps constructors to initialize instances */
		void initialise(CL_NS(store)::Directory* directory, const char* segment, int32_t interval, bool IsIndex);
		void writeTerm(int32_t fieldNumber, const TCHAR* termText, int32_t termTextLength);
	};
CL_NS_END
#endif
