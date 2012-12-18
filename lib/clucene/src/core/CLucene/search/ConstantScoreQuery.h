/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_ConstantScoreQuery_
#define _lucene_search_ConstantScoreQuery_

CL_CLASS_DEF(index,IndexReader)

#include "Query.h"
#include "Filter.h"
#include "Explanation.h"

CL_NS_DEF(search)

/**
 * A query that wraps a filter and simply returns a constant score equal to the
 * query boost for every document in the filter.
 *
 */
class CLUCENE_EXPORT ConstantScoreQuery : public Query {
protected:
    Filter* filter;

public:
    /**
    * Constructs a new ConstantScoreQuery, and takes ownership of the filter object
    *
    * @memory this object consumes _filter
    */
    ConstantScoreQuery(Filter* _filter);
    virtual ~ConstantScoreQuery();

    /** Returns the encapsulated filter */
    Filter* getFilter() const;

    Query* rewrite(CL_NS(index)::IndexReader* reader);

    /** Constant score query does not return any terms */
    void extractTerms( TermSet * termset ) const;

protected:
    Weight* _createWeight(Searcher* searcher);

public:
    /** Prints a user-readable version of this query. */
    TCHAR* toString(const TCHAR* field) const;

    /** Returns true if <code>o</code> is equal to this. */
    bool equals(Query* o) const;

    /** Returns a hash code value for this object. */
    size_t hashCode() const;

    const char* getObjectName() const;
	static const char* getClassName(){ return "ConstantScoreQuery"; }
    Query* clone() const;

    friend class ConstantWeight;

protected:
    ConstantScoreQuery( const ConstantScoreQuery& copy );
};


/**
 * A range query that returns a constant score equal to its boost for
 * all documents in the range.
 * <p>
 * It does not have an upper bound on the number of clauses covered in the range.
 * <p>
 * If an endpoint is null, it is said to be "open".
 * Either or both endpoints may be open.  Open endpoints may not be exclusive
 * (you can't select all but the first or last term without explicitly specifying the term to exclude.)
 *
 */
class CLUCENE_EXPORT ConstantScoreRangeQuery : public Query
{
private:
    TCHAR* fieldName;
    TCHAR* lowerVal;
    TCHAR* upperVal;
    bool includeLower;
    bool includeUpper;

public:
    ConstantScoreRangeQuery(const TCHAR* _fieldName, const TCHAR* _lowerVal, const TCHAR* _upperVal,
        bool _includeLower, bool _includeUpper);
    virtual ~ConstantScoreRangeQuery();

    /** Returns the field name for this query */
    TCHAR* getField() const { return fieldName; }
    /** Returns the value of the lower endpoint of this range query, null if open ended */
    TCHAR* getLowerVal() const { return lowerVal; }
    /** Returns the value of the upper endpoint of this range query, null if open ended */
    TCHAR* getUpperVal() const { return upperVal; }
    /** Returns <code>true</code> if the lower endpoint is inclusive */
    bool includesLower() const { return includeLower; }
    /** Returns <code>true</code> if the upper endpoint is inclusive */
    bool includesUpper() const { return includeUpper; }

    Query* rewrite(CL_NS(index)::IndexReader* reader);

    /** Prints a user-readable version of this query. */
    TCHAR* toString(const TCHAR* field) const;

    /** Returns true if <code>o</code> is equal to this. */
    bool equals(Query* o) const;

    /** Returns a hash code value for this object.*/
    size_t hashCode() const;

    const char* getObjectName() const;
	static const char* getClassName(){ return "ConstantScoreRangeQuery"; }
    Query* clone() const;
protected:
    ConstantScoreRangeQuery( const ConstantScoreRangeQuery& copy );
};

CL_NS_END

#endif

