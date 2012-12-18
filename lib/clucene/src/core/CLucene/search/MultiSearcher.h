/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_multisearcher
#define _lucene_search_multisearcher


//#include "SearchHeader.h"
#include "Searchable.h"
CL_CLASS_DEF(document,Document)
CL_CLASS_DEF(index,Term)

CL_NS_DEF(search)
    
 /** Implements search over a set of <code>Searchables</code>.
	*
	* <p>Applications usually need only call the inherited {@link #search(Query)}
	* or {@link #search(Query,Filter)} methods.
	*/
	class CLUCENE_EXPORT MultiSearcher: public Searcher {
  private:
    Searchable** searchables;
		int32_t searchablesLen;
    int32_t* starts;
    int32_t _maxDoc;
	protected:
		int32_t* getStarts();
		int32_t getLength();
  public:
      /** Creates a searcher which searches <i>Searchables</i>. */
      MultiSearcher(Searchable** searchables);
      
      ~MultiSearcher();

      /** Frees resources associated with this <code>Searcher</code>. */
      void close() ;

	  int32_t docFreq(const CL_NS(index)::Term* term) const ;

      /** For use by {@link HitCollector} implementations. */
	  bool doc(int32_t n, CL_NS(document)::Document* document);

      /** For use by {@link HitCollector} implementations to identify the
       * index of the sub-searcher that a particular hit came from. */
      int32_t searcherIndex(int32_t n) const;

	  int32_t subSearcher(int32_t n) const;

	  int32_t subDoc(int32_t n) const;

      int32_t maxDoc() const;
    
      TopDocs* _search(Query* query, Filter* filter, const int32_t nDocs) ;
      
      TopFieldDocs* _search (Query* query, Filter* filter, const int32_t n, const Sort* sort);
     
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
        void _search(Query* query, Filter* filter, HitCollector* results);

		Query* rewrite(Query* original);
		void explain(Query* query, int32_t doc, Explanation* ret);
    };

CL_NS_END
#endif
