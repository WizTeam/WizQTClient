/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_SearchHeader_
#define _lucene_search_SearchHeader_


//#include "CLucene/index/IndexReader.h"
CL_CLASS_DEF(index,Term)
CL_CLASS_DEF(index,IndexReader)
//#include "Filter.h"
CL_CLASS_DEF(document,Document)
CL_CLASS_DEF(util,Comparable)
//#include "Sort.h"
//#include "CLucene/util/VoidList.h"
//#include "Explanation.h"
//#include "Similarity.h"

CL_NS_DEF(search)

	class Query;
	class Scorer;
	class Explanation;
	class Hits;
	class Sort;
	class FieldDoc;
	class TopFieldDocs;
   
   /** Expert: Returned by low-level search implementations.
	* @see TopDocs */
	struct CLUCENE_EXPORT ScoreDoc {
		/** Expert: A hit document's number.
		* @see Searcher#doc(int32_t)
		*/
		int32_t doc;

		/** Expert: The score of this document for the query. */
		float_t score;
	};

	/** Expert: Returned by low-level search implementations.
	* @see Searcher#search(Query,Filter,int32_t) */
	class CLUCENE_EXPORT TopDocs:LUCENE_BASE {
	public:
		/** Expert: The total number of hits for the query.
		 * @see Hits#length()
		*/
		int32_t totalHits;

		/** Expert: The top hits for the query. */
		ScoreDoc* scoreDocs;
		int32_t scoreDocsLength;

		/** Expert: Constructs a TopDocs. TopDocs takes ownership of the ScoreDoc array*/
		TopDocs(const int32_t th, ScoreDoc* sds, int32_t scoreDocsLength);
		virtual ~TopDocs();

	private:
		/** Expert: Stores the maximum score value encountered, needed for normalizing. */
		//float_t maxScore;
	};

    /** Lower-level search API.
    * <br>HitCollectors are primarily meant to be used to implement queries,
    * sorting and filtering.
    * @see Searcher#search(Query,HitCollector)
    */
	class CLUCENE_EXPORT HitCollector: LUCENE_BASE {
    public:
      /** Called once for every non-zero scoring document, with the document number
      * and its score.
      *
      * <P>If, for example, an application wished to collect all of the hits for a
      * query in a BitSet, then it might:<pre>
      *   Searcher searcher = new IndexSearcher(indexReader);
      *   final BitSet bits = new BitSet(indexReader.maxDoc());
      *   searcher.search(query, new HitCollector() {
      *       public void collect(int32_t doc, float score) {
      *         bits.set(doc);
      *       }
      *     });
      * </pre>
      *
      * <p>Note: This is called in an inner search loop.  For good search
      * performance, implementations of this method should not call
      * {@link Searcher#doc(int32_t)} or
      * {@link IndexReader#document(int32_t)} on every
      * document number encountered.  Doing so can slow searches by an order
      * of magnitude or more.
      * <p>Note: The <code>score</code> passed to this method is a raw score.
      * In other words, the score will not necessarily be a float whose value is
      * between 0 and 1.
      */
      virtual void collect(const int32_t doc, const float_t score) = 0;
      virtual ~HitCollector(){}
    };

   /** Expert: Calculate query weights and build query scorers.
   *
   * <p>A Weight is constructed by a query, given a Searcher ({@link
   * Query#_createWeight(Searcher)}).  The {@link #sumOfSquaredWeights()} method
   * is then called on the top-level query to compute the query normalization
   * factor (@link Similarity#queryNorm(float_t)}).  This factor is then passed to
   * {@link #normalize(float_t)}.  At this point the weighting is complete and a
   * scorer may be constructed by calling {@link #scorer(IndexReader)}.
   */
	class CLUCENE_EXPORT Weight
    {
    public:
		virtual ~Weight();

      /** The query that this concerns. */
      virtual Query* getQuery() = 0;

      /** The weight for this query. */
      virtual float_t getValue() = 0;

      /** The sum of squared weights of contained query clauses. */
      virtual float_t sumOfSquaredWeights() = 0;

      /** Assigns the query normalization factor to this. */
      virtual void normalize(float_t norm) = 0;

      /** Constructs a scorer for this. */
      virtual Scorer* scorer(CL_NS(index)::IndexReader* reader) = 0;

      /** An explanation of the score computation for the named document. */
      virtual Explanation* explain(CL_NS(index)::IndexReader* reader, int32_t doc) = 0;

      virtual TCHAR* toString();
   };

   class CLUCENE_EXPORT HitDoc
   {
    public:
		float_t score;
		int32_t id;
		CL_NS(document)::Document* doc;
		
		HitDoc* next;					  // in doubly-linked cache
		HitDoc* prev;					  // in doubly-linked cache
		
		HitDoc(const float_t s, const int32_t i);
		virtual ~HitDoc();
    };


CL_NS_END
#endif
