/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_BooleanClause_
#define _lucene_search_BooleanClause_

CL_CLASS_DEF(util,StringBuffer)
CL_CLASS_DEF(search,Query)

CL_NS_DEF(search)

// A clause in a BooleanQuery. 
class CLUCENE_EXPORT BooleanClause:LUCENE_BASE {
public:
	/** Specifies how clauses are to occur in matching documents. */
	enum Occur {
		/** Use this operator for clauses that <i>must</i> appear in the matching documents. */
		MUST=1,

		/** Use this operator for clauses that <i>should</i> appear in the 
		* matching documents. For a BooleanQuery with no <code>MUST</code> 
		* clauses one or more <code>SHOULD</code> clauses must match a document 
		* for the BooleanQuery to match.
		* @see BooleanQuery#setMinimumNumberShouldMatch
		*/
		SHOULD=2,

		/** Use this operator for clauses that <i>must not</i> appear in the matching documents.
		* Note that it is not possible to search for queries that only consist
		* of a <code>MUST_NOT</code> clause. */
		MUST_NOT=4
	};
private:	
	/** The query whose matching documents are combined by the boolean query.
	*     @deprecated use {@link #setQuery(Query)} instead */
	Query* query;
	
	Occur occur;

	/* Middle layer for the Occur enum; will be removed soon enough. */
	void setFields(Occur occur);
public:
	bool deleteQuery;

	
	int32_t getClauseCount();


	/** Constructs a BooleanClause with query <code>q</code>, required
	* <code>r</code> and prohibited <code>p</code>.
	* @deprecated use BooleanClause(Query, Occur) instead
	* <ul>
	*  <li>For BooleanClause(query, true, false) use BooleanClause(query, BooleanClause.Occur.MUST)
	*  <li>For BooleanClause(query, false, false) use BooleanClause(query, BooleanClause.Occur.SHOULD)
	*  <li>For BooleanClause(query, false, true) use BooleanClause(query, BooleanClause.Occur.MUST_NOT)
	* </ul>
	*/ 
	BooleanClause(Query* q, const bool DeleteQuery,const bool req, const bool p);

	BooleanClause(const BooleanClause& clone);

	/** Constructs a BooleanClause.
	*/ 
	BooleanClause(Query* q, const bool DeleteQuery, Occur o);


	BooleanClause* clone() const;

	~BooleanClause();


	/** Returns true if <code>o</code> is equal to this. */
	bool equals(const BooleanClause* other) const;

	/** Returns a hash code value for this object.*/
	size_t hashCode() const;

	Occur getOccur() const;
	void setOccur(Occur o);

	Query* getQuery() const;
	void setQuery(Query* q);

	bool isProhibited() const;
	bool isRequired() const;

	TCHAR* toString() const;

public: // TODO: Make private and remove for CLucene 2.3.2
	/** If true, documents documents which <i>do not</i>
	match this sub-query will <i>not</i> match the boolean query.
	@deprecated use {@link #setOccur(BooleanClause.Occur)} instead */
	bool required;

	/** If true, documents documents which <i>do</i>
	match this sub-query will <i>not</i> match the boolean query.
	@deprecated use {@link #setOccur(BooleanClause.Occur)} instead */
	bool prohibited;
};


CL_NS_END
#endif

