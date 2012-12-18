/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_HitQueue_
#define _lucene_search_HitQueue_

CL_NS_DEF(search)
struct ScoreDoc;

/**
* An optimised PriorityQueue which takes ScoreDoc structs. Some by-ref passing
* and memory related optimisations have been done.
*/
class HitQueue: LUCENE_BASE {
private:
	ScoreDoc* heap;
	size_t _size;
	size_t maxSize;

	void upHeap();
	void downHeap();

protected:
	bool lessThan(struct ScoreDoc& hitA, struct ScoreDoc& hitB);

public:
	void adjustTop();
	struct ScoreDoc& top();
	void put(struct ScoreDoc& element);
	ScoreDoc pop();
	/**
	* Adds element to the PriorityQueue in log(size) time if either
	* the PriorityQueue is not full, or not lessThan(element, top()).
	* @param element
	* @return true if element is added, false otherwise.
	*/
	bool insert(struct ScoreDoc& element);
	/**
	* Returns the number of elements currently stored in the PriorityQueue.
	*/ 
	size_t size();
	HitQueue(const int32_t maxSize);
	~HitQueue();

};
CL_NS_END
#endif
