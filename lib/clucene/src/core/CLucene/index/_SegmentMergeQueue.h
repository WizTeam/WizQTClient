/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_SegmentMergeQueue_
#define _lucene_index_SegmentMergeQueue_

#include "CLucene/util/PriorityQueue.h"
//#include "SegmentMergeInfo.h"

CL_NS_DEF(index)
	class SegmentMergeQueue :public CL_NS(util)::PriorityQueue<SegmentMergeInfo*,CL_NS(util)::Deletor::Object<SegmentMergeInfo> > {
	public:
		//Constructor
	    //Creates a queue of length size
		SegmentMergeQueue(const int32_t size);
		
		//Destructor
	    //Does nothing as its parent class will clean up everything
		~SegmentMergeQueue();

		//Closes and destroyes all SegmentMergeInfo Instances in the queue
		void close();
	protected:
		//Overloaded method that implements the lessThan operator for the parent class
        //This method is used by the parent class Priority queue to reorder its internal
        //data structures. This implementation check if stiA is less than the current term of stiB.
		bool lessThan(SegmentMergeInfo* stiA, SegmentMergeInfo* stiB);

	};
CL_NS_END
#endif
