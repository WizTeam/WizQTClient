/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"

#include "FieldSelector.h"

CL_NS_USE(util)
CL_NS_DEF(document)

FieldSelector::~FieldSelector(){
}

LoadFirstFieldSelector::~LoadFirstFieldSelector(){
}

FieldSelector::FieldSelectorResult LoadFirstFieldSelector::accept(const TCHAR* /*fieldName*/) const{
	return LOAD_AND_BREAK;
}

MapFieldSelector::~MapFieldSelector(){
    _CLDELETE(fieldSelections);
}

MapFieldSelector::MapFieldSelector() :
  fieldSelections(_CLNEW FieldSelectionsType(true,false))
{
}

MapFieldSelector::MapFieldSelector(std::vector<const TCHAR*>& fields) :
  fieldSelections(_CLNEW FieldSelectionsType(true,false))
{
  std::vector<const TCHAR*>::iterator itr = fields.begin();
  while ( itr != fields.end() ){
    add(*itr);
    itr++;
  }
}

MapFieldSelector::MapFieldSelector(ArrayBase<const TCHAR*>& fields):
  fieldSelections(_CLNEW FieldSelectionsType(true,false))
{
  for ( size_t i=0;i<fields.length;i++ ){
    add(fields[i]);
  }
}

FieldSelector::FieldSelectorResult MapFieldSelector::accept(const TCHAR* field) const{
  FieldSelectionsType::iterator itr = fieldSelections->find((TCHAR*)field);
  if ( itr != fieldSelections->end() ){
      return itr->second;
  }
  return FieldSelector::NO_LOAD;
}

void MapFieldSelector::add(const TCHAR* field, FieldSelector::FieldSelectorResult action){
  fieldSelections->insert(fieldSelections->end(),std::pair<TCHAR*,FieldSelectorResult>(
    STRDUP_TtoT(field), action));
}


CL_NS_END
