/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "MultipleTermPositions.h"
#include "IndexReader.h"
#include "CLucene/util/Array.h"
#include "CLucene/util/PriorityQueue.h"

CL_NS_USE(util)

CL_NS_DEF(index)

void MultipleTermPositions::seek(Term*) {
	_CLTHROWA(CL_ERR_UnsupportedOperation, "Unsupported operation: MultipleTermPositions::seek");
}

void MultipleTermPositions::seek(TermEnum*) {
	_CLTHROWA(CL_ERR_UnsupportedOperation, "Unsupported operation: MultipleTermPositions::seek");
}

int32_t MultipleTermPositions::read(int32_t*, int32_t*,int32_t) {
	_CLTHROWA(CL_ERR_UnsupportedOperation, "Unsupported operation: MultipleTermPositions::read");
}

int32_t MultipleTermPositions::getPayloadLength() const {
	_CLTHROWA(CL_ERR_UnsupportedOperation, "Unsupported operation: MultipleTermPositions::getPayloadLength");
}

uint8_t* MultipleTermPositions::getPayload(uint8_t*) {
	_CLTHROWA(CL_ERR_UnsupportedOperation, "Unsupported operation: MultipleTermPositions::getPayload");
}

bool MultipleTermPositions::isPayloadAvailable() const{
	return false;
} 

TermDocs* MultipleTermPositions::__asTermDocs(){ 
	return (TermDocs*)this; 
}
TermPositions* MultipleTermPositions::__asTermPositions(){ 
	return (TermPositions*)this; 
}

	
class MultipleTermPositions::TermPositionsQueue : public CL_NS(util)::PriorityQueue<TermPositions*,
	CL_NS(util)::Deletor::Object<TermPositions> > {
public:
		TermPositionsQueue(TermPositions** termPositions, size_t termPositionsSize) {
			initialize(termPositionsSize, false);

			size_t i=0;
			while (termPositions[i]!=NULL) {
				if (termPositions[i]->next())
					put(termPositions[i]);
                else
                    _CLDELETE( termPositions[ i ] );

				++i;
			}
		}
		virtual ~TermPositionsQueue(){
		}

		TermPositions* peek() {
			return top();
		}

		bool lessThan(TermPositions* a, TermPositions* b) {
			return a->doc() < b->doc();
		}
};

int IntQueue_sort(const void* a, const void* b){
  return ( *(int*)a - *(int*)b );
}
class MultipleTermPositions::IntQueue {
private:
	ValueArray<int32_t>* _array;
	int32_t _index;
	int32_t _lastIndex;

public:
	IntQueue():_array(_CLNEW ValueArray<int32_t>(16)), _index(0), _lastIndex(0){
	}
	virtual ~IntQueue(){
		_CLLDELETE(_array);
	}

	void add(const int32_t i) {
		if (_lastIndex == _array->length)
			_array->resize(_array->length*2);

		_array->values[_lastIndex++] = i;
	}

	int32_t next() {
		return _array->values[_index++];
	}

	void sort() {
    int len = _lastIndex - _index;
    qsort(_array->values+_index, len, sizeof(int32_t), IntQueue_sort);
	}

	void clear() {
		_index = 0;
		_lastIndex = 0;
	}

	int32_t size() {
		return (_lastIndex - _index);
	}
};

MultipleTermPositions::MultipleTermPositions(IndexReader* indexReader, const CL_NS(util)::ArrayBase<Term*>* terms) : _posList(_CLNEW IntQueue()){
	CLLinkedList<TermPositions*> termPositions;
  for ( size_t i=0;i<terms->length;i++){
    termPositions.push_back( indexReader->termPositions(terms->values[i]));
	}

	TermPositions** tps = _CL_NEWARRAY(TermPositions*, terms->length+1); // i == tpsSize
	termPositions.toArray_nullTerminated(tps);

	_termPositionsQueue = _CLNEW TermPositionsQueue(tps,terms->length);
	_CLDELETE_LARRAY(tps);
}

MultipleTermPositions::~MultipleTermPositions() {
	_CLLDELETE(_termPositionsQueue);
	_CLLDELETE(_posList);
}

bool MultipleTermPositions::next() {
	if (_termPositionsQueue->size() == 0)
		return false;

	_posList->clear();
	_doc = _termPositionsQueue->peek()->doc();

	TermPositions* tp;
	do {
		tp = _termPositionsQueue->peek();

		for (int32_t i = 0; i < tp->freq(); i++)
			_posList->add(tp->nextPosition());

		if (tp->next())
			_termPositionsQueue->adjustTop();
		else {
			_termPositionsQueue->pop();
			tp->close();
			_CLLDELETE(tp);
		}
	} while (_termPositionsQueue->size() > 0 && _termPositionsQueue->peek()->doc() == _doc);

	_posList->sort();
	_freq = _posList->size();

	return true;
}

int32_t MultipleTermPositions::nextPosition() {
	return _posList->next();
}

bool MultipleTermPositions::skipTo(int32_t target) {
	while (_termPositionsQueue->peek() != NULL && target > _termPositionsQueue->peek()->doc()) {
		TermPositions* tp = _termPositionsQueue->pop();
		if (tp->skipTo(target))
			_termPositionsQueue->put(tp);
		else {
			tp->close();
			_CLLDELETE(tp);
		}
	}
	return next();
}

int32_t MultipleTermPositions::doc() const {
	return _doc;
}

int32_t MultipleTermPositions::freq() const {
	return _freq;
}

void MultipleTermPositions::close() {
	while (_termPositionsQueue->size() > 0) {
		TermPositions* tp = _termPositionsQueue->pop();
		tp->close();
		_CLLDELETE(tp);
	}
}

CL_NS_END
