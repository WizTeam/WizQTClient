/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_TermInfo
#define _lucene_index_TermInfo


CL_NS_DEF(index)

// A TermInfo is the record of information stored for a term.
class TermInfo: LUCENE_BASE{
public:
	// The number of documents which contain the term. 
	int32_t docFreq;

	//A pointer into the TermFreqs file (.frq)
	//The .frq file contains the lists of documents which contain each term, 
	//along with the frequency of the term in that document.
	int64_t freqPointer;

	//A pointer into the TermPosition file (.prx).
	//The .prx file contains the lists of positions that each term 
	//occurs at within documents.
	int64_t proxPointer;

  int32_t skipOffset;

    //Constructor
	TermInfo();

    //Constructor
	TermInfo(const int32_t df, const int64_t fp, const int64_t pp);

	//Constructor
	//Initialises this instance by copying the values of another TermInfo ti
	TermInfo(const TermInfo* ti);

    //Destructor
	~TermInfo();

	//Sets a new document frequency, a new freqPointer and a new proxPointer
	void set(const int32_t docFreq, const int64_t freqPointer, const int64_t proxPointer, int32_t skipOffset);

	//Sets a new document frequency, a new freqPointer and a new proxPointer
    //by copying these values from another instance of TermInfo
	void set(const TermInfo* ti);
};
CL_NS_END
#endif
