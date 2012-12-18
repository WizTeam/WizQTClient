/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/IndexReader.h"
#include "MultiSearcher.h"
#include "SearchHeader.h"
#include "Query.h"
#include "_HitQueue.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/Term.h"
#include "_FieldDocSortedHitQueue.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)

CL_NS_DEF(search)

    class MultiHitCollector: public HitCollector{
    private:
      HitCollector* results;
      int32_t start;
    public: 
      MultiHitCollector(HitCollector* _results, int32_t _start);
      void collect(const int32_t doc, const float_t score) ;
    };
    

  /** Creates a searcher which searches <i>searchers</i>. */
  MultiSearcher::MultiSearcher(Searchable** _searchables):
		_maxDoc(0) {
	searchablesLen = 0;
	while ( _searchables[searchablesLen] != NULL )
		++searchablesLen;

    searchables=_CL_NEWARRAY(Searchable*,searchablesLen+1);
    starts = _CL_NEWARRAY(int32_t,searchablesLen + 1);	  // build starts array
		for (int32_t i = 0; i < searchablesLen; ++i) {
	  searchables[i]=_searchables[i];
      starts[i] = _maxDoc; 
      _maxDoc += searchables[i]->maxDoc();		  // compute maxDocs
    }
    starts[searchablesLen] = _maxDoc;
  }

  MultiSearcher::~MultiSearcher() {
    _CLDELETE_ARRAY(searchables);
    _CLDELETE_ARRAY(starts);
  }

	int32_t* MultiSearcher::getStarts() {
		return starts;
	}
	int32_t MultiSearcher::getLength() {
		return searchablesLen;
	}

  // inherit javadoc
  void MultiSearcher::close() {
	for (int32_t i = 0; i < searchablesLen; ++i){
      searchables[i]->close();
      searchables[i]=NULL;
     }
  }

  int32_t MultiSearcher::docFreq(const Term* term) const {
    int32_t docFreq = 0;
	for (int32_t i = 0; i < searchablesLen; ++i)
      docFreq += searchables[i]->docFreq(term);
    return docFreq;
  }

  /** For use by {@link HitCollector} implementations. */
  bool MultiSearcher::doc(int32_t n, Document* d) {
    int32_t i = subSearcher(n);			  // find searcher index
    return searchables[i]->doc(n - starts[i], d);	  // dispatch to searcher
  }

  int32_t MultiSearcher::searcherIndex(int32_t n) const{
	 return subSearcher(n);
  }

  /** Returns index of the searcher for document <code>n</code> in the array
   * used to construct this searcher. */
  int32_t MultiSearcher::subSearcher(int32_t n) const{
    // replace w/ call to Arrays.binarySearch in Java 1.2
    int32_t lo = 0;					  // search starts array
    int32_t hi = searchablesLen - 1;		  // for first element less
              // than n, return its index
	int32_t mid,midValue;
    while (hi >= lo) {
		mid = (lo + hi) >> 1;
		midValue = starts[mid];
      if (n < midValue)
		hi = mid - 1;
      else if (n > midValue)
		lo = mid + 1;
      else{  // found a match
        while (mid+1 < searchablesLen && starts[mid+1] == midValue) {
				++mid;  // scan to last match
        }
		return mid;
	  }
    }
    return hi;
  }

  /** Returns the document number of document <code>n</code> within its
   * sub-index. */
  int32_t MultiSearcher::subDoc(int32_t n)  const{
    return n - starts[subSearcher(n)];
  }

  int32_t MultiSearcher::maxDoc() const{
    return _maxDoc;
  }

  TopDocs* MultiSearcher::_search(Query* query, Filter* filter, const int32_t nDocs) {
    HitQueue* hq = _CLNEW HitQueue(nDocs);
    int32_t totalHits = 0;
	TopDocs* docs;
	int32_t j;
	ScoreDoc* scoreDocs;
    for (int32_t i = 0; i < searchablesLen; i++) {  // search each searcher
		docs = searchables[i]->_search(query, filter, nDocs);
		totalHits += docs->totalHits;		  // update totalHits
		scoreDocs = docs->scoreDocs;
		for ( j = 0; j <docs->scoreDocsLength; ++j) { // merge scoreDocs int_to hq
			scoreDocs[j].doc += starts[i];		  // convert doc
			if ( !hq->insert(scoreDocs[j]))
				break;				  // no more scores > minScore
		}
		
		_CLDELETE(docs);
    }

    int32_t scoreDocsLen = hq->size();
		scoreDocs = new ScoreDoc[scoreDocsLen];
	{//MSVC 6 scope fix
		for (int32_t i = scoreDocsLen-1; i >= 0; --i)	  // put docs in array
	  		scoreDocs[i] = hq->pop();
	}

	//cleanup
	_CLDELETE(hq);

    return _CLNEW TopDocs(totalHits, scoreDocs, scoreDocsLen);
  }

  /** Lower-level search API.
   *
   * <p>{@link HitCollector#collect(int32_t,float_t)} is called for every non-zero
   * scoring document.
   *
   * <p>Applications should only use this if they need <i>all</i> of the
   * matching documents.  The high-level search API ({@link
   * Searcher#search(Query)}) is usually more efficient, as it skips
   * non-high-scoring hits.
   *
   * @param query to match documents
   * @param filter if non-null, a bitset used to eliminate some documents
   * @param results to receive hits
   */
  void MultiSearcher::_search(Query* query, Filter* filter, HitCollector* results){
	for (int32_t i = 0; i < searchablesLen; ++i) {
      /* DSR:CL_BUG: Old implementation leaked and was misconceived.  We need
      ** to have the original HitCollector ($results) collect *all* hits;
      ** the MultiHitCollector instantiated below serves only to adjust
      ** (forward by starts[i]) the docNo passed to $results.
      ** Old implementation instead created a sort of linked list of
      ** MultiHitCollectors that applied the adjustments in $starts
      ** cumulatively (and was never deleted). */
      HitCollector *docNoAdjuster = _CLNEW MultiHitCollector(results, starts[i]);
      searchables[i]->_search(query, filter, docNoAdjuster);
      _CLDELETE(docNoAdjuster);
    }
  }

  TopFieldDocs* MultiSearcher::_search (Query* query, Filter* filter, const int32_t n, const Sort* sort){
    FieldDocSortedHitQueue* hq = NULL;
    int32_t totalHits = 0;
	TopFieldDocs* docs;
	int32_t j;
	FieldDoc** fieldDocs;

	for (int32_t i = 0; i < searchablesLen; ++i) { // search each searcher
		docs = searchables[i]->_search (query, filter, n, sort);
		if (hq == NULL){
			hq = _CLNEW FieldDocSortedHitQueue (docs->fields, n);
			docs->fields = NULL; //hit queue takes fields memory
		}

      totalHits += docs->totalHits;		  // update totalHits
      fieldDocs = docs->fieldDocs;
	  for(j = 0;j<docs->scoreDocsLength;++j){ // merge scoreDocs into hq
		fieldDocs[j]->scoreDoc.doc += starts[i];                // convert doc
		if (!hq->insert (fieldDocs[j]) )
			break;                                  // no more scores > minScore
      }
	  for ( int32_t x=0;x<j;++x )
			fieldDocs[x]=NULL; //move ownership of FieldDoc to the hitqueue

	  _CLDELETE(docs);
    }

    int32_t hqlen = hq->size();
	fieldDocs = _CL_NEWARRAY(FieldDoc*,hqlen);
	for (j = hqlen - 1; j >= 0; j--)	  // put docs in array
      fieldDocs[j] = hq->pop();

	SortField** hqFields = hq->getFields();
	hq->setFields(NULL); //move ownership of memory over to TopFieldDocs
    _CLDELETE(hq);

    return _CLNEW TopFieldDocs (totalHits, fieldDocs, hqlen, hqFields);
  }

  Query* MultiSearcher::rewrite(Query* query) {
    // this is a bit of a hack. We know that a query which
    // creates a Weight based on this Dummy-Searcher is
    // always already rewritten (see preparedWeight()).
    // Therefore we just return the unmodified query here
    return query;
  }

  void MultiSearcher::explain(Query* query, int32_t doc, Explanation* ret) {
    int32_t i = subSearcher(doc);			  // find searcher index
    searchables[i]->explain(query,doc-starts[i], ret); // dispatch to searcher
  }

  MultiHitCollector::MultiHitCollector(HitCollector* _results, int32_t _start):
  results(_results),
	start(_start) {
  }

  void MultiHitCollector::collect(const int32_t doc, const float_t score) {
    results->collect(doc + start, score);
  }

CL_NS_END
