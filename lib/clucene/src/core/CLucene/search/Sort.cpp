/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Sort.h"
#include "Compare.h"
#include "SearchHeader.h"
#include "CLucene/util/_StringIntern.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_USE(util)
CL_NS_DEF(search)


  SortField* SortField_FIELD_SCORE = NULL;
  SortField* SortField_FIELD_DOC = NULL;
  SortField* SortField::FIELD_SCORE(){
    if ( SortField_FIELD_SCORE == NULL )
        SortField_FIELD_SCORE = _CLNEW SortField (NULL, DOCSCORE,false);
    return SortField_FIELD_SCORE;
  }
  SortField* SortField::FIELD_DOC(){
    if ( SortField_FIELD_DOC == NULL )
        SortField_FIELD_DOC = _CLNEW SortField (NULL, DOC,false);
    return SortField_FIELD_DOC;
  }
  void SortField::_shutdown(){
    _CLDELETE(SortField_FIELD_SCORE);
    _CLDELETE(SortField_FIELD_DOC);
  }
  
  Sort* Sort_RELEVANCE = NULL;
  Sort* Sort_INDEXORDER = NULL;
  Sort* Sort::RELEVANCE(){
    if ( Sort_RELEVANCE == NULL )
        Sort_RELEVANCE = _CLNEW Sort();
    return Sort_RELEVANCE;
  }
  Sort* Sort::INDEXORDER(){
    if ( Sort_INDEXORDER == NULL )
        Sort_INDEXORDER = _CLNEW Sort (SortField::FIELD_DOC());
    return Sort_INDEXORDER;
  }
  void Sort::_shutdown(){
    _CLDELETE(Sort_RELEVANCE);
    _CLDELETE(Sort_INDEXORDER);
  }

  ScoreDocComparator* ScoreDocComparator_INDEXORDER = NULL;
  ScoreDocComparator* ScoreDocComparator_RELEVANCE = NULL;
  ScoreDocComparator* ScoreDocComparator::INDEXORDER(){
    if ( ScoreDocComparator_INDEXORDER == NULL )
        ScoreDocComparator_INDEXORDER = _CLNEW ScoreDocComparators::IndexOrder;
    return ScoreDocComparator_INDEXORDER;
  }
  ScoreDocComparator* ScoreDocComparator::RELEVANCE(){
    if ( ScoreDocComparator_RELEVANCE == NULL )
        ScoreDocComparator_RELEVANCE = _CLNEW ScoreDocComparators::Relevance;
    return ScoreDocComparator_RELEVANCE;
  }
  void ScoreDocComparator::_shutdown(){
    _CLDELETE(ScoreDocComparator_INDEXORDER);
    _CLDELETE(ScoreDocComparator_RELEVANCE);
  }



  SortField::SortField (const TCHAR* field) {
     this->type = AUTO;
     this->reverse = false;
     this->field = CLStringIntern::intern(field);
	 this->factory = NULL;
  }

  SortField::SortField (const TCHAR* field, int32_t type, bool reverse) {
    this->field = (field != NULL) ? CLStringIntern::intern(field) : field;
    this->type = type;
    this->reverse = reverse;
	 this->factory = NULL;
  }
  
  SortField::SortField(const SortField& clone){
    this->field = (clone.field != NULL) ? CLStringIntern::intern(clone.field) : clone.field;
    this->type = clone.type;
    this->reverse = clone.reverse;
	 this->factory = clone.factory;
  }
  SortField* SortField::clone() const{
   return _CLNEW SortField(*this); 
  }

  const TCHAR* SortField::getField() const { 
    return field; 
  }
  int32_t SortField::getType() const { 
        return type; 
  }
  bool SortField::getReverse() const { 
        return reverse; 
  }
  SortComparatorSource* SortField::getFactory() const { 
        return factory; 
  }
  
  /** Creates a sort by terms in the given field sorted
   * according to the given locale.
   * @param field  Name of field to sort by, cannot be <code>null</code>.
   * @param locale Locale of values in the field.
   */
  /*SortField::SortField (TCHAR* field, Locale* locale) {
    this->field = (field != NULL) ? CLStringIntern::intern(field): field;
    this->type = STRING;
    this->locale = locale;
  }*/

  /** Creates a sort, possibly in reverse, by terms in the given field sorted
   * according to the given locale.
   * @param field  Name of field to sort by, cannot be <code>null</code>.
   * @param locale Locale of values in the field.
   */
  /*SortField::SortField (TCHAR* field, Locale* locale, bool reverse) {
     this->field = (field != NULL) ? CLStringIntern::intern(field): field;
    this->type = STRING;
    this->locale = locale;
    this->reverse = reverse;
  }*/


  SortField::SortField (const TCHAR* field, SortComparatorSource* comparator, bool reverse) {
    this->field = (field != NULL) ? CLStringIntern::intern(field): field;
    this->type = CUSTOM;
    this->reverse = reverse;
    this->factory = comparator;
  }

  SortField::~SortField(){
	  CLStringIntern::unintern(field);
  }
  
  TCHAR* SortField::toString() const {
	CL_NS(util)::StringBuffer buffer;
    switch (type) {
      case DOCSCORE:
        buffer.append(_T("<score>"));
        break;

      case DOC:
        buffer.append(_T("<doc>"));
        break;

      case CUSTOM:
        buffer.append (_T("<custom:\""));
        buffer.append( field );
        buffer.append( _T("\": "));
        buffer.append(factory->getName());
        buffer.append(_T(">"));
        break;

      default:
        buffer.append( _T("\""));
        buffer.append( field );
        buffer.append( _T("\"") );
        break;
    }

    //if (locale != null) buffer.append ("("+locale+")"); todo:
    if (reverse) buffer.appendChar('!');

    return buffer.toString();
  }












	Sort::Sort() {
		fields=NULL;
		SortField** fields=_CL_NEWARRAY(SortField*,3);
		fields[0]=SortField::FIELD_SCORE();
		fields[1]=SortField::FIELD_DOC();
		fields[2]=NULL;
		setSort (fields);
        _CLDELETE_ARRAY(fields);
	}

	Sort::~Sort(){
		clear();
	}
	void Sort::clear(){
		if ( fields != NULL ){
			int32_t i=0;
			while ( fields[i] != NULL ){
				if ( fields[i] != SortField::FIELD_SCORE() &&
					 fields[i] != SortField::FIELD_DOC() ){
					_CLDELETE(fields[i]);
				}
				i++;
			}
			_CLDELETE_ARRAY(fields);
		}
	}
	
	Sort::Sort (const TCHAR* field, bool reverse) {
		this->fields=NULL;
		setSort (field, reverse);
	}

	Sort::Sort (const TCHAR** fields) {
		this->fields=NULL;
		setSort (fields);
	}
	Sort::Sort (SortField* field) {
		this->fields=NULL;
		setSort (field);
	}

	Sort::Sort (SortField** fields) {
		this->fields=NULL;
		setSort (fields);
	}

	void Sort::setSort (const TCHAR* field, bool reverse) {
		clear();
		fields = _CL_NEWARRAY(SortField*,3);
		fields[0] = _CLNEW SortField (field, SortField::AUTO, reverse);
		fields[1] = SortField::FIELD_DOC();
		fields[2] = NULL;
	}

	void Sort::setSort (const TCHAR** fieldnames) {
		clear();

		int32_t n = 0;
		while ( fieldnames[n] != NULL )
		   n++;

		fields = _CL_NEWARRAY(SortField*,n+1);
		for (int32_t i = 0; i < n; ++i) {
			fields[i] = _CLNEW SortField (fieldnames[i], SortField::AUTO,false);
		}
		fields[n]=NULL;
	}


	void Sort::setSort (SortField* field) {
		clear();

        this->fields = _CL_NEWARRAY(SortField*,2);
		this->fields[0] = field;
		this->fields[1] = NULL;
	}

	void Sort::setSort (SortField** fields) {
		clear();
        
        int n=0;
		while ( fields[n] != NULL )
		   n++;
        this->fields = _CL_NEWARRAY(SortField*,n+1);
        for (int i=0;i<n+1;i++)
            this->fields[i]=fields[i];
	}

	TCHAR* Sort::toString() const {
		CL_NS(util)::StringBuffer buffer;

		int32_t i = 0;
		while ( fields[i] != NULL ){
			if (i>0)
				buffer.appendChar(',');

			TCHAR* p = fields[i]->toString();
			buffer.append(p);
			_CLDELETE_CARRAY(p);
			  
			i++;
		}

		return buffer.toString();
	}






  ScoreDocComparator::~ScoreDocComparator(){
  }


class ScoreDocComparatorImpl: public ScoreDocComparator{
    Comparable** cachedValues;
	FieldCacheAuto* fca;
	int32_t cachedValuesLen;
public:
	ScoreDocComparatorImpl(FieldCacheAuto* fca){
		this->fca = fca;
		if ( fca->contentType != FieldCacheAuto::COMPARABLE_ARRAY )
		_CLTHROWA(CL_ERR_InvalidCast,"Invalid field cache auto type");
		this->cachedValues = fca->comparableArray;
		this->cachedValuesLen = fca->contentLen;
	}
	~ScoreDocComparatorImpl(){
	}
	int32_t compare (struct ScoreDoc* i, struct ScoreDoc* j){
		CND_PRECONDITION(i->doc >= 0 && i->doc < cachedValuesLen, "i->doc out of range")
		CND_PRECONDITION(j->doc >= 0 && j->doc < cachedValuesLen, "j->doc out of range")
		return cachedValues[i->doc]->compareTo (cachedValues[j->doc]);
	}

	CL_NS(util)::Comparable* sortValue (struct ScoreDoc* i){
		CND_PRECONDITION(i->doc >= 0 && i->doc < cachedValuesLen, "i->doc out of range")
		return cachedValues[i->doc];
	}

	int32_t sortType(){
		return SortField::CUSTOM;
	}
};
    
ScoreDocComparator* SortComparator::newComparator (CL_NS(index)::IndexReader* reader, const TCHAR* fieldname){
	return _CLNEW ScoreDocComparatorImpl(FieldCache::DEFAULT()->getCustom (reader, fieldname, this));
}
SortComparator::SortComparator(){
}
SortComparator::~SortComparator(){
}


CL_NS_END
