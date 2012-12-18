/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "QueryToken.h"

CL_NS_DEF2(queryParser,legacy)

QueryToken::QueryToken():
	Value(NULL)
{
    set(UNKNOWN_);
}
QueryToken::QueryToken(const TCHAR* value, const int32_t start, const int32_t end, const QueryToken::Types type):
	Value(NULL)
{
  set(value,start,end,type);
}

QueryToken::~QueryToken(){
//Func - Destructor
//Pre  - true
//Post - Instance has been destroyed

   #ifndef LUCENE_TOKEN_WORD_LENGTH
	_CLDELETE_CARRAY( Value );
  #endif
}

// Initializes a new instance of the Token class LUCENE_EXPORT.
//
QueryToken::QueryToken(const TCHAR* value, const QueryToken::Types type):
	Value(NULL)
{
    set(value,type);
}

// Initializes a new instance of the Token class LUCENE_EXPORT.
//
QueryToken::QueryToken(QueryToken::Types type):
	Value(NULL)
{
  set(type);
}


void QueryToken::set(const TCHAR* value, const Types type){
    set(value,0,-1,type);
}
void QueryToken::set(const TCHAR* value, const int32_t start, const int32_t end, const Types type){
  #ifndef LUCENE_TOKEN_WORD_LENGTH
	_CLDELETE_CARRAY(Value);
    Value = STRDUP_TtoT(value);
  #else
    _tcsncpy(Value,value,LUCENE_TOKEN_WORD_LENGTH);
    Value[LUCENE_TOKEN_WORD_LENGTH];
  #endif
    this->Start = start;
    this->End = end;
	this->Type = type;
    
    if ( this->End < 0 )
        this->End = _tcslen(Value);
}
void QueryToken::set(Types type){
    set(LUCENE_BLANK_STRING,0,0,type);
}

CL_NS_END2
