/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "SearchHeader.h"
#include "_BooleanScorer.h"

#include "Scorer.h"
#include "Similarity.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_USE(util)
CL_NS_DEF(search)

   BooleanScorer::BooleanScorer(Similarity* similarity, int32_t minNrShouldMatch ):
    Scorer(similarity),
    scorers(NULL),
    maxCoord(1),
    nextMask(1),
	end(0),
	current(NULL),
	minNrShouldMatch(minNrShouldMatch),
    requiredMask(0),
    prohibitedMask(0),
	coordFactors(NULL)
  {
    bucketTable = _CLNEW BucketTable(this);
  }

  BooleanScorer::~BooleanScorer(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      _CLDELETE(bucketTable);
	  	_CLDELETE_ARRAY(coordFactors);
      _CLDELETE(scorers);
  }


  bool BooleanScorer::next() {
	bool more;
	do {
		while (bucketTable->first != NULL) {         // more queued
			current = bucketTable->first;
			bucketTable->first = current->next;         // pop the queue

			// check prohibited & required
			if ((current->bits & prohibitedMask) == 0 &&
				(current->bits & requiredMask) == requiredMask) {
			return true;
		}
	}

	// refill the queue
	more = false;
	end += BooleanScorer::BucketTable_SIZE;
	for (SubScorer* sub = scorers; sub != NULL; sub = sub->next) {
		Scorer* scorer = sub->scorer;
			int32_t doc;
			while (!sub->done && (doc=scorer->doc()) < end) {
				sub->collector->collect(doc, scorer->score());
			sub->done = !scorer->next();
		}
		if (!sub->done) {
			more = true;
		}
	}
	} while (bucketTable->first != NULL || more);

	return false;
  }

	float_t BooleanScorer::score(){
		if (coordFactors == NULL)
			computeCoordFactors();
		return current->score * coordFactors[current->coord];
	}

	void BooleanScorer::score( HitCollector* results ) {
		next();
		score( results, LUCENE_INT32_MAX_SHOULDBE );
	}

	bool BooleanScorer::skipTo(int32_t /*target*/) {
		_CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: BooleanScorer::skipTo");
	}

	Explanation* BooleanScorer::explain(int32_t /*doc*/) {
		_CLTHROWA(CL_ERR_UnsupportedOperation,"UnsupportedOperationException: BooleanScorer::explain");
	}

	TCHAR* BooleanScorer::toString() {
		CL_NS(util)::StringBuffer buffer;
		buffer.append(_T("boolean("));
		for (SubScorer* sub = scorers; sub != NULL; sub = sub->next) {
			TCHAR* tmp = sub->scorer->toString();
			buffer.append(tmp);
			_CLDELETE_LCARRAY(tmp);
			buffer.appendChar(_T(' '));
		}
		buffer.appendChar(_T(')'));
		return buffer.toString();
	}

  void BooleanScorer::add(Scorer* scorer, const bool required, const bool prohibited) {
    int32_t mask = 0;
    if (required || prohibited) {
      if (nextMask == 0)
        _CLTHROWA(CL_ERR_IndexOutOfBounds, "More than 32 required/prohibited clauses in query.");
      mask = nextMask;
      nextMask = ( nextMask << 1 );
    } else
      mask = 0;

    if (!prohibited)
      maxCoord++;

    if (prohibited)
      prohibitedMask |= mask;			  // update prohibited mask
    else if (required)
      requiredMask |= mask;			  // update required mask

    //scorer, HitCollector, and scorers is delete in the SubScorer
    scorers = _CLNEW SubScorer(scorer, required, prohibited,
    bucketTable->newCollector(mask), scorers);
  }

  void BooleanScorer::computeCoordFactors(){
    coordFactors = _CL_NEWARRAY(float_t,maxCoord);
    for (int32_t i = 0; i < maxCoord; i++)
      coordFactors[i] = getSimilarity()->coord(i, maxCoord-1);
  }

  bool BooleanScorer::score( HitCollector* results, const int32_t maxDoc ) {
    if ( coordFactors == NULL ) {
    	computeCoordFactors();
    }

    bool more;
    Bucket* tmp;

    do {
    	bucketTable->first = NULL;
    	while ( current != NULL ) {

    		if (( current->bits & prohibitedMask ) == 0 &&
    		    ( current->bits & requiredMask ) == requiredMask ) {

    			if ( current->doc >= maxDoc ) {
    				tmp = current;
    				current = current->next;
    				tmp->next = bucketTable->first;
    				bucketTable->first = tmp;
    				continue;
    			}

    			if ( current->coord >= minNrShouldMatch ) {
    				results->collect( current->doc, current->score * coordFactors[current->coord] );
    			}
    		}

    		current = current->next;
    	}

    	if ( bucketTable->first != NULL ) {
    		current = bucketTable->first;
    		bucketTable->first = current->next;
    		return true;
    	}

    	more = false;
    	end += BucketTable_SIZE;

    	for ( SubScorer* sub = scorers; sub != NULL; sub = sub->next ) {
    		if ( !sub->done ) {
    			sub->done = !sub->scorer->score( sub->collector, end );
    			if ( !sub->done )
    				more = true;
    		}
    	}
    	current = bucketTable->first;

    } while ( current != NULL || more );

    return false;
  }



  BooleanScorer::SubScorer::SubScorer(Scorer* scr, const bool r, const bool p, HitCollector* c, SubScorer* nxt):
      scorer(scr),
      required(r),
      prohibited(p),
      collector(c),
      next(nxt)
  {
  //Func - Constructor
  //Pre  - scr != NULL,
  //       c   != NULL
  //       nxt may or may not be NULL
  //Post - The instance has been created

      CND_PRECONDITION(scr != NULL,"scr is NULL");
      CND_PRECONDITION(c != NULL,"c is NULL");

      done        = !scorer->next();
  }

  BooleanScorer::SubScorer::~SubScorer(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

	for (SubScorer * ptr = next; ptr; ){
		SubScorer* next = ptr->next;
		ptr->next = NULL;
		_CLDELETE(ptr);
		ptr = next;
	}
	_CLDELETE(scorer);
	_CLDELETE(collector);
  }

  BooleanScorer::Bucket::Bucket():
      doc(-1),
      score(0.0),
      bits(0),
      coord(0),
      next(NULL)
  {
  }
  BooleanScorer::Bucket::~Bucket(){
  }




  BooleanScorer::BucketTable::BucketTable(BooleanScorer* scr):
    scorer(scr),
    first(NULL)
  {
		buckets = new Bucket[BucketTable_SIZE];
  }
  BooleanScorer::BucketTable::~BucketTable(){
		clear();
		delete [] buckets;
  }

  void BooleanScorer::BucketTable::clear(){
    //delete first;
    first = NULL;
  }
  int32_t BooleanScorer::BucketTable::size() const { return BooleanScorer::BucketTable_SIZE; }

  HitCollector* BooleanScorer::BucketTable::newCollector(const int32_t mask) {
    return _CLNEW Collector(mask, this);
  }









  BooleanScorer::Collector::Collector(const int32_t msk, BucketTable* bucketTbl):
    bucketTable(bucketTbl),
    mask(msk)
  {
  }

  void BooleanScorer::Collector::collect(const int32_t doc, const float_t score){
    BucketTable* table = bucketTable;
    int32_t i = doc & (BooleanScorer::BucketTable_SIZE-1);
    Bucket* bucket = &table->buckets[i];

    if (bucket->doc != doc) {			  // invalid bucket
      bucket->doc = doc;			  // set doc
      bucket->score = score;			  // initialize score
      bucket->bits = mask;			  // initialize mask
      bucket->coord = 1;			  // initialize coord

      bucket->next = table->first;		  // push onto valid list
      table->first = bucket;
    } else {					  // valid bucket
      bucket->score += score;			  // increment score
      bucket->bits |= mask;			  // add bits in mask
      bucket->coord++;				  // increment coord
    }
  }



CL_NS_END
