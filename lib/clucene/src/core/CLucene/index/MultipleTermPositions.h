/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_index_MultipleTermPositions_
#define _lucene_index_MultipleTermPositions_

#include "Terms.h"
#include "CLucene/util/Array.h"

CL_NS_DEF(index)

class Term;
class IndexReader;

class CLUCENE_EXPORT MultipleTermPositions : public TermPositions {
private:
	class TermPositionsQueue;
	class IntQueue;

	int32_t _doc;
	int32_t _freq;
	TermPositionsQueue* _termPositionsQueue;
	IntQueue* _posList;

public:
	/**
	* Creates a new <code>MultipleTermPositions</code> instance.
	* 
	* @exception IOException
	*/ 
  MultipleTermPositions(IndexReader* indexReader, const CL_NS(util)::ArrayBase<Term*>* terms);
	virtual ~MultipleTermPositions();

	bool next();

	int32_t nextPosition();

	bool skipTo(const int32_t target);

	int32_t doc() const;

	int32_t freq() const;

	void close();

	/**
	* Not implemented.
	* @throws UnsupportedOperationException
	*/
	void seek(Term*);

	/**
	* Not implemented.
	* @throws UnsupportedOperationException
	*/
	void seek(TermEnum*);

	/**
	* Not implemented.
	* @throws UnsupportedOperationException
	*/
	int32_t read(int32_t*, int32_t*,int32_t);

	/**
	* Not implemented.
	* @throws UnsupportedOperationException
	*/
	int32_t getPayloadLength() const;

	/**
	* Not implemented.
	* @throws UnsupportedOperationException
	*/
	uint8_t* getPayload(uint8_t*);

	/**
	*
	* @return false
	*/
	// Java-TODO: Remove warning after API has been finalized
	bool isPayloadAvailable() const; 

	TermDocs* __asTermDocs();
	TermPositions* __asTermPositions();
};


CL_NS_END
#endif
