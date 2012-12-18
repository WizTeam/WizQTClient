/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_PhraseScorer_
#define _lucene_search_PhraseScorer_

#include "_PhraseQueue.h"
#include "Scorer.h"

CL_NS_DEF(search)

class Explanation;
/** Expert: Scoring functionality for phrase queries.
* <br>A document is considered matching if it contains the phrase-query terms  
* at "valid" positons. What "valid positions" are
* depends on the type of the phrase query: for an exact phrase query terms are required 
* to appear in adjacent locations, while for a sloppy phrase query some distance between 
* the terms is allowed. The abstract method {@link #phraseFreq()} of extending classes
* is invoked for each document containing all the phrase query terms, in order to 
* compute the frequency of the phrase query in that document. A non zero frequency
* means a match. 
*/
class PhraseScorer: public Scorer {
private:
	Weight* weight;
protected:
	uint8_t* norms;
	float_t value;
private:
	bool firstTime;
	bool more;
protected:
	float_t freq; //phrase frequency in current doc as computed by phraseFreq().

	PhraseQueue* pq;        //is used to order the list point to by first and last
	PhrasePositions* first; //Points to the first in the list of PhrasePositions
	PhrasePositions* last;  //Points to the last in the list of PhrasePositions

public:
	//Constructor
	PhraseScorer(Weight* _weight, CL_NS(index)::TermPositions** tps, 
		int32_t* offsets, Similarity* similarity, uint8_t* _norms);
	virtual ~PhraseScorer();

	int32_t doc() const;
	bool next();
	float_t score();
	bool skipTo(int32_t target);


	Explanation* explain(int32_t doc);
	virtual TCHAR* toString();
protected:
	/**
	* For a document containing all the phrase query terms, compute the
	* frequency of the phrase in that document. 
	* A non zero frequency means a match.
	* <br>Note, that containing all phrase terms does not guarantee a match - they have to be found in matching locations.  
	* @return frequency of the phrase in current doc, 0 if not found. 
	*/
	virtual float_t phraseFreq() =0;

	//Transfers the PhrasePositions from the PhraseQueue pq to
	//the PhrasePositions list with first as its first element
	void pqToList();

	//Moves first to the end of the list
	void firstToLast();
private:
	bool doNext();
	void init();
	void sort();
};
CL_NS_END
#endif
