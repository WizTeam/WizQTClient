/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Hits.h"
#include "SearchHeader.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexReader.h"
#include "Filter.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/search/IndexSearcher.h"

CL_NS_USE(document)
CL_NS_USE(util)
CL_NS_USE(index)

CL_NS_DEF(search)



	HitDoc::HitDoc(const float_t s, const int32_t i)
	{
	//Func - Constructor
	//Pre  - true
	//Post - The instance has been created

		next  = NULL;
		prev  = NULL;
		doc   = NULL;
		score = s;
		id    = i;
	}

	HitDoc::~HitDoc(){
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

		_CLLDELETE(doc);
	}


	Hits::Hits(Searcher* s, Query* q, Filter* f, const Sort* _sort):
		query(q), searcher(s), filter(f), sort(_sort) , _length(0), first(NULL), last(NULL),
			numDocs(0), maxDocs(200), nDeletedHits(0), debugCheckedForDeletions(false)
	{
	//Func - Constructor
	//Pre  - s contains a valid reference to a searcher s
	//       q contains a valid reference to a Query
	//       f is NULL or contains a pointer to a filter
	//Post - The instance has been created

		hitDocs = _CLNEW CL_NS(util)::CLVector<HitDoc*, CL_NS(util)::Deletor::Object<HitDoc> >;
		nDeletions = countDeletions(s);

		//retrieve 100 initially
		getMoreDocs(50);

		_lengthAtStart = _length;
	}

	Hits::~Hits(){
		_CLLDELETE(hitDocs);
	}

	// count # deletions, return -1 if unknown.
	int32_t Hits::countDeletions(CL_NS(search)::Searcher* s) {
		int32_t cnt = -1;
		if ( s->getObjectName() == IndexSearcher::getClassName() ) {
			cnt = s->maxDoc() - static_cast<IndexSearcher*>(s)->getReader()->numDocs(); 
		}
		return cnt;
	}

	size_t Hits::length() const {
		return _length;
	}

	Document& Hits::doc(const int32_t n){
		HitDoc* hitDoc = getHitDoc(n);

		// Update LRU cache of documents
		remove(hitDoc);				  // remove from list, if there
		addToFront(hitDoc);				  // add to front of list
		if (numDocs > maxDocs) {			  // if cache is full
			HitDoc* oldLast = last;
			remove(last);				  // flush last

			_CLLDELETE( oldLast->doc );
			oldLast->doc = NULL;
		}

		if (hitDoc->doc == NULL){
			hitDoc->doc = _CLNEW Document;
			searcher->doc(hitDoc->id, hitDoc->doc);	  // cache miss: read document
		}

		return *hitDoc->doc;
	}

	int32_t Hits::id (const int32_t n){
		return getHitDoc(n)->id;
	}

    float_t Hits::score(const int32_t n){
		return getHitDoc(n)->score;
	}

	void Hits::getMoreDocs(const size_t m){
		size_t _min = m;
		if ( hitDocs->size() > _min)
			_min = hitDocs->size();

		size_t n = _min * 2;				  // double # retrieved
		TopDocs* topDocs = NULL;
		if ( sort==NULL )
			topDocs = (TopDocs*)((Searchable*)searcher)->_search(query, filter, n);
		else
			topDocs = (TopDocs*)((Searchable*)searcher)->_search(query, filter, n, sort);

		_length = topDocs->totalHits;
		ScoreDoc* scoreDocs = topDocs->scoreDocs;
		size_t scoreDocsLength = topDocs->scoreDocsLength;

		float_t scoreNorm = 1.0f;

		//Check that scoreDocs is a valid pointer before using it
		if (scoreDocs != NULL){
			if (_length > 0 && scoreDocs[0].score > 1.0f){
				scoreNorm = 1.0f / scoreDocs[0].score;
			}

			int32_t start = hitDocs->size() - nDeletedHits;

			// any new deletions?
			int32_t nDels2 = countDeletions(searcher);
			debugCheckedForDeletions = false;
			if (nDeletions < 0 || nDels2 > nDeletions) { 
				// either we cannot count deletions, or some "previously valid hits" might have been deleted, so find exact start point
				nDeletedHits = 0;
				debugCheckedForDeletions = true;
				size_t i2 = 0;
				for (size_t i1=0; i1<hitDocs->size() && i2 < scoreDocsLength; i1++) {
					int32_t id1 = ((*hitDocs)[i1])->id;
					int32_t id2 = scoreDocs[i2].doc;
					if (id1 == id2) {
						i2++;
					} else {
						nDeletedHits++;
					}
				}
				start = i2;
			}

			size_t end = scoreDocsLength < _length ? scoreDocsLength : _length;
			_length += nDeletedHits;
			for (size_t i = start; i < end; i++) {
				hitDocs->push_back(_CLNEW HitDoc(scoreDocs[i].score * scoreNorm, scoreDocs[i].doc));
			}

			nDeletions = nDels2;
		}

		_CLDELETE(topDocs);
	}

	HitDoc* Hits::getHitDoc(const size_t n){
		if (n >= _lengthAtStart){
		    TCHAR buf[100];
            _sntprintf(buf, 100,_T("Not a valid hit number: %d"), (int)n);
			_CLTHROWT(CL_ERR_IndexOutOfBounds, buf );
		}
		if (n >= hitDocs->size())
			getMoreDocs(n);

		if (n >= _length) {
		    TCHAR buf[100];
            _sntprintf(buf, 100,_T("Not a valid hit number: %d"), (int)n);
			_CLTHROWT(CL_ERR_ConcurrentModification, buf );
		}

		return (*hitDocs)[n];
	}

	void Hits::addToFront(HitDoc* hitDoc) {  // insert at front of cache
		if (first == NULL)
			last = hitDoc;
		else
			first->prev = hitDoc;

		hitDoc->next = first;
		first = hitDoc;
		hitDoc->prev = NULL;

		numDocs++;
	}

	void Hits::remove(const HitDoc* hitDoc) {	  // remove from cache
		if (hitDoc->doc == NULL)			  // it's not in the list
			return;					  // abort

		if (hitDoc->next == NULL)
			last = hitDoc->prev;
		else
			hitDoc->next->prev = hitDoc->prev;

		if (hitDoc->prev == NULL)
			first = hitDoc->next;
		else
			hitDoc->prev->next = hitDoc->next;

		numDocs--;
	}
	
CL_NS_END
