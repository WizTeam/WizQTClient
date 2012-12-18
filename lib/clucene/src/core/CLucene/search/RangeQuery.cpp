/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "RangeQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "Similarity.h"

#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/StringBuffer.h"


CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)

	RangeQuery::RangeQuery(Term* lowerTerm, Term* upperTerm, const bool Inclusive){
	//Func - Constructor
	//Pre  - (LowerTerm != NULL OR UpperTerm != NULL) AND
	//       if LowerTerm and UpperTerm are valid pointer then the fieldnames must be the same
	//Post - The instance has been created

		if (lowerTerm == NULL && upperTerm == NULL)
        {
            _CLTHROWA(CL_ERR_IllegalArgument,"At least one term must be non-null");
        }
        if (lowerTerm != NULL && upperTerm != NULL && lowerTerm->field() != upperTerm->field())
        {
            _CLTHROWA(CL_ERR_IllegalArgument,"Both terms must be for the same field");
        }

		// if we have a lowerTerm, start there. otherwise, start at beginning
        if (lowerTerm != NULL) {
            this->lowerTerm = _CL_POINTER(lowerTerm);
        }
        else {
            this->lowerTerm = _CLNEW Term(upperTerm, LUCENE_BLANK_STRING);
        }
        this->upperTerm = (upperTerm != NULL ? _CL_POINTER(upperTerm) : NULL);
        this->inclusive = Inclusive;
    }
	RangeQuery::RangeQuery(const RangeQuery& clone):
		Query(clone){
		this->inclusive = clone.inclusive;
		this->upperTerm = (clone.upperTerm != NULL ? _CL_POINTER(clone.upperTerm) : NULL );
		this->lowerTerm = (clone.lowerTerm != NULL ? _CL_POINTER(clone.lowerTerm) : NULL );
	}
	Query* RangeQuery::clone() const{
	  return _CLNEW RangeQuery(*this);
	}

    RangeQuery::~RangeQuery() {
    //Func - Destructor
    //Pre  - true
    //Post - The instance has been destroyed

        _CLDECDELETE(lowerTerm);
        _CLDECDELETE(upperTerm);
    }

	/** Returns a hash code value for this object.*/
    size_t RangeQuery::hashCode() const {
			return Similarity::floatToByte(getBoost()) ^
	            (lowerTerm != NULL ? lowerTerm->hashCode() : 0) ^
	            (upperTerm != NULL ? upperTerm->hashCode() : 0) ^
	            (this->inclusive ? 1 : 0);
    }

    const char* RangeQuery::getObjectName() const{
      return getClassName();
    }
	const char* RangeQuery::getClassName(){
		return "RangeQuery";
	}

	Query* RangeQuery::combine(CL_NS(util)::ArrayBase<Query*>* queries) {
		return Query::mergeBooleanQueries(queries);
	}
	
	bool RangeQuery::equals(Query * other) const{
	  if (!(other->instanceOf(RangeQuery::getClassName())))
            return false;

        RangeQuery* rq = (RangeQuery*)other;
		bool ret = (this->getBoost() == rq->getBoost())
			&& (this->isInclusive() == rq->isInclusive())
			&& (this->getLowerTerm()->equals(rq->getLowerTerm()))
			&& (this->getUpperTerm()->equals(rq->getUpperTerm()));

		return ret;
	}


    Query* RangeQuery::rewrite(IndexReader* reader){

        BooleanQuery* query = _CLNEW BooleanQuery( true );
        TermEnum* enumerator = reader->terms(lowerTerm);
		Term* lastTerm = NULL;
        try {
            bool checkLower = false;
            if (!inclusive) // make adjustments to set to exclusive
                checkLower = true;

            const TCHAR* testField = getField();
            do {
                lastTerm = enumerator->term();
                if (lastTerm != NULL && lastTerm->field() == testField ) {
                    if (!checkLower || _tcscmp(lastTerm->text(),lowerTerm->text()) > 0) {
                        checkLower = false;
                        if (upperTerm != NULL) {
                            int compare = _tcscmp(upperTerm->text(),lastTerm->text());
                            /* if beyond the upper term, or is exclusive and
                             * this is equal to the upper term, break out */
                            if ((compare < 0) || (!inclusive && compare == 0))
                                break;
                        }
                        TermQuery* tq = _CLNEW TermQuery(lastTerm); // found a match
                        tq->setBoost(getBoost()); // set the boost
                        query->add(tq, true, false, false); // add to query
                    }
                }else {
                    break;
                }
				_CLDECDELETE(lastTerm);
            }
            while (enumerator->next());
		}catch(...){
			_CLDECDELETE(lastTerm); //always need to delete this
			_CLDELETE(query); //in case of error, delete the query
            enumerator->close();
			_CLDELETE(enumerator);
			throw; //rethrow
		}
		_CLDECDELETE(lastTerm); //always need to delete this
		enumerator->close();
		_CLDELETE(enumerator);

        return query;
    }

    TCHAR* RangeQuery::toString(const TCHAR* field) const
    {
        StringBuffer buffer;
        if ( field==NULL || _tcscmp(getField(),field)!=0 )
        {
            buffer.append( getField() );
            buffer.append( _T(":"));
        }
        buffer.append(inclusive ? _T("[") : _T("{"));
        buffer.append(lowerTerm != NULL ? lowerTerm->text() : _T("NULL"));
        buffer.append(_T(" TO "));
        buffer.append(upperTerm != NULL ? upperTerm->text() : _T("NULL"));
        buffer.append(inclusive ? _T("]") : _T("}"));
        if (getBoost() != 1.0f)
        {
            buffer.append( _T("^"));
            buffer.appendFloat( getBoost(),1 );
        }
        return buffer.toString();
    }


    const TCHAR* RangeQuery::getField() const	
	{
		return (lowerTerm != NULL ? lowerTerm->field() : upperTerm->field());
	}

    Term* RangeQuery::getLowerTerm(bool pointer) const { 
		if ( pointer )
			return _CL_POINTER(lowerTerm); 
		else
			return lowerTerm;
	}

    Term* RangeQuery::getUpperTerm(bool pointer) const { 
		if ( pointer )
			return _CL_POINTER(upperTerm); 
		else
			return upperTerm;
	}

    bool RangeQuery::isInclusive() const { return inclusive; }


CL_NS_END
