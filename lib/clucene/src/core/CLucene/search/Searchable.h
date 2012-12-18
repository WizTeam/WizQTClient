/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Searcher_
#define _lucene_search_Searcher_


//#include "CLucene/index/IndexReader.h"
CL_CLASS_DEF(index,Term)
//#include "Filter.h"
CL_CLASS_DEF(document,Document)
//#include "Sort.h"
//#include "CLucene/util/VoidList.h"
//#include "Explanation.h"
//#include "Similarity.h"

CL_NS_DEF(search)

	//predefine classes
	class Query;
	class Filter;
	class HitCollector;
	class TopDocs;
	class Explanation;
	class Hits;
	class Similarity;
	class TopFieldDocs;
	class Sort;
	

   /** The interface for search implementations.
   *
   * <p>Implementations provide search over a single index, over multiple
   * indices, and over indices on remote servers.
   */
   class CLUCENE_EXPORT Searchable: LUCENE_BASE {
   public:
	  virtual ~Searchable();

      /** Lower-level search API.
      *
      * <p>{@link HitCollector#collect(int32_t,float_t)} is called for every non-zero
      * scoring document.
      *
      * <p>Applications should only use this if they need <i>all</i> of the
      * matching documents.  The high-level search API ({@link
      * Searcher#search(Query*)}) is usually more efficient, as it skips
      * non-high-scoring hits.
      *
      * @param query to match documents
      * @param filter if non-null, a bitset used to eliminate some documents
      * @param results to receive hits
      */
      virtual void _search(Query* query, Filter* filter, HitCollector* results) = 0;

      /** Frees resources associated with this Searcher.
      * Be careful not to call this method while you are still using objects
      * like {@link Hits}.
      */
      virtual void close() = 0;

      /** Expert: Returns the number of documents containing <code>term</code>.
      * Called by search code to compute term weights.
      * @see IndexReader#docFreq(Term).
      */
      virtual int32_t docFreq(const CL_NS(index)::Term* term) const = 0;

      /** Expert: Returns one greater than the largest possible document number.
      * Called by search code to compute term weights.
      * @see IndexReader#maxDoc().
      */
      virtual int32_t maxDoc() const = 0;

      /** Expert: Low-level search implementation.  Finds the top <code>n</code>
      * hits for <code>query</code>, applying <code>filter</code> if non-null.
      *
      * <p>Called by {@link Hits}.
      *
      * <p>Applications should usually call {@link Searcher#search(Query*)} or
      * {@link Searcher#search(Query*,Filter*)} instead.
      */
      virtual TopDocs* _search(Query* query, Filter* filter, const int32_t n) = 0;

      /** Expert: Returns the stored fields of document <code>i</code>.
      * Called by {@link HitCollector} implementations.
      * @see IndexReader#document(int32_t).
      */
      virtual bool doc(int32_t i, CL_NS(document)::Document* d) = 0;
      _CL_DEPRECATED( doc(i, document) ) CL_NS(document)::Document* doc(const int32_t i);

      /** Expert: called to re-write queries into primitive queries. */
      virtual Query* rewrite(Query* query) = 0;

      /** Returns an Explanation that describes how <code>doc</code> scored against
      * <code>query</code>.
      *
      * <p>This is intended to be used in developing Similarity implementations,
      * and, for good performance, should not be displayed with every hit.
      * Computing an explanation is as expensive as executing the query over the
      * entire index.
      */
      virtual void explain(Query* query, int32_t doc, Explanation* ret) = 0;

      /** Expert: Low-level search implementation with arbitrary sorting.  Finds
      * the top <code>n</code> hits for <code>query</code>, applying
      * <code>filter</code> if non-null, and sorting the hits by the criteria in
      * <code>sort</code>.
      *
      * <p>Applications should usually call {@link
      * Searcher#search(Query,Filter,Sort)} instead.
      */
	  	virtual TopFieldDocs* _search(Query* query, Filter* filter, const int32_t n, const Sort* sort) = 0;
   };



	/** An abstract base class for search implementations.
	* Implements some common utility methods.
	*/
	class CLUCENE_EXPORT Searcher:public Searchable {
	private:
		/** The Similarity implementation used by this searcher. */
		Similarity* similarity;
    public:
		Searcher();
		virtual ~Searcher();

		// Returns the documents matching <code>query</code>.
		Hits* search(Query* query);

		// Returns the documents matching <code>query</code> and
		//	<code>filter</code>. 
		Hits* search(Query* query, Filter* filter);

		/** Returns documents matching <code>query</code> sorted by
		* <code>sort</code>.
		*/
		Hits* search(Query* query, const Sort* sort);

		/** Returns documents matching <code>query</code> and <code>filter</code>,
			* sorted by <code>sort</code>.
			*/
		Hits* search(Query* query, Filter* filter, const Sort* sort);

		/** Lower-level search API.
		*
		* <p>{@link HitCollector#collect(int32_t	,float_t)} is called for every non-zero
		* scoring document.
		*
		* <p>Applications should only use this if they need <i>all</i> of the
		* matching documents.  The high-level search API ({@link
		* Searcher#search(Query*)}) is usually more efficient, as it skips
		* non-high-scoring hits.
		* <p>Note: The <code>score</code> passed to this method is a raw score.
		* In other words, the score will not necessarily be a float whose value is
		* between 0 and 1.
		*/
		void _search(Query* query, HitCollector* results);

		/** Expert: Set the Similarity implementation used by this Searcher.
		*
		* @see Similarity#setDefault(Similarity)
		*/
		void setSimilarity(Similarity* similarity);

		/** Expert: Return the Similarity implementation used by this Searcher.
		*
		* <p>This defaults to the current value of {@link Similarity#getDefault()}.
		*/
		Similarity* getSimilarity();

		virtual const char* getObjectName() const;
		static const char* getClassName();

	    virtual void _search(Query* query, Filter* filter, HitCollector* results) = 0;
	};

CL_NS_END
#endif
