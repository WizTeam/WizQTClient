/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_SegmentHeader.h"
#include "_SegmentMergeInfo.h"
#include "_SegmentTermEnum.h"

CL_NS_DEF(index)

SegmentMergeInfo::SegmentMergeInfo(const int32_t b, TermEnum* te, IndexReader* r):
    docMap(NULL),
    termEnum(te),
    base(b),
    reader(r)
{
//Func - Constructor
//Pre  - b >= 0
//       te contains a valid reference to a SegmentTermEnum instance
//       r contains a valid reference to a SegmentReader instance
//Post - The instance has been created

    CND_PRECONDITION(b >= 0, "b is a negative number");

    postings=NULL;
	term   = te->term();
}

SegmentMergeInfo::~SegmentMergeInfo(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed

  close();
}

int32_t* SegmentMergeInfo::getDocMap(){
	if ( docMap == NULL ){
      	// build array which maps document numbers around deletions 
		if (reader->hasDeletions()) {
			//Get the total number of documents managed by the reader including the deleted ones
			int32_t maxDoc = reader->maxDoc();
			//Create a map for all documents
			docMap = _CL_NEWARRAY(int32_t,maxDoc);
			int32_t j = 0;
			//Iterate through all the document numbers
			for (int32_t i = 0; i < maxDoc; i++) {
            //Check if document i is marked deleted
				if (reader->isDeleted(i)){
					//Document i has not been marked deleted so assign -1
					docMap[i] = -1;
				}else{
					docMap[i] = j++;
				}
			}
		}
	}
	return docMap;
}

TermPositions* SegmentMergeInfo::getPositions() {
    if (postings == NULL) {
    	postings = reader->termPositions();
    }
    return postings;
}


bool SegmentMergeInfo::next() {
//Func - Moves the current term of the enumeration termEnum to the next and term
//       points to this new current term
//Pre  - true
//Post - Returns true if the term has been moved to the next otherwise false
	if (termEnum->next()) {
		_CLDECDELETE(term);
		term = termEnum->term();
		return true;
	} else {
		_CLDECDELETE(term); //TODO: test HighFreqTerms errors with this
		term = NULL;
		return false;
	}
}

void SegmentMergeInfo::close() {
//Func - Closes the the resources
//Pre  - true
//Post - The resources have been closed

    //First make sure posting has been closed
    if ( postings != NULL ){
        postings->close();
        _CLVDELETE(postings); //todo: not a clucene object... should be
    }

    if ( termEnum != NULL ){
        termEnum->close();
        _CLDELETE(termEnum);
    }
	_CLDECDELETE(term);
	_CLDELETE_ARRAY(docMap);
}

CL_NS_END
