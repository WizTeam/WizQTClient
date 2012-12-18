/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"
#include "CLucene/util/BitSet.h"
#include "RangeFilter.h"

CL_NS_DEF(search)
CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(document)


RangeFilter::RangeFilter( const TCHAR* _fieldName, const TCHAR* _lowerTerm, const TCHAR* _upperTerm,
                         bool _includeLower, bool _includeUpper )
                         : fieldName(NULL), lowerTerm(NULL), upperTerm(NULL)
                         , includeLower(_includeLower), includeUpper(_includeUpper)
{
    if (NULL == _lowerTerm && NULL == _upperTerm) {
        _CLTHROWT(CL_ERR_IllegalArgument, _T("At least one value must be non-null"));
    }
    if (_includeLower && NULL == _lowerTerm) {
        _CLTHROWT(CL_ERR_IllegalArgument, _T("The lower bound must be non-null to be inclusive"));
    }
    if (_includeUpper && NULL == _upperTerm) {
        _CLTHROWT(CL_ERR_IllegalArgument, _T("The upper bound must be non-null to be inclusive"));
    }

	this->fieldName = STRDUP_TtoT(_fieldName);
	if ( _lowerTerm != NULL )
		this->lowerTerm = STRDUP_TtoT(_lowerTerm);
	if ( _upperTerm != NULL )
		this->upperTerm = STRDUP_TtoT(_upperTerm);
}

RangeFilter::RangeFilter( const RangeFilter& copy ) : 
	fieldName( STRDUP_TtoT(copy.fieldName) ),
	lowerTerm( STRDUP_TtoT(copy.lowerTerm) ), 
	upperTerm( STRDUP_TtoT(copy.upperTerm) ),
	includeLower( copy.includeLower ),
	includeUpper( copy.includeUpper )
{
}

RangeFilter::~RangeFilter()
{
    _CLDELETE_LCARRAY( fieldName );
	_CLDELETE_LCARRAY( lowerTerm );
	_CLDELETE_LCARRAY( upperTerm );
}

RangeFilter* RangeFilter::Less( const TCHAR* _fieldName, const TCHAR* _upperTerm ) {
	return _CLNEW RangeFilter( _fieldName, NULL, _upperTerm, false, true );
}

RangeFilter* RangeFilter::More( const TCHAR* _fieldName, const TCHAR* _lowerTerm ) {
	return _CLNEW RangeFilter( _fieldName, _lowerTerm, NULL, true, false );
}

BitSet* RangeFilter::bits( IndexReader* reader )
{
	BitSet* bts = _CLNEW BitSet( reader->maxDoc() );
	Term* term = NULL;
	
	Term* t = _CLNEW Term( fieldName, (lowerTerm ? lowerTerm : _T("")), false );
	TermEnum* enumerator = reader->terms( t );	// get enumeration of all terms after lowerValue
	_CLDECDELETE( t );
	
	if( enumerator->term(false) == NULL ) {
		_CLLDELETE( enumerator );
		return bts;
	}
	
	bool checkLower = false;
	if( !includeLower ) // make adjustments to set to exclusive
		checkLower = true;
	
	TermDocs* termDocs = reader->termDocs();
	
  #define CLEANUP \
    _CLLDECDELETE( term ); \
    termDocs->close(); \
    _CLLDELETE( termDocs ); \
    enumerator->close(); \
    _CLLDELETE( enumerator )
    
	try
	{
		do
		{
			term = enumerator->term();
			
			if( term == NULL || _tcscmp(term->field(), fieldName) )
				break;
			
			if( !checkLower || lowerTerm == NULL || _tcscmp(term->text(), lowerTerm) > 0 )
			{
				checkLower = false;
				if( upperTerm != NULL )
				{
					int compare = _tcscmp( upperTerm, term->text() );
					
					/* if beyond the upper term, or is exclusive and
					 * this is equal to the upper term, break out */
					if( (compare < 0) || (!includeUpper && compare == 0) )
						break;
				}
				
				termDocs->seek( enumerator->term(false) );
				while( termDocs->next() ) {
					bts->set( termDocs->doc() );
				}
			}
			
			_CLDECDELETE( term );
		}
		while( enumerator->next() );
	}catch(CLuceneError& err){
    _CLDELETE(bts);
    CLEANUP;
    throw err;
  }
  CLEANUP;
	
	return bts;
}

TCHAR* RangeFilter::toString()
{
	size_t len = (fieldName ? _tcslen(fieldName) : 0) + (lowerTerm ? _tcslen(lowerTerm) : 0) + (upperTerm ? _tcslen(upperTerm) : 0) + 8;
	TCHAR* ret = _CL_NEWARRAY( TCHAR, len );
	ret[0] = 0;
	_sntprintf( ret, len, _T("%s: [%s-%s]"), fieldName, (lowerTerm?lowerTerm:_T("")), (upperTerm?upperTerm:_T("")) );
	
	return ret;
}

Filter* RangeFilter::clone() const {
	return _CLNEW RangeFilter(*this );
}

CL_NS_END
