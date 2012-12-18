/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_WildcardQuery_
#define _lucene_search_WildcardQuery_

//#include "CLucene/index/IndexReader.h"
CL_CLASS_DEF(index,Term)
#include "MultiTermQuery.h"
#include "Filter.h"
//#include "WildcardTermEnum.h"

CL_NS_DEF(search)

/** Implements the wildcard search query. Supported wildcards are <code>*</code>, which
  * matches any character sequence (including the empty one), and <code>?</code>,
  * which matches any single character. Note this query can be slow, as it
  * needs to iterate over all terms. In order to prevent extremely slow WildcardQueries,
  * a Wildcard term must not start with one of the wildcards <code>*</code> or
  * <code>?</code>.
  *
  * @see WildcardTermEnum
  */
class CLUCENE_EXPORT WildcardQuery: public MultiTermQuery {
protected:
  FilteredTermEnum* getEnum(CL_NS(index)::IndexReader* reader);
  WildcardQuery(const WildcardQuery& clone);
public:
  WildcardQuery(CL_NS(index)::Term* term);
  ~WildcardQuery();

  const char* getObjectName() const;
  static const char* getClassName();

  size_t hashCode() const;
  bool equals(Query* other) const;
  Query* clone() const;

  Query* rewrite(CL_NS(index)::IndexReader* reader);
private:
  bool termContainsWildcard;
};
  
    
    
class CLUCENE_EXPORT WildcardFilter: public Filter 
{
private:
	CL_NS(index)::Term* term;
protected:
	WildcardFilter( const WildcardFilter& copy );
	
public:
	WildcardFilter(CL_NS(index)::Term* term);
	~WildcardFilter();

	/** Returns a BitSet with true for documents which should be permitted in
	search results, and false for those that should not. */
	CL_NS(util)::BitSet* bits( CL_NS(index)::IndexReader* reader );
	
	Filter* clone() const;
	TCHAR* toString();
};


CL_NS_END
#endif
