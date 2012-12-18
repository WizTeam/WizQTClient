/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Hits_h
#define _lucene_search_Hits_h

#include "CLucene/util/VoidList.h"
CL_CLASS_DEF(index,Term)
CL_CLASS_DEF(document,Document)

CL_NS_DEF(search)

	class Query;
	class Searcher;
	class Filter;
	class HitDoc;
	class Sort;

	/** A ranked list of documents, used to hold search results.
	* <p>
	* <b>Caution:</b> Iterate only over the hits needed.  Iterating over all
	* hits is generally not desirable and may be the source of
	* performance issues. If you need to iterate over many or all hits, consider
	* using the search method that takes a {@link HitCollector}.
	* </p>
	* <p><b>Note:</b> Deleting matching documents concurrently with traversing 
	* the hits, might, when deleting hits that were not yet retrieved, decrease
	* {@link #length()}. In such case, 
	* {@link java.util.ConcurrentModificationException ConcurrentModificationException}
	* is thrown when accessing hit <code>n</code> &ge; current_{@link #length()} 
	* (but <code>n</code> &lt; {@link #length()}_at_start). 
	*/
   class CLUCENE_EXPORT Hits {
    private:
	    Query* query;
		Searcher* searcher;
		Filter* filter;
		const Sort* sort;

		size_t _length;				  // the total number of hits
		CL_NS(util)::CLVector<HitDoc*, CL_NS(util)::Deletor::Object<HitDoc> >* hitDocs;	  // cache of hits retrieved

		HitDoc* first;				  // head of LRU cache
		HitDoc* last;				  // tail of LRU cache
		int32_t numDocs;			  // number cached
		int32_t maxDocs;			  // max to cache

		int32_t nDeletions;       // # deleted docs in the index.    
		size_t _lengthAtStart;    // this is the number apps usually count on (although deletions can bring it down). 
		int32_t nDeletedHits;    // # of already collected hits that were meanwhile deleted.

		bool debugCheckedForDeletions; // for test purposes.

		/**
		* Tries to add new documents to hitDocs.
		* Ensures that the hit numbered <code>_min</code> has been retrieved.
		*/
		void getMoreDocs(const size_t _min);
	    
		/** Returns the score for the n<sup>th</sup> document in this set. */
		HitDoc* getHitDoc(const size_t n);
	    
		void addToFront(HitDoc* hitDoc);
	    
		void remove(const HitDoc* hitDoc);

    public:
		Hits(Searcher* s, Query* q, Filter* f, const Sort* sort=NULL);
		virtual ~Hits();

		/** Returns the total number of hits available in this set. */
		size_t length() const;
	    
		/** Returns the stored fields of the n<sup>th</sup> document in this set.
		* <p>Documents are cached, so that repeated requests for the same element may
		* return the same Document object.
		* @throws CorruptIndexException if the index is corrupt
		* @throws IOException if there is a low-level IO error
		*
		* @memory Memory belongs to the hits object. Don't delete the return value.
		*/
		CL_NS(document)::Document& doc(const int32_t n);
	      
		/** Returns the id for the n<sup>th</sup> document in this set.
		* Note that ids may change when the index changes, so you cannot
		* rely on the id to be stable.
		*/
		int32_t id (const int32_t n);
	    
		/** Returns the score for the n<sup>th</sup> document in this set. */
		float_t score(const int32_t n);

		/** count # deletions, return -1 if unknown. */
		int32_t countDeletions(CL_NS(search)::Searcher* s);
  };

CL_NS_END
#endif
