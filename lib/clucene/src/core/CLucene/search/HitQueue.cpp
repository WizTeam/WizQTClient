/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "SearchHeader.h"
#include "_HitQueue.h"

CL_NS_DEF(search)

void HitQueue::upHeap(){
	size_t i = _size;
	ScoreDoc node = heap[i];			  // save bottom node (WAS object)
	int32_t j = ((uint32_t)i) >> 1;
	while (j > 0 && lessThan(node,heap[j])) {
		heap[i] = heap[j];			  // shift parents down
		i = j;
		j = ((uint32_t)j) >> 1;
	}
	heap[i] = node;				  // install saved node
}
void HitQueue::downHeap(){
	size_t i = 1;
	ScoreDoc node = heap[i];			  // save top node
	size_t j = i << 1;				  // find smaller child
	size_t k = j + 1;
	if (k <= _size && lessThan(heap[k], heap[j])) {
		j = k;
	}
	while (j <= _size && lessThan(heap[j],node)) {
		heap[i] = heap[j];			  // shift up child
		i = j;
		j = i << 1;
		k = j + 1;
		if (k <= _size && lessThan(heap[k], heap[j])) {
			j = k;
		}
	}
	heap[i] = node;				  // install saved node
}

void HitQueue::adjustTop(){
	downHeap();
}
size_t HitQueue::size(){
	return _size;
}

struct ScoreDoc& HitQueue::top(){
	if ( _size == 0 )
		_CLTHROWA(CL_ERR_IndexOutOfBounds, "Attempted to access empty hitqueue::top");
	return heap[1];
}

void HitQueue::put(struct ScoreDoc& element){
	if ( _size>=maxSize )
		_CLTHROWA(CL_ERR_IndexOutOfBounds,"add is out of bounds");

	_size++;	
	heap[_size] = element;		
	upHeap();
}

ScoreDoc HitQueue::pop(){
	if (_size > 0) {
		ScoreDoc result = heap[1];			  // save first value
		heap[1] = heap[_size];			  // move last to first

		_size--;
		downHeap();				  // adjust heap
		return result;
	} else
		_CLTHROWA(CL_ERR_IndexOutOfBounds, "Attempted to access empty hitqueue::top");
}

bool HitQueue::insert(struct ScoreDoc& element){
	if(_size < maxSize){
		put(element);
		return true;
	}else if(_size > 0 && !lessThan(element, heap[1])){
		heap[1] = element;
		adjustTop();
		return true;
	}else
		return false;
}

HitQueue::HitQueue(const int32_t maxSize){
	_size = 0;
    this->maxSize = maxSize;
	int32_t heapSize = maxSize + 1;
	heap = new ScoreDoc[heapSize];
}
HitQueue::~HitQueue(){
	delete [] heap;
}

bool HitQueue::lessThan(struct ScoreDoc& hitA, struct ScoreDoc& hitB){
	if (hitA.score == hitB.score)
		return hitA.doc > hitB.doc; 
	else
		return hitA.score < hitB.score;
}


CL_NS_END
