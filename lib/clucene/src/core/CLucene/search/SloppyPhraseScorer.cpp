/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Scorer.h"
#include "CLucene/index/Terms.h"
#include "SearchHeader.h"
#include "_PhrasePositions.h"
#include "_SloppyPhraseScorer.h"
#include "Similarity.h"

CL_NS_USE(index)
CL_NS_DEF(search)

  SloppyPhraseScorer::SloppyPhraseScorer(Weight* _weight, TermPositions** tps, int32_t* offsets,
			Similarity* similarity, int32_t _slop, uint8_t* norms):
      PhraseScorer(_weight,tps,offsets,similarity,norms),slop(_slop),repeats(NULL),repeatsLen(0){
  //Func - Constructor
  //Pre  - tps != NULL 
  //       tpsLength >= 0
  //       n != NULL
  //Post - Instance has been created

      CND_PRECONDITION(tps != NULL, "tps is NULL");
  }

  SloppyPhraseScorer::~SloppyPhraseScorer(){
	  _CLDELETE_LARRAY(repeats);
  }

  float_t SloppyPhraseScorer::phraseFreq() {

	  CND_PRECONDITION(first != NULL,"first is NULL");
	  CND_PRECONDITION(last  != NULL,"last is NULL");
	  CND_PRECONDITION(pq    != NULL,"pq is NULL");

      int32_t end = initPhrasePositions();
	  float_t freq = 0.0f;
	  bool done = (end<0);

	  while (!done) {
		  PhrasePositions* pp = pq->pop();
		  int32_t start = pp->position;
		  int32_t next = pq->top()->position;

		  bool tpsDiffer = true;
		  for (int32_t pos = start; pos <= next || !tpsDiffer; pos = pp->position) {
			  if (pos<=next && tpsDiffer)
				  start = pos;				  // advance pp to min window
			  if (!pp->nextPosition()) {
				  done = true;          // ran out of a term -- done
				  break;
			  }
			  tpsDiffer = !pp->repeats || termPositionsDiffer(pp);
		  }

		  const int32_t matchLength = end - start;
		  if (matchLength <= slop)
			  freq += getSimilarity()->sloppyFreq(matchLength); // score match

		  if (pp->position > end)
			  end = pp->position;
		  pq->put(pp);				  // restore pq

	  }
	  return freq;
  }

  int32_t SloppyPhraseScorer::initPhrasePositions() {
	  int32_t end = 0;
	  PhrasePositions* pp = NULL; // used in order to solve msvc6 scope issues

	  // no repeats at all (most common case is also the simplest one)
	  if (checkedRepeats && repeats==NULL) {
		  // build queue from list
		  pq->clear();
		  for (pp = first; pp != NULL; pp = pp->_next) {
			  pp->firstPosition();
			  if (pp->position > end)
				  end = pp->position;
			  pq->put(pp);         // build pq from list
		  }
		  return end;
	  }

	  // position the pp's
	  for (pp = first; pp != NULL; pp = pp->_next)
		  pp->firstPosition();

	  // one time initializatin for this scorer
	  if (!checkedRepeats) {
		  checkedRepeats = true;
		  // check for repeats
		  // TODO: is this correct, filtering clones using CLHashMap???
		  PhrasePositionsMap* m = NULL;
		  for (pp = first; pp != NULL; pp = pp->_next) {
			  int32_t tpPos = pp->position + pp->offset;
			  for (PhrasePositions* pp2 = pp->_next; pp2 != NULL; pp2 = pp2->_next) {
				  int32_t tpPos2 = pp2->position + pp2->offset;
				  if (tpPos2 == tpPos) { 
					  if (m == NULL)
						  m = new PhrasePositionsMap(false,false);
					  pp->repeats = true;
					  pp2->repeats = true;
					  m->put(pp,NULL);
					  m->put(pp2,NULL);
				  }
			  }
		  }
		  if (m!=NULL) {
			  repeatsLen = m->size();
			  repeats = _CL_NEWARRAY(PhrasePositions*, repeatsLen + 1);
			  PhrasePositionsMap::iterator itr = m->begin();
			  size_t pos = 0;
			  while ( itr!=m->end() ){
				  repeats[pos] = itr->first;
				  ++itr;
				  ++pos;
			  }
			  repeats[repeatsLen + 1] = NULL; // NULL terminate the array
		  }
		  delete m;
	  }

	  // with repeats must advance some repeating pp's so they all start with differing tp's       
	  if (repeats!=NULL) {
		  // must propagate higher offsets first (otherwise might miss matches).
		  qsort(repeats, repeatsLen, sizeof(PhrasePositions*), comparePhrasePositions);
		  // now advance them
		  for (size_t i = 0; i < repeatsLen; i++) {
			  PhrasePositions* pp = repeats[i];
			  while (!termPositionsDiffer(pp)) {
				  if (!pp->nextPosition())
					  return -1;    // ran out of a term -- done  
			  } 
		  }
	  }

	  // build queue from list
	  pq->clear();
	  for (pp = first; pp != NULL; pp = pp->_next) {
		  if (pp->position > end)
			  end = pp->position;
		  pq->put(pp);         // build pq from list
	  }

	  return end;
  }

  // disalow two pp's to have the same tp position, so that same word twice 
  // in query would go elswhere in the matched doc
  bool SloppyPhraseScorer::termPositionsDiffer(PhrasePositions* pp) {
	  // efficiency note: a more efficient implemention could keep a map between repeating 
	  // pp's, so that if pp1a, pp1b, pp1c are repeats term1, and pp2a, pp2b are repeats 
	  // of term2, pp2a would only be checked against pp2b but not against pp1a, pp1b, pp1c. 
	  // However this would complicate code, for a rather rare case, so choice is to compromise here.
	  const int32_t tpPos = pp->position + pp->offset;
	  for (size_t i = 0; i < repeatsLen; i++) {
		  PhrasePositions* pp2 = repeats[i];
		  if (pp2 == pp)
			  continue;
		  const int32_t tpPos2 = pp2->position + pp2->offset;
		  if (tpPos2 == tpPos)
			  return false;
	  }
	  return true;
  }

  TCHAR* SloppyPhraseScorer::toString(){
	  return stringDuplicate(_T("SloppyPhraseScorer"));
  }
CL_NS_END
