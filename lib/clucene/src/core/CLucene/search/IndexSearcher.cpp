/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "IndexSearcher.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "_HitQueue.h"
#include "Query.h"
#include "Filter.h"
#include "_FieldDocSortedHitQueue.h"
#include "CLucene/store/Directory.h"
#include "CLucene/document/Document.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/BitSet.h"
#include "FieldSortedHitQueue.h"
#include "Explanation.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)

CL_NS_DEF(search)

	class SimpleTopDocsCollector:public HitCollector{ 
	private:
		float_t minScore;
		const CL_NS(util)::BitSet* bits;
		HitQueue* hq;
		size_t nDocs;
		int32_t* totalHits;
	public:
		SimpleTopDocsCollector(const CL_NS(util)::BitSet* bs, HitQueue* hitQueue, int32_t* totalhits, size_t ndocs, const float_t ms=-1.0f):
    		minScore(ms),
    		bits(bs),
    		hq(hitQueue),
    		nDocs(ndocs),
    		totalHits(totalhits)
    	{
    	}
		~SimpleTopDocsCollector(){}
		void collect(const int32_t doc, const float_t score){
    		if (score > 0.0f &&			  // ignore zeroed buckets
    			(bits==NULL || bits->get(doc))) {	  // skip docs not in bits
    			++totalHits[0];
    			if (hq->size() < nDocs || (minScore==-1.0f || score >= minScore)) {
    				ScoreDoc sd = {doc, score};
    				hq->insert(sd);	  // update hit queue
    				if ( minScore != -1.0f )
    					minScore = hq->top().score; // maintain minScore
    			}
    		}
    	}
	};

	class SortedTopDocsCollector:public HitCollector{ 
	private:
		const CL_NS(util)::BitSet* bits;
		FieldSortedHitQueue* hq;
		size_t nDocs;
		int32_t* totalHits;
	public:
		SortedTopDocsCollector(const CL_NS(util)::BitSet* bs, FieldSortedHitQueue* hitQueue, int32_t* totalhits, size_t _nDocs):
    		bits(bs),
    		hq(hitQueue),
    		nDocs(_nDocs),
    		totalHits(totalhits)
    	{
    	}
		~SortedTopDocsCollector(){
		}
		void collect(const int32_t doc, const float_t score){
    		if (score > 0.0f &&			  // ignore zeroed buckets
    			(bits==NULL || bits->get(doc))) {	  // skip docs not in bits
    			++totalHits[0];
    			FieldDoc* fd = _CLNEW FieldDoc(doc, score); //todo: see jlucene way... with fields def???
    			if ( !hq->insert(fd) )	  // update hit queue
    				_CLDELETE(fd);
    		}
    	}
	};

	class SimpleFilteredCollector: public HitCollector{
	private:
		CL_NS(util)::BitSet* bits;
		HitCollector* results;
	public:
		SimpleFilteredCollector(CL_NS(util)::BitSet* bs, HitCollector* collector):
            bits(bs),
            results(collector)
        {
        }
		~SimpleFilteredCollector(){
		}
	protected:
		void collect(const int32_t doc, const float_t score){
            if (bits->get(doc)) {		  // skip docs not in bits
                results->collect(doc, score);
            }
        }
	};


  IndexSearcher::IndexSearcher(const char* path){
  //Func - Constructor
  //       Creates a searcher searching the index in the named directory.  */
  //Pre  - path != NULL
  //Post - The instance has been created

      CND_PRECONDITION(path != NULL, "path is NULL");

      reader = IndexReader::open(path);
      readerOwner = true;
  }
  
  IndexSearcher::IndexSearcher(CL_NS(store)::Directory* directory){
  //Func - Constructor
  //       Creates a searcher searching the index in the specified directory.  */
  //Pre  - path != NULL
  //Post - The instance has been created

      CND_PRECONDITION(directory != NULL, "directory is NULL");

      reader = IndexReader::open(directory);
      readerOwner = true;
  }

  IndexSearcher::IndexSearcher(IndexReader* r){
  //Func - Constructor
  //       Creates a searcher searching the index with the provide IndexReader
  //Pre  - path != NULL
  //Post - The instance has been created

      reader      = r;
      readerOwner = false;
  }

  IndexSearcher::~IndexSearcher(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

	  close();
  }

  void IndexSearcher::close(){
  //Func - Frees resources associated with this Searcher.
  //Pre  - true
  //Post - The resources associated have been freed
      if (readerOwner && reader){
          reader->close();
          _CLDELETE(reader);
      }
  }

  // inherit javadoc
  int32_t IndexSearcher::docFreq(const Term* term) const{
  //Func - 
  //Pre  - reader != NULL
  //Post -

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->docFreq(term);
  }

  _CL_DEPRECATED( doc(i, document) ) CL_NS(document)::Document* IndexSearcher::doc(int32_t i){
	CL_NS(document)::Document* ret = _CLNEW CL_NS(document)::Document;
	if (!doc(i,ret) )
		_CLDELETE(ret);
	return ret;
  }
  
  // inherit javadoc
  bool IndexSearcher::doc(int32_t i, CL_NS(document)::Document& d) {
  //Func - Retrieves i-th document found
  //       For use by HitCollector implementations.
  //Pre  - reader != NULL
  //Post - The i-th document has been returned

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->document(i,d);
  }
  bool IndexSearcher::doc(int32_t i, CL_NS(document)::Document* d) {
  //Func - Retrieves i-th document found
  //       For use by HitCollector implementations.
  //Pre  - reader != NULL
  //Post - The i-th document has been returned

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->document(i,*d);
  }

  // inherit javadoc
  int32_t IndexSearcher::maxDoc() const {
  //Func - Return total number of documents including the ones marked deleted
  //Pre  - reader != NULL
  //Post - The total number of documents including the ones marked deleted 
  //       has been returned

      CND_PRECONDITION(reader != NULL, "reader is NULL");

      return reader->maxDoc();
  }

  //todo: find out why we are passing Query* and not Weight*, as Weight is being extracted anyway from Query*
  TopDocs* IndexSearcher::_search(Query* query, Filter* filter, const int32_t nDocs){
  //Func -
  //Pre  - reader != NULL
  //Post -

      CND_PRECONDITION(reader != NULL, "reader is NULL");
      CND_PRECONDITION(query != NULL, "query is NULL");

      Weight* weight = query->weight(this);
      Scorer* scorer = weight->scorer(reader);
      if (scorer == NULL) {
        Query* wq = weight->getQuery();
        if (wq != query)
          _CLLDELETE(wq);
        _CLLDELETE(weight);
          return _CLNEW TopDocs(0, NULL, 0);
      }

      BitSet* bits = filter != NULL ? filter->bits(reader) : NULL;
      HitQueue* hq = _CLNEW HitQueue(nDocs);

		  //Check hq has been allocated properly
		  CND_CONDITION(hq != NULL, "Could not allocate memory for HitQueue hq");
	
		  int32_t* totalHits = _CL_NEWARRAY(int32_t,1);
      totalHits[0] = 0;

      SimpleTopDocsCollector hitCol(bits,hq,totalHits,nDocs,0.0f);
      scorer->score( &hitCol );
      _CLDELETE(scorer);

      int32_t scoreDocsLength = hq->size();

		ScoreDoc* scoreDocs = new ScoreDoc[scoreDocsLength];

		for (int32_t i = scoreDocsLength-1; i >= 0; --i)	  // put docs in array
			scoreDocs[i] = hq->pop();

      int32_t totalHitsInt = totalHits[0];

      _CLDELETE(hq);
		  if ( bits != NULL && filter->shouldDeleteBitSet(bits) )
				_CLDELETE(bits);
	    _CLDELETE_ARRAY(totalHits);
		  Query* wq = weight->getQuery();
		  if ( query != wq ) //query was re-written
			  _CLLDELETE(wq);
		  _CLDELETE(weight);

      return _CLNEW TopDocs(totalHitsInt, scoreDocs, scoreDocsLength);
  }

  // inherit javadoc
  TopFieldDocs* IndexSearcher::_search(Query* query, Filter* filter, const int32_t nDocs,
         const Sort* sort) {
             
      CND_PRECONDITION(reader != NULL, "reader is NULL");
      CND_PRECONDITION(query != NULL, "query is NULL");

    Weight* weight = query->weight(this);
    Scorer* scorer = weight->scorer(reader);
    if (scorer == NULL){
		return _CLNEW TopFieldDocs(0, NULL, 0, NULL );
	}

    BitSet* bits = filter != NULL ? filter->bits(reader) : NULL;
    FieldSortedHitQueue hq(reader, sort->getSort(), nDocs);
    int32_t* totalHits = _CL_NEWARRAY(int32_t,1);
	totalHits[0]=0;
    
	SortedTopDocsCollector hitCol(bits,&hq,totalHits,nDocs);
	scorer->score(&hitCol);
    _CLLDELETE(scorer);

	int32_t hqLen = hq.size();
    FieldDoc** fieldDocs = _CL_NEWARRAY(FieldDoc*,hqLen);
	for (int32_t i = hqLen-1; i >= 0; --i){	  // put docs in array
	  fieldDocs[i] = hq.fillFields (hq.pop());
	}

    Query* wq = weight->getQuery();
	if ( query != wq ) //query was re-written
		_CLLDELETE(wq);
	_CLLDELETE(weight);

    SortField** hqFields = hq.getFields();
	hq.setFields(NULL); //move ownership of memory over to TopFieldDocs
    int32_t totalHits0 = totalHits[0];
	if ( bits != NULL && filter->shouldDeleteBitSet(bits) )
		_CLLDELETE(bits);
    _CLDELETE_LARRAY(totalHits);
    return _CLNEW TopFieldDocs(totalHits0, fieldDocs, hqLen, hqFields );
  }

  void IndexSearcher::_search(Query* query, Filter* filter, HitCollector* results){
  //Func - _search an index and fetch the results
  //       Applications should only use this if they need all of the
  //       matching documents.  The high-level search API (search(Query)) is usually more efficient, 
  //       as it skips non-high-scoring hits.
  //Pre  - query is a valid reference to a query
  //       filter may or may not be NULL
  //       results is a valid reference to a HitCollector and used to store the results
  //Post - filter if non-NULL, a bitset used to eliminate some documents

      CND_PRECONDITION(reader != NULL, "reader is NULL");
      CND_PRECONDITION(query != NULL, "query is NULL");

      BitSet* bits = NULL;
      SimpleFilteredCollector* fc = NULL; 

      if (filter != NULL){
          bits = filter->bits(reader);
          fc = _CLNEW SimpleFilteredCollector(bits, results);
       }

      Weight* weight = query->weight(this);
      Scorer* scorer = weight->scorer(reader);
      if (scorer != NULL) {
		  if (fc == NULL){
              scorer->score(results);
		  }else{
              scorer->score((HitCollector*)fc);
		  }
          _CLDELETE(scorer); 
      }

    _CLLDELETE(fc);
	Query* wq = weight->getQuery();
	if (wq != query) // query was rewritten
		_CLLDELETE(wq);
	_CLLDELETE(weight);
	if ( bits != NULL && filter->shouldDeleteBitSet(bits) )
		_CLLDELETE(bits);
  }

  Query* IndexSearcher::rewrite(Query* original) {
        Query* query = original;
		Query* last = original;
        for (Query* rewrittenQuery = query->rewrite(reader); 
				rewrittenQuery != query;
				rewrittenQuery = query->rewrite(reader)) {
			query = rewrittenQuery;
			if ( query != last && last != original ){
				_CLLDELETE(last);
			}
			last = query;
        }
        return query;
    }

    void IndexSearcher::explain(Query* query, int32_t doc, Explanation* ret){
        Weight* weight = query->weight(this);
        ret->addDetail(weight->explain(reader, doc)); // TODO: A hack until this function will return Explanation* as well

        Query* wq = weight->getQuery();
	    if ( query != wq ) //query was re-written
		  _CLLDELETE(wq);
        _CLDELETE(weight);
    }

	CL_NS(index)::IndexReader* IndexSearcher::getReader(){
		return reader;
	}

	const char* IndexSearcher::getClassName(){
		return "IndexSearcher";
	}
	const char* IndexSearcher::getObjectName() const{
		return IndexSearcher::getClassName();
	}
	
CL_NS_END
