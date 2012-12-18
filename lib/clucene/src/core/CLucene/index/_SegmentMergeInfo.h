/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SegmentMergeInfo_
#define _lucene_index_SegmentMergeInfo_


//#include "SegmentTermEnum.h"
//#include "SegmentHeader.h"
#include "Terms.h"

CL_NS_DEF(index)
class IndexReader;

class SegmentMergeInfo:LUCENE_BASE {
private:
	int32_t* docMap;				  // maps around deleted docs
	TermPositions* postings;
public:
	TermEnum* termEnum;
	Term* term;
	int32_t base;
	IndexReader* reader;
     
	//Constructor
	SegmentMergeInfo(const int32_t b, TermEnum* te, IndexReader* r);

	//Destructor
	~SegmentMergeInfo();

	//Moves the current term of the enumeration termEnum to the next and term
    //points to this new current term
	bool next();

	//Closes the the resources
	void close();

    // maps around deleted docs
	int32_t* getDocMap();

	TermPositions* getPositions();
};
CL_NS_END
#endif

