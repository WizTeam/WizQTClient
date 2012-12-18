/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_TermQuery_
#define _lucene_search_TermQuery_

CL_CLASS_DEF(index,Term)
CL_CLASS_DEF(util,StringBuffer)

#include "Query.h"

CL_NS_DEF(search)
    /** A Query that matches documents containing a term.
	This may be combined with other terms with a {@link BooleanQuery}.
	*/
    class CLUCENE_EXPORT TermQuery: public Query {
    private:
		CL_NS(index)::Term* term;
    protected:
        Weight* _createWeight(Searcher* searcher);
        TermQuery(const TermQuery& clone);
	public:
		// Constructs a query for the term <code>t</code>.
		TermQuery(CL_NS(index)::Term* t);
		virtual ~TermQuery();

		static const char* getClassName();
		const char* getObjectName() const;

		/** Returns the term of this query. */
		CL_NS(index)::Term* getTerm(bool pointer=true) const;

		/** Prints a user-readable version of this query. */
		TCHAR* toString(const TCHAR* field) const;

		/** Returns true if <code>o</code> is equal to this. */
		bool equals(Query* other) const;
		Query* clone() const;

		/** Returns a hash code value for this object.*/
		size_t hashCode() const;

        /** Expert: adds all terms occurring in this query to the termset set. */
        void extractTerms( TermSet * termset ) const;

    };
CL_NS_END
#endif

