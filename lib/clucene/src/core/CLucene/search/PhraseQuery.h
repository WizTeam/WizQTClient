/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_PhraseQuery_
#define _lucene_search_PhraseQuery_

#include "Query.h"
CL_CLASS_DEF(index,Term)
CL_CLASS_DEF(search,Scorer)
#include "CLucene/util/Array.h"
#include "CLucene/util/VoidList.h"

CL_NS_DEF(search)
	/** A Query that matches documents containing a particular sequence of terms.
	* A PhraseQuery is built by QueryParser for input like <code>"new york"</code>.
	*
	* <p>This query may be combined with other terms or queries with a {@link BooleanQuery}.
	*/
	class CLUCENE_EXPORT PhraseQuery: public Query {
	private:
		const TCHAR* field;
		CL_NS(util)::CLVector<CL_NS(index)::Term*>* terms;
		CL_NS(util)::CLVector<int32_t,CL_NS(util)::Deletor::DummyInt32>* positions;
		int32_t slop;

    	friend class PhraseWeight;
	protected:
		Weight* _createWeight(Searcher* searcher);
		PhraseQuery(const PhraseQuery& clone);
	public:
		/** Constructs an empty phrase query. */
        PhraseQuery();
		virtual ~PhraseQuery();

		/** Sets the number of other words permitted between words in query phrase.
		If zero, then this is an exact phrase search.  For larger values this works
		like a <code>WITHIN</code> or <code>NEAR</code> operator.

		<p>The slop is in fact an edit-distance, where the units correspond to
		moves of terms in the query phrase out of position.  For example, to switch
		the order of two words requires two moves (the first move places the words
		atop one another), so to permit re-orderings of phrases, the slop must be
		at least two.

		<p>More exact matches are scored higher than sloppier matches, thus search
		results are sorted by exactness.

		<p>The slop is zero by default, requiring exact matches.*/
        void setSlop(const int32_t s);

		/** Returns the slop.  See setSlop(). */
        int32_t getSlop() const;

		/**
		* Adds a term to the end of the query phrase.
		* The relative position of the term is the one immediately after the last term added.
		*/
        void add(CL_NS(index)::Term* term);

		/**
		* Adds a term to the end of the query phrase.
		* The relative position of the term within the phrase is specified explicitly.
		* This allows e.g. phrases with more than one term at the same position
		* or phrases with gaps (e.g. in connection with stopwords).
		*
		* @param term
		* @param position
		*/
		void add(CL_NS(index)::Term* term, int32_t position);

		/** Returns the set of terms in this phrase. */
        CL_NS(index)::Term** getTerms() const;

		/**
		* Returns the relative positions of terms in this phrase.
		*/
		void getPositions(CL_NS(util)::ValueArray<int32_t>& result) const;

		//Returns the sum of squared weights
    float_t sumOfSquaredWeights(Searcher* searcher);

		//Normalizes the Weight
    void normalize(const float_t norm);

    Scorer* scorer(CL_NS(index)::IndexReader* reader);

    const TCHAR* getFieldName() const;

		/** Prints a user-readable version of this query. */
    TCHAR* toString(const TCHAR* f) const;

		Query* clone() const;
		bool equals(Query *) const;

		size_t hashCode() const;

		const char* getObjectName() const;
		static const char* getClassName();

        /** Expert: adds all terms occurring in this query to the terms set. */
        void extractTerms( TermSet * termset ) const;
	};
CL_NS_END
#endif
