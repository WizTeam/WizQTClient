/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Scorer.h"
#include "Explanation.h"
#include "Similarity.h"
#include "SearchHeader.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/index/Terms.h"
#include "_PhraseQueue.h"
#include "_PhraseScorer.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)


	PhraseScorer::PhraseScorer(Weight* _weight, TermPositions** tps, 
		int32_t* offsets, Similarity* similarity, uint8_t* _norms):
		Scorer(similarity), weight(_weight), norms(_norms), value(_weight->getValue()), firstTime(true), more(true), freq(0.0f),
			first(NULL), last(NULL)
	{
	//Func - Constructor
	//Pre  - tps != NULL and is an array of TermPositions
	//       tpsLength >= 0
	//       n != NULL
	//Post - The instance has been created

		CND_PRECONDITION(tps != NULL,"tps is NULL");

		// convert tps to a list of phrase positions.
		// note: phrase-position differs from term-position in that its position
		// reflects the phrase offset: pp.pos = tp.pos - offset.
		// this allows to easily identify a matching (exact) phrase 
		// when all PhrasePositions have exactly the same position.
		int32_t i = 0;
		while(tps[i] != NULL){
			PhrasePositions *pp = _CLNEW PhrasePositions(tps[i], offsets[i]);
			CND_CONDITION(pp != NULL,"Could not allocate memory for pp");

			//Store PhrasePos into the PhrasePos pq
			if (last != NULL) {			  // add next to end of list
				last->_next = pp;
			} else
				first = pp;
			last = pp;

			i++;
		}

		pq = _CLNEW PhraseQueue(i); //i==tps.length
		CND_CONDITION(pq != NULL,"Could not allocate memory for pq");
	}

	PhraseScorer::~PhraseScorer() {
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		//The PhraseQueue pq (which is a PriorityQueue) pq is actually empty at present, the elements
		//having been transferred by pqToList() to the linked list starting with
		//first.  The nodes of that linked list are deleted by the destructor of
		//first, rather than the destructor of pq.
		_CLLDELETE(first);
		_CLLDELETE(pq);
	}

	bool PhraseScorer::next(){
		if (firstTime) {
			init();
			firstTime = false;
		} else if (more) {
			more = last->next(); // trigger further scanning
		}
		return doNext();
	}

	// next without initial increment
	bool PhraseScorer::doNext() {
		while (more) {
			while (more && first->doc < last->doc) {      // find doc w/ all the terms
				more = first->skipTo(last->doc);            // skip first upto last
				firstToLast();                            // and move it to the end
			}

			if (more) {
				// found a doc with all of the terms
				freq = phraseFreq();                      // check for phrase
				if (freq == 0.0f)                         // no match
					more = last->next();                     // trigger further scanning
				else
					return true;                            // found a match
			}
		}
		return false;                                 // no more matches
	}

	float_t PhraseScorer::score(){
		//System.out.println("scoring " + first.doc);
		float_t raw = getSimilarity()->tf(freq) * value; // raw score
		return raw * Similarity::decodeNorm(norms[first->doc]); // normalize
	}

	bool PhraseScorer::skipTo(int32_t target) {
		firstTime = false;
		for (PhrasePositions* pp = first; more && pp != NULL; pp = pp->_next) {
			more = pp->skipTo(target);
		}
		if (more)
			sort();                                     // re-sort
		return doNext();
	}

	void PhraseScorer::init() {
		for (PhrasePositions* pp = first; more && pp != NULL; pp = pp->_next) 
			more = pp->next();
		if(more)
			sort();
	}
	  
	void PhraseScorer::sort() {
		pq->clear();
		for (PhrasePositions* pp = first; pp != NULL; pp = pp->_next)
			pq->put(pp);
		pqToList();
	}

	void PhraseScorer::pqToList(){
	//Func - Transfers the PhrasePositions from the PhraseQueue pq to
	//       the PhrasePositions list with first as its first element
	//Pre  - pq != NULL
	//       first = NULL
	//       last = NULL
	//Post - All PhrasePositions have been transfered to the list
	//       of PhrasePositions of which the first element is pointed to by first
	//       and the last element is pointed to by last

		CND_PRECONDITION(pq != NULL,"pq is NULL");
		
		last = first = NULL;

		PhrasePositions* PhrasePos = NULL;

		//As long pq is not empty
		while (pq->top() != NULL){
			//Pop a PhrasePositions instance
			PhrasePos = pq->pop();

			// add next to end of list
			if (last != NULL) {
				last->_next = PhrasePos;
			} else {
				first = PhrasePos;
			}

			//Let last point to the new last PhrasePositions instance just added
			last = PhrasePos;
			//Reset the next of last to NULL
			last->_next = NULL;
		}

		//Check to see that pq is empty now
		CND_CONDITION(pq->size()==0, "pq is not empty while it should be");
	}

	void PhraseScorer::firstToLast(){
	//Func - Moves first to the end of the list
	//Pre  - first is NULL or points to an PhrasePositions Instance
	//       last  is NULL or points to an PhrasePositions Instance
	//       first and last both are NULL or both are not NULL
	//Post - The first element has become the last element in the list

		CND_PRECONDITION(((first==NULL && last==NULL) ||(first !=NULL && last != NULL)),
					   "Either first or last is NULL but not both");

		//Check if first and last are valid pointers
		if(first && last){
			last->_next = first;
			last = first;
			first = first->_next;
			last->_next = NULL;
		}
	}


	Explanation* PhraseScorer::explain(int32_t _doc) {
		Explanation* tfExplanation = _CLNEW Explanation();

		while (next() && doc() < _doc){
		}

		float_t phraseFreq = (doc() == _doc) ? freq : 0.0f;
		tfExplanation->setValue(getSimilarity()->tf(phraseFreq));

		StringBuffer buf;
		buf.append(_T("tf(phraseFreq="));
		buf.appendFloat(phraseFreq,2);
		buf.append(_T(")"));
		tfExplanation->setDescription(buf.getBuffer());

		return tfExplanation;
	}

	TCHAR* PhraseScorer::toString() { 
		StringBuffer buf;
		buf.append(_T("scorer("));

		TCHAR* tmp = weight->toString();
		buf.append(tmp);
		_CLDELETE_CARRAY(tmp);

		buf.append(_T(")"));

		return buf.toString();
	}

	int32_t PhraseScorer::doc() const { return first->doc; }

CL_NS_END
