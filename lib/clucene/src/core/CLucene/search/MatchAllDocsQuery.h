/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_MatchAllDocsQuery_h
#define _lucene_search_MatchAllDocsQuery_h

#include "Scorer.h"
#include "SearchHeader.h"
#include "Query.h"

CL_CLASS_DEF(search,Explanation)
CL_CLASS_DEF(search,Similarity)
CL_CLASS_DEF(index,IndexReader)

CL_NS_DEF(search)
  class Weight;

	/**
	* A query that matches all documents.
	*
	*/
	class CLUCENE_EXPORT MatchAllDocsQuery : public Query {
	protected:
		MatchAllDocsQuery(const MatchAllDocsQuery& clone);
	public:
		MatchAllDocsQuery();
		virtual ~MatchAllDocsQuery();

		class MatchAllScorer;
		class MatchAllDocsWeight;

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
		*/
        virtual TCHAR* toString(const TCHAR* field = NULL) const;

	protected:
		/** Expert: Constructs an appropriate Weight implementation for this query.
		*
		* <p>Only implemented by primitive queries, which re-write to themselves.
		* <i>This is an Internal function</i>
		*/
		virtual Weight* _createWeight(Searcher* searcher);

	public:
        /** Returns a clone of this query. */
        virtual Query* clone() const;

        /** Expert: MatchAllDocsQuery provides no terms at all. */
        void extractTerms( TermSet * termset ) const;

        virtual bool equals(Query* o) const;
        virtual size_t hashCode() const;

		static const char* getClassName();
		const char* getObjectName() const;
	};

CL_NS_END
#endif
