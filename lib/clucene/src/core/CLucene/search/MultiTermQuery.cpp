/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "MultiTermQuery.h"
#include "BooleanQuery.h"
#include "FilteredTermEnum.h"
#include "TermQuery.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

/** Constructs a query for terms matching <code>term</code>. */

  MultiTermQuery::MultiTermQuery(Term* t){
  //Func - Constructor
  //Pre  - t != NULL
  //Post - The instance has been created

      CND_PRECONDITION(t != NULL, "t is NULL");

      term  = _CL_POINTER(t);

  }
  MultiTermQuery::MultiTermQuery(const MultiTermQuery& clone):
  	Query(clone)	
  {
	term = _CLNEW Term(clone.getTerm(false),clone.getTerm(false)->text());
  }

  MultiTermQuery::~MultiTermQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      _CLDECDELETE(term);
  }

  Term* MultiTermQuery::getTerm(bool pointer) const{
	if ( pointer )
		return _CL_POINTER(term);
	else
		return term;
  }

	Query* MultiTermQuery::rewrite(IndexReader* reader) {
		FilteredTermEnum* enumerator = getEnum(reader);
		BooleanQuery* query = _CLNEW BooleanQuery( true );
		try {
            do {
                Term* t = enumerator->term(false);
                if (t != NULL) {
                    TermQuery* tq = _CLNEW TermQuery(t);	// found a match
                    tq->setBoost(getBoost() * enumerator->difference()); // set the boost
                    query->add(tq,true, false, false);		// add to q
                }
            } while (enumerator->next());
        } _CLFINALLY ( enumerator->close(); _CLDELETE(enumerator) );

		//if we only added one clause and the clause is not prohibited then
		//we can just return the query
		if (query->getClauseCount() == 1) {                    // optimize 1-clause queries
			BooleanClause* c=0;
		    query->getClauses(&c);

			if (!c->prohibited) {			  // just return clause
				c->deleteQuery=false;
				Query* ret = c->getQuery();

				_CLDELETE(query);
				return ret;
            }
        }
		return query;
	}
	
	Query* MultiTermQuery::combine(CL_NS(util)::ArrayBase<Query*>* queries) {
		return Query::mergeBooleanQueries(queries);
  }

    /** Prints a user-readable version of this query. */
    TCHAR* MultiTermQuery::toString(const TCHAR* field) const{
        StringBuffer buffer;

        if ( field==NULL || _tcscmp(term->field(),field)!=0 ) {
            buffer.append(term->field());
            buffer.append( _T(":"));
        }
        buffer.append(term->text());
		// todo: use ToStringUtils.boost()
        if (getBoost() != 1.0f) {
            buffer.appendChar ( '^' );
            buffer.appendFloat( getBoost(),1);
        }
        return buffer.toString();
    }

CL_NS_END
