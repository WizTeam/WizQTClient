#include "CLucene/_ApiHeader.h"
#include "ScorerDocQueue.h"
#include "Scorer.h"

CL_NS_DEF(util)

class ScorerDocQueue::HeapedScorerDoc:LUCENE_BASE {
public:
	Scorer* _scorer;
	int32_t _doc;
	
	HeapedScorerDoc( Scorer* s );
	HeapedScorerDoc( Scorer* s, int32_t doc );
	~HeapedScorerDoc();
	
	void adjust();
};

ScorerDocQueue::HeapedScorerDoc::HeapedScorerDoc( Scorer* scorer ) : _scorer(scorer), _doc(scorer->doc())
{
}

ScorerDocQueue::HeapedScorerDoc::HeapedScorerDoc( Scorer* scorer, int32_t doc ) : _scorer(scorer), _doc(doc)
{		
}

ScorerDocQueue::HeapedScorerDoc::~HeapedScorerDoc()
{	
}

void ScorerDocQueue::HeapedScorerDoc::adjust()
{
	this->_doc = _scorer->doc();
}

ScorerDocQueue::ScorerDocQueue( int32_t maxSize ) : maxSize(maxSize), _size(0)
{
	int heapSize = maxSize + 1;
	heap = _CL_NEWARRAY( HeapedScorerDoc*, heapSize );
	for ( int32_t i = 0; i < heapSize; i++ ) {
		heap[i] = NULL;
	}
	topHsd = heap[1];
}

ScorerDocQueue::~ScorerDocQueue()
{
	clear();
	_CLDELETE_ARRAY( heap );
}

void ScorerDocQueue::put( Scorer* scorer )
{
	_size++;
	heap[ _size ] = _CLNEW HeapedScorerDoc( scorer );
	upHeap();
}

bool ScorerDocQueue::insert( Scorer* scorer )
{
	if ( _size < maxSize ) {
		put( scorer );
		return true;
	} else {
		int32_t docNr = scorer->doc();
		if (( _size > 0 ) && ( !( docNr < topHsd->_doc ))) {
			_CLDELETE( heap[1] );
			heap[1] = _CLNEW HeapedScorerDoc( scorer, docNr );
			downHeap();
			return true;
		} else {
			return false;
		}
	}
}

Scorer* ScorerDocQueue::pop()
{
	Scorer* result = topHsd->_scorer;
	popNoResult();
	return result;
}

void ScorerDocQueue::adjustTop()
{
	topHsd->adjust();
	downHeap();
}

int32_t ScorerDocQueue::size()
{
	return _size;
}

void ScorerDocQueue::clear()
{
	for ( int32_t i = 0; i <= _size; i++ ) {
		_CLDELETE( heap[i] );
	}
	_size = 0;
}

Scorer* ScorerDocQueue::top()
{
	return topHsd->_scorer;
}

int32_t ScorerDocQueue::topDoc()
{
	return topHsd->_doc;
}

float_t ScorerDocQueue::topScore()
{
	return topHsd->_scorer->score();
}

bool ScorerDocQueue::topNextAndAdjustElsePop()
{
	return checkAdjustElsePop( topHsd->_scorer->next() );
}

bool ScorerDocQueue::topSkipToAndAdjustElsePop( int32_t target )
{
	return checkAdjustElsePop( topHsd->_scorer->skipTo( target ));
}

bool ScorerDocQueue::checkAdjustElsePop( bool cond )
{
	if ( cond ) {
		topHsd->_doc = topHsd->_scorer->doc();
	} else {
		_CLLDELETE( heap[1] );
		heap[1] = heap[_size];
		heap[_size] = NULL;
		_size--;
	}
	downHeap();
	return cond;
}

void ScorerDocQueue::popNoResult()
{
	_CLLDELETE( heap[1] );
	heap[1] = heap[_size];
	heap[_size] = NULL;
	_size--;
	downHeap();
}

void ScorerDocQueue::upHeap()
{
	int32_t i = _size;
	HeapedScorerDoc* node = heap[i];
	int32_t j = i >> 1;
	while (( j > 0 ) && ( node->_doc < heap[j]->_doc )) {
		heap[i] = heap[j];
		i = j;
		j = j >> 1;
	}
	heap[i] = node;
	topHsd = heap[1];
}

void ScorerDocQueue::downHeap()
{
	int32_t i = 1;
	HeapedScorerDoc* node = heap[i];
	int32_t j = i << 1;
	int32_t k = j + 1;
	
	if (( k <= _size ) && ( heap[k]->_doc < heap[j]->_doc )) {
		j = k;
	}
	
	while (( j <= _size ) && ( heap[j]->_doc < node->_doc )) {
		heap[i] = heap[j];
		i = j;
		j = i << 1;
		k = j + 1;
		if (( k <= _size ) && ( heap[k]->_doc < heap[j]->_doc )) {
			j = k;
		}
	}
	
	heap[i] = node;
	topHsd = heap[1];
}

CL_NS_END

