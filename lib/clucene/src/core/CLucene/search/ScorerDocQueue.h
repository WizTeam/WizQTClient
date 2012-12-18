/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_ScorerDocQueue_
#define _lucene_util_ScorerDocQueue_


CL_CLASS_DEF(search,Scorer)

CL_NS_USE(search)
CL_NS_DEF(util)

class CLUCENE_EXPORT ScorerDocQueue:LUCENE_BASE {
private:
	class HeapedScorerDoc;
	
	HeapedScorerDoc** heap;
	HeapedScorerDoc* topHsd;
	
	int32_t maxSize;
	int32_t _size;
	
public:
	
	ScorerDocQueue( int32_t maxSize );
	virtual ~ScorerDocQueue();
	
	void put( Scorer* scorer );
	bool insert( Scorer* scorer );
	Scorer* pop();
	void adjustTop();
	int32_t size();
	void clear();
	
	Scorer* top();
	int32_t topDoc();
	float_t topScore();
	bool topNextAndAdjustElsePop();
	bool topSkipToAndAdjustElsePop( int32_t target );
	
private:
	
	bool checkAdjustElsePop( bool cond );
	void popNoResult();
	void upHeap();
	void downHeap();
	
};

CL_NS_END
#endif
