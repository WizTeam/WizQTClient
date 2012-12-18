/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_BooleanQuery_
#define _lucene_search_BooleanQuery_

CL_CLASS_DEF(util,StringBuffer)
CL_CLASS_DEF(search,Weight)
#include "Query.h"
#include "BooleanClause.h"
#include "CLucene/util/VoidList.h"

CL_NS_DEF(search)


    // A Query that matches documents matching boolean combinations of other
    // queries, typically {@link TermQuery}s or {@link PhraseQuery}s.
	class CLUCENE_EXPORT BooleanQuery:public Query {
	public:
		typedef CL_NS(util)::CLVector<BooleanClause*,CL_NS(util)::Deletor::Object<BooleanClause> > ClausesType;
	private:
		ClausesType* clauses;
		static size_t maxClauseCount;

    /** Whether hit docs may be collected out of docid order. */
    static bool allowDocsOutOfOrder;

		bool disableCoord;
    protected:
		int32_t minNrShouldMatch;
		Weight* _createWeight(Searcher* searcher);
		BooleanQuery(const BooleanQuery& clone);

	public:
		/** Constructs an empty boolean query. */
		BooleanQuery( bool disableCoord = false );

		~BooleanQuery();
		const char* getObjectName() const;
		static const char* getClassName();

     /** Return the maximum number of clauses permitted, 1024 by default.
      * Attempts to add more than the permitted number of clauses cause {@link
      * TooManyClauses} to be thrown.*/
     static size_t getMaxClauseCount();

     /** Set the maximum number of clauses permitted. */
     static void setMaxClauseCount(const size_t maxClauseCount);

		 /** Adds a clause to a boolean query.  Clauses may be:
		 * <ul>
		 * <li><code>required</code> which means that documents which <i>do not</i>
		 * match this sub-query will <i>not</i> match the boolean query;
		 * <li><code>prohibited</code> which means that documents which <i>do</i>
		 * match this sub-query will <i>not</i> match the boolean query; or
		 * <li>neither, in which case matched documents are neither prohibited from
		 * nor required to match the sub-query. However, a document must match at
		 * least 1 sub-query to match the boolean query.
		 * </ul>
		 * It is an error to specify a clause as both <code>required</code> and
		 * <code>prohibited</code>.
		 *
		 * @deprecated use {@link #add(Query, BooleanClause.Occur)} instead:
		 * <ul>
		 *  <li>For add(query, true, false) use add(query, BooleanClause.Occur.MUST)
		 *  <li>For add(query, false, false) use add(query, BooleanClause.Occur.SHOULD)
		 *  <li>For add(query, false, true) use add(query, BooleanClause.Occur.MUST_NOT)
		 * </ul>
		 */
		void add(Query* query, const bool required, const bool prohibited){
			add(query,false,required,prohibited);
		}
		void add(Query* query, const bool deleteQuery, const bool required, const bool prohibited);

		void add(Query* query, const bool deleteQuery, BooleanClause::Occur occur);
		void add(Query* query, BooleanClause::Occur occur) { add(query,false,occur); };

		/** Copies the clauses of this query into the array.
		* The array must be at least as long as getClauseCount()
		* If you want to use the clauses, make sure you null terminate it.
		*/
		void getClauses(BooleanClause** clauses) const;

		///@deprecated
		_CL_DEPRECATED( getClauses(clauses) ) BooleanClause** getClauses() const;

    /**
    * Give client code access to clauses.size() so we know how
    * large the array returned by getClauses is.
    */
    size_t getClauseCount() const;

	  /** Adds a clause to a boolean query.
	  * @see #getMaxClauseCount()
	  */
	  void add(BooleanClause* clause);

	  Query* rewrite(CL_NS(index)::IndexReader* reader);
	  Query* clone() const;

    /** Expert: adds all terms occurring in this query to the termset set. */
    void extractTerms( TermSet * termset ) const;

	  bool equals(Query* o) const;
	  Similarity* getSimilarity( Searcher* searcher );

	  bool isCoordDisabled();
	  void setCoordDisabled( bool disableCoord );

	  static bool getUseScorer14();
	  static void setUseScorer14( bool use14 );

    /**
     * Expert: Indicates whether hit docs may be collected out of docid
     * order.
     *
     * <p>
     * Background: although the contract of the Scorer class requires that
     * documents be iterated in order of doc id, this was not true in early
     * versions of Lucene.  Many pieces of functionality in the current
     * Lucene code base have undefined behavior if this contract is not
     * upheld, but in some specific simple cases may be faster.  (For
     * example: disjunction queries with less than 32 prohibited clauses;
     * This setting has no effect for other queries.)
     * </p>
     *
     * <p>
     * Specifics: By setting this option to true, calls to
     * {@link HitCollector#collect(int,float)} might be
     * invoked first for docid N and only later for docid N-1.
     * Being static, this setting is system wide.
     * </p>
     */
    static void setAllowDocsOutOfOrder(bool allow);

    /**
     * Whether hit docs may be collected out of docid order.
     * @see #setAllowDocsOutOfOrder(boolean)
     */
    static bool getAllowDocsOutOfOrder();


  	/** Prints a user-readable version of this query. */
	  TCHAR* toString(const TCHAR* field) const;
	  /** Returns a hash code value for this object.*/
	  size_t hashCode() const;

	  //internal
	  int32_t getMinNrShouldMatch();
    friend class BooleanWeight;
  };

CL_NS_END
#endif
