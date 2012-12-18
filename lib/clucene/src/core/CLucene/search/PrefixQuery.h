/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_PrefixQuery
#define _lucene_search_PrefixQuery

CL_CLASS_DEF(index,Term)
//#include "CLucene/index/Terms.h"
//#include "CLucene/index/IndexReader.h"
//#include "SearchHeader.h"
//#include "BooleanQuery.h"
//#include "TermQuery.h"
#include "Query.h"
#include "Filter.h"
CL_CLASS_DEF(util,StringBuffer)

CL_NS_DEF(search) 
/** A Query that matches documents containing terms with a specified prefix. A PrefixQuery
* is built by QueryParser for input like <code>app*</code>. */
	class CLUCENE_EXPORT PrefixQuery: public Query {
	private:
		CL_NS(index)::Term* prefix;
	protected:
		PrefixQuery(const PrefixQuery& clone);
	public:

		//Constructor. Constructs a query for terms starting with prefix
		PrefixQuery(CL_NS(index)::Term* Prefix);

		//Destructor
		~PrefixQuery();

		//Returns the name "PrefixQuery"
		const char* getObjectName() const;
		static const char* getClassName();

		/** Returns the prefix of this query. */
		CL_NS(index)::Term* getPrefix(bool pointer=true);

    Query* combine(CL_NS(util)::ArrayBase<Query*>* queries);
		Query* rewrite(CL_NS(index)::IndexReader* reader);
		Query* clone() const;
		bool equals(Query * other) const;

		//Creates a user-readable version of this query and returns it as as string
		TCHAR* toString(const TCHAR* field) const;

		size_t hashCode() const;
	};
	
	
    class CLUCENE_EXPORT PrefixFilter: public Filter 
    {
    private:
    	CL_NS(index)::Term* prefix;
    protected:
    	PrefixFilter( const PrefixFilter& copy );
    public:
      class PrefixGenerator;

    	PrefixFilter(CL_NS(index)::Term* prefix);
    	~PrefixFilter();
    
    	/** Returns a BitSet with true for documents which should be permitted in
    	search results, and false for those that should not. */
    	CL_NS(util)::BitSet* bits( CL_NS(index)::IndexReader* reader );
    	
    	Filter* clone() const;

		/** Prints a user-readable version of this query. */
    	TCHAR* toString();

		// Returns a reference of internal prefix
		CL_NS(index)::Term* getPrefix() const;
    };
CL_NS_END
#endif
