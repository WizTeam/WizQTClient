/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_PriorityQueue_
#define _lucene_util_PriorityQueue_

#include <stdlib.h>

CL_NS_DEF(util)

/** A PriorityQueue maintains a partial ordering of its elements such that the
  least element can always be found in constant time.  Put()'s and pop()'s
  require log(size) time. */
template <class _type,typename _valueDeletor> 
class CLUCENE_INLINE_EXPORT PriorityQueue {
	private:
		size_t _size;
		bool dk;
		size_t maxSize;
	protected:
		_type* heap; //(was object[])

	private:
		void upHeap(){
			size_t i = _size;
			_type node = heap[i];			  // save bottom node (WAS object)
			int32_t j = ((uint32_t)i) >> 1;
			while (j > 0 && lessThan(node,heap[j])) {
				heap[i] = heap[j];			  // shift parents down
				i = j;
				j = ((uint32_t)j) >> 1;
			}
			heap[i] = node;				  // install saved node
		}
		void downHeap(){
			size_t i = 1;
			_type node = heap[i];			  // save top node
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

	protected:
		PriorityQueue():_size(0),dk(false),maxSize(0),heap(NULL){
		}

		// Determines the ordering of objects in this priority queue.  Subclasses
		//	must define this one method. 
		virtual bool lessThan(_type a, _type b)=0;

		// Subclass constructors must call this. 
		void initialize(const int32_t maxSize, bool deleteOnClear){
			_size = 0;
			dk = deleteOnClear;
			int32_t heapSize;
			if (0 == maxSize)
				// We allocate 1 extra to avoid if statement in top()
				heapSize = 2;
			else
				heapSize = maxSize + 1;
			heap = _CL_NEWARRAY(_type,heapSize);
            this->maxSize = maxSize;
		}

	public:
		virtual ~PriorityQueue(){
			clear();
			_CLDELETE_LARRAY(heap);
		}

	 /**
      * Adds an Object to a PriorityQueue in log(size) time.
      * If one tries to add more objects than maxSize from initialize
      * a RuntimeException (ArrayIndexOutOfBound) is thrown.
      */
      void put(_type element){
      		if ( _size>=maxSize )
				_CLTHROWA(CL_ERR_IndexOutOfBounds,"add is out of bounds");

			++_size;	
			heap[_size] = element;		
			upHeap();
		}

	  /**
	  * Adds element to the PriorityQueue in log(size) time if either
	  * the PriorityQueue is not full, or not lessThan(element, top()).
	  * @param element
	  * @return true if element is added, false otherwise.
	  */
	  bool insert(_type element){
		  _type t = insertWithOverflow(element);
		  if (t != element) {
			  if (t) _valueDeletor::doDelete(t);
			  return true;
		  }
		  return false;
	  }

	  /**
	  * insertWithOverflow() is the same as insert() except its
	  * return value: it returns the object (if any) that was
	  * dropped off the heap because it was full. This can be
	  * the given parameter (in case it is smaller than the
	  * full heap's minimum, and couldn't be added), or another
	  * object that was previously the smallest value in the
	  * heap and now has been replaced by a larger one, or null
	  * if the queue wasn't yet full with maxSize elements.
	  * NOTE: value is not being deleted - its the user responsibilty
	  * to dispose the returned _type (only if != NULL && != element).
	  */
	  _type insertWithOverflow(_type element) {
		  if(_size < maxSize){
			  put(element);
			  return NULL;
		  }else if(_size > 0 && !lessThan(element, heap[1])){
			  _type ret = heap[1];
			  heap[1] = element;
			  adjustTop();
			  return ret;
		  }else
			  return element;
	  }

	  /**
	  * Returns the least element of the PriorityQueue in constant time. 
	  */
	  _type top(){
		  // We don't need to check size here: if maxSize is 0,
		  // then heap is length 2 array with both entries null.
		  // If size is 0 then heap[1] is already null.
		  return heap[1];
	  }

		/** Removes and returns the least element of the PriorityQueue in log(size)
		*	time.  
		*/
		_type pop(){
			if (_size > 0) {
				_type result = heap[1];			  // save first value
				heap[1] = heap[_size];			  // move last to first

				heap[_size] = (_type)0;			  // permit GC of objects
				--_size;
				downHeap();				  // adjust heap
				return result;
			} else
				return (_type)NULL;
		}

		/**Should be called when the object at top changes values.  Still log(n)
		   worst case, but it's at least twice as fast to <pre>
		    { pq.top().change(); pq.adjustTop(); }
		   </pre> instead of <pre>
		    { o = pq.pop(); o.change(); pq.push(o); }
		   </pre>
		*/
		void adjustTop(){
			downHeap();
		}
		    

		/**
		* Returns the number of elements currently stored in the PriorityQueue.
		*/ 
		size_t size(){
			return _size;
		}
		  
		/** 
		* Removes all entries from the PriorityQueue. 
		*/
		void clear(){
			for (size_t i = 1; i <= _size; ++i){
				if ( dk ){
					_valueDeletor::doDelete(heap[i]);
				}
			}
			_size = 0;
		}
	};

CL_NS_END
#endif
