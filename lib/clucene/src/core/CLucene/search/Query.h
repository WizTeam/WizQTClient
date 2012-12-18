/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_Query_h
#define _lucene_search_Query_h


#include "CLucene/util/Array.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/Equators.h"

CL_CLASS_DEF(index,IndexReader)


CL_NS_DEF(search)
    class Weight;
    class Similarity;
    class Searcher;

    typedef std::set<CL_NS(index)::Term *, CL_NS(index)::Term_UnorderedCompare>  TermSet;

	/** The abstract base class for queries.
    <p>Instantiable subclasses are:
    <ul>
    <li> {@link TermQuery}
    <li> {@link MultiTermQuery}
    <li> {@link BooleanQuery}
    <li> {@link WildcardQuery}
    <li> {@link PhraseQuery}
    <li> {@link PrefixQuery}
    <li> {@link PhrasePrefixQuery}
    <li> {@link FuzzyQuery}
    <li> {@link RangeQuery}
    <li> {@link spans.SpanQuery}
    </ul>
    <p>A parser for queries is contained in:
    <ul>
    <li>{@link queryParser.QueryParser QueryParser}
    </ul>
	*/
  class CLUCENE_EXPORT Query : public CL_NS(util)::NamedObject {
	private:
		// query boost factor
		float_t boost;
	protected:
        Query();
        Query(const Query& clone);
	public:
		virtual ~Query();

		/** Sets the boost for this query clause to <code>b</code>.  Documents
		* matching this clause will (in addition to the normal weightings) have
		* their score multiplied by <code>b</code>.
		*/
		void setBoost(float_t b);

		/** Gets the boost for this clause.  Documents matching
		* this clause will (in addition to the normal weightings) have their score
		* multiplied by <code>b</code>.   The boost is 1.0 by default.
		*/
		float_t getBoost() const;

        /** Expert: Constructs an initializes a Weight for a top-level query. */
        Weight* weight(Searcher* searcher);

        /** Expert: called to re-write queries into primitive queries.
        *
        * @memory:
        * The caller has to clean up. When rewrite() returns a pointer which
        * differs from the pointer to the initial query, the pointer points
        * to a newly allocated object and has also to be cleaned up. */
        virtual Query* rewrite(CL_NS(index)::IndexReader* reader);

        /** Expert: called when re-writing queries under MultiSearcher.
        *
        * Create a single query suitable for use by all subsearchers (in 1-1
        * correspondence with queries). This is an optimization of the OR of
        * all queries. We handle the common optimization cases of equal
        * queries and overlapping clauses of boolean OR queries (as generated
        * by MultiTermQuery.rewrite() and RangeQuery.rewrite()).
        * Be careful overriding this method as queries[0] determines which
        * method will be called and is not necessarily of the same type as
        * the other queries.
        */
        virtual Query* combine(CL_NS(util)::ArrayBase<Query*>* queries);

        /** Expert: adds all terms occurring in this query to the terms set. Only
         * works if this query is in its {@link #rewrite rewritten} form.
         *
         * @memory:
         * CLucene specific - all terms in the list have their reference counter
         * increased by one.
         *
         * @throws CLuceneError with CL_ERR_UnsupportedOperation
         */
        virtual void extractTerms( TermSet * termset ) const;

        /** Expert: merges the clauses of a set of BooleanQuery's into a single
        * BooleanQuery.
        *
        *<p>A utility for use by {@link #combine(Query[])} implementations.
        */
        static Query* mergeBooleanQueries(CL_NS(util)::ArrayBase<Query*>* queries);

        /** Expert: Returns the Similarity implementation to be used for this query.
        * Subclasses may override this method to specify their own Similarity
        * implementation, perhaps one that delegates through that of the Searcher.
        * By default the Searcher's Similarity implementation is returned.*/
        Similarity* getSimilarity(Searcher* searcher);

        /** Returns a clone of this query. */
        virtual Query* clone() const = 0;
        _CL_DEPRECATED(getObjectName) const char* getQueryName() const;

        /** Prints a query to a string, with <code>field</code> assumed to be the
        * default field and omitted.
        * <p>The representation used is one that is supposed to be readable
        * by {@link org.apache.lucene.queryParser.QueryParser QueryParser}. However,
        * there are the following limitations:
        * <ul>
        *  <li>If the query was created by the parser, the printed
        *  representation may not be exactly what was parsed. For example,
        *  characters that need to be escaped will be represented without
        *  the required backslash.</li>
        * <li>Some of the more complicated queries (e.g. span queries)
        *  don't have a representation that can be parsed by QueryParser.</li>
        * </ul>
        *
        * @memory always returns a newly allocated string, which the caller is
        * responsible for deleting
        *
        */
        virtual TCHAR* toString(const TCHAR* field) const = 0;

        virtual bool equals(Query* other) const = 0;
        virtual size_t hashCode() const = 0;

        /** Prints a query to a string. */
        TCHAR* toString() const;


        /** Expert: Constructs an appropriate Weight implementation for this query.
        *
        * <p>Only implemented by primitive queries, which re-write to themselves.
        * <i>This is an Internal function</i>
        */
        virtual Weight* _createWeight(Searcher* searcher);
  };

CL_NS_END
#endif
