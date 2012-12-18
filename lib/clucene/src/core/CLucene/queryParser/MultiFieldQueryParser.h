/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_MultiFieldQueryParser_
#define _lucene_queryParser_MultiFieldQueryParser_

#include "QueryParser.h"
#include "CLucene/util/VoidMap.h"


CL_NS_DEF(queryParser)

typedef CL_NS(util)::CLHashMap<TCHAR*,
                               float_t,
                               CL_NS(util)::Compare::TChar,
                               CL_NS(util)::Equals::TChar,
                               CL_NS(util)::Deletor::Dummy,
                               CL_NS(util)::Deletor::DummyFloat
                              > BoostMap;

/**
* A QueryParser which constructs queries to search multiple fields.
*
*/
class CLUCENE_EXPORT MultiFieldQueryParser: public QueryParser
{
protected:
  const TCHAR** fields;
  BoostMap* boosts;
public:
  /**
  * Creates a MultiFieldQueryParser.
  * Allows passing of a map with term to Boost, and the boost to apply to each term.
  *
  * <p>It will, when parse(String query)
  * is called, construct a query like this (assuming the query consists of
  * two terms and you specify the two fields <code>title</code> and <code>body</code>):</p>
  *
  * <code>
  * (title:term1 body:term1) (title:term2 body:term2)
  * </code>
  *
  * <p>When setDefaultOperator(AND_OPERATOR) is set, the result will be:</p>
  *
  * <code>
  * +(title:term1 body:term1) +(title:term2 body:term2)
  * </code>
  *
  * <p>When you pass a boost (title=>5 body=>10) you can get </p>
  *
  * <code>
  * +(title:term1^5.0 body:term1^10.0) +(title:term2^5.0 body:term2^10.0)
  * </code>
  *
  * <p>In other words, all the query's terms must appear, but it doesn't matter in
  * what fields they appear.</p>
  */
  MultiFieldQueryParser(const TCHAR** _fields, CL_NS(analysis)::Analyzer* a, BoostMap* _boosts = NULL);
  virtual ~MultiFieldQueryParser();


protected:
  CL_NS(search)::Query* getFieldQuery(const TCHAR* field, TCHAR* queryText, const int32_t slop);
  CL_NS(search)::Query* getFieldQuery(const TCHAR* field, TCHAR* queryText) { return getFieldQuery(field,queryText,0); }
  CL_NS(search)::Query* getFuzzyQuery(const TCHAR* field, TCHAR* termStr, const float_t minSimilarity);
  CL_NS(search)::Query* getPrefixQuery(const TCHAR* field, TCHAR* termStr);
  CL_NS(search)::Query* getWildcardQuery(const TCHAR* field, TCHAR* termStr);
  CL_NS(search)::Query* getRangeQuery(const TCHAR* field, TCHAR* part1, TCHAR* part2, const bool inclusive);

public:
  /**
  * Parses a query which searches on the fields specified.
  * <p>
  * If x fields are specified, this effectively constructs:
  * <pre>
  * <code>
  * (field1:query1) (field2:query2) (field3:query3)...(fieldx:queryx)
  * </code>
  * </pre>
  * @param queries Queries strings to parse
  * @param fields Fields to search on
  * @param analyzer Analyzer to use
  * @throws ParseException if query parsing fails
  * @throws IllegalArgumentException if the length of the queries array differs
  *  from the length of the fields array
  */
  static CL_NS(search)::Query* parse(const TCHAR** _queries, const TCHAR** _fields,
    CL_NS(analysis)::Analyzer* analyzer);

  /**
  * Parses a query, searching on the fields specified.
  * Use this if you need to specify certain fields as required,
  * and others as prohibited.
  * <p><pre>
  * Usage:
  * <code>
  * String[] fields = {"filename", "contents", "description"};
  * BooleanClause.Occur[] flags = {BooleanClause.Occur.SHOULD,
  *                BooleanClause.Occur.MUST,
  *                BooleanClause.Occur.MUST_NOT};
  * MultiFieldQueryParser.parse("query", fields, flags, analyzer);
  * </code>
  * </pre>
  *<p>
  * The code above would construct a query:
  * <pre>
  * <code>
  * (filename:query) +(contents:query) -(description:query)
  * </code>
  * </pre>
  *
  * @param query Query string to parse
  * @param fields Fields to search on
  * @param flags Flags describing the fields
  * @param analyzer Analyzer to use
  * @throws ParseException if query parsing fails
  * @throws IllegalArgumentException if the length of the fields array differs
  *  from the length of the flags array
  */
  static CL_NS(search)::Query* parse(const TCHAR* query, const TCHAR** _fields,
    const uint8_t* flags, CL_NS(analysis)::Analyzer* analyzer);

  /**
  * Parses a query, searching on the fields specified.
  * Use this if you need to specify certain fields as required,
  * and others as prohibited.
  * <p><pre>
  * Usage:
  * <code>
  * String[] query = {"query1", "query2", "query3"};
  * String[] fields = {"filename", "contents", "description"};
  * BooleanClause.Occur[] flags = {BooleanClause.Occur.SHOULD,
  *                BooleanClause.Occur.MUST,
  *                BooleanClause.Occur.MUST_NOT};
  * MultiFieldQueryParser.parse(query, fields, flags, analyzer);
  * </code>
  * </pre>
  *<p>
  * The code above would construct a query:
  * <pre>
  * <code>
  * (filename:query1) +(contents:query2) -(description:query3)
  * </code>
  * </pre>
  *
  * @param queries Queries string to parse
  * @param fields Fields to search on
  * @param flags Flags describing the fields
  * @param analyzer Analyzer to use
  * @throws ParseException if query parsing fails
  * @throws IllegalArgumentException if the length of the queries, fields,
  *  and flags array differ
  */
  static CL_NS(search)::Query* parse(const TCHAR** _queries, const TCHAR** _fields, const uint8_t* flags,
    CL_NS(analysis)::Analyzer* analyzer);

  CL_NS(search)::Query* parse(const TCHAR* _query){return QueryParser::parse(_query);}
};
CL_NS_END
#endif
