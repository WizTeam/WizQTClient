/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_legacy_MultiFieldQueryParser
#define _lucene_queryParser_legacy_MultiFieldQueryParser

#include "QueryParser.h"
#include "CLucene/util/VoidMap.h"

CL_NS_DEF2(queryParser,legacy)

  typedef CL_NS(util)::CLHashMap<const TCHAR*,
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
  	LUCENE_STATIC_CONSTANT(uint8_t, NORMAL_FIELD=0);
	  LUCENE_STATIC_CONSTANT(uint8_t, REQUIRED_FIELD=1);
	  LUCENE_STATIC_CONSTANT(uint8_t, PROHIBITED_FIELD=2);

    /**
    * Creates a MultiFieldQueryParser.
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
    * <p>In other words, all the query's terms must appear, but it doesn't matter in
    * what fields they appear.</p>
    */
		MultiFieldQueryParser(const TCHAR** fields, CL_NS(analysis)::Analyzer* a, BoostMap* boosts = NULL);
		virtual ~MultiFieldQueryParser();

    /**
     * <p>
     * Parses a query which searches on the fields specified.
     * <p>
     * If x fields are specified, this effectively constructs:
     * <pre>
     * <code>
     * (field1:query) (field2:query) (field3:query)...(fieldx:query)
     * </code>
     * </pre>
     *
     * @param query Query string to parse
     * @param fields Fields to search on
     * @param analyzer Analyzer to use
     * @throws ParserException if query parsing fails
     * @throws TokenMgrError if query parsing fails
     */
		static CL_NS(search)::Query* parse(const TCHAR* query, const TCHAR** fields, CL_NS(analysis)::Analyzer* analyzer);

    /**
     * <p>
     * Parses a query, searching on the fields specified.
     * Use this if you need to specify certain fields as required,
     * and others as prohibited.
     * <p><pre>
     * Usage:
     * <code>
     * TCHAR** fields = {"filename", "contents", "description"};
     * int8_t* flags = {MultiFieldQueryParser::NORMAL FIELD,
     *                MultiFieldQueryParser::REQUIRED FIELD,
     *                MultiFieldQueryParser::PROHIBITED FIELD};
     * parse(query, fields, flags, analyzer);
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
     * @throws ParserException if query parsing fails
     * @throws TokenMgrError if query parsing fails
     */
		static CL_NS(search)::Query* parse(const TCHAR* query, const TCHAR** fields, const uint8_t* flags, CL_NS(analysis)::Analyzer* analyzer);

		// non-static version of the above
		CL_NS(search)::Query* parse(const TCHAR* query);

	protected:
		CL_NS(search)::Query* GetFieldQuery(const TCHAR* field, TCHAR* queryText);
		CL_NS(search)::Query* GetFieldQuery(const TCHAR* field, TCHAR* queryText, int32_t slop);
        CL_NS(search)::Query* GetFuzzyQuery(const TCHAR* field, TCHAR* termStr);
		CL_NS(search)::Query* GetRangeQuery(const TCHAR* field, TCHAR* part1, TCHAR* part2, bool inclusive);
	    CL_NS(search)::Query* GetPrefixQuery(const TCHAR* field, TCHAR* termStr);
	    CL_NS(search)::Query* GetWildcardQuery(const TCHAR* field, TCHAR* termStr);

		/**
		* A special virtual function for the MultiFieldQueryParser which can be used
		* to clean up queries. Once the field name is known and the query has been
		* created, its passed to this function.
		* An example of this usage is to set boosts.
		*/
		virtual CL_NS(search)::Query* QueryAddedCallback(const TCHAR* field, CL_NS(search)::Query* query){ return query; }
  };
CL_NS_END2
#endif
