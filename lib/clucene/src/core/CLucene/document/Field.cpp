/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Field.h"
#include "CLucene/util/_StringIntern.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_USE(util)
CL_NS_DEF(document)

Field::Field(const TCHAR* Name, Reader* reader, int config):
	lazy(false)
{
	CND_PRECONDITION(Name != NULL, "Name cannot be NULL");
	CND_PRECONDITION(reader != NULL, "reader cannot be NULL");

	_name        = CLStringIntern::intern( Name );
	fieldsData = reader;
	valueType = VALUE_READER;

	boost=1.0f;

	setConfig(config);
}


Field::Field(const TCHAR* Name, const TCHAR* Value, int _config, const bool duplicateValue):
	lazy(false)
{
	CND_PRECONDITION(Name != NULL, "Name cannot be NULL");
	CND_PRECONDITION(Value != NULL, "value cannot be NULL");
	CND_PRECONDITION(_tcslen(Value)>0 || _tcslen(Name)>0, "name and value cannot both be empty");

	/*
	if (_config & INDEX_NO && _config & STORE_NO)
		_CLTHROWA(CL_ERR_IllegalArgument,"it doesn't make sense to have a field that is neither indexed nor stored");
	if (_config & INDEX_NO && _config & TERMVECTOR_YES)
		_CLTHROWA(CL_ERR_IllegalArgument,"cannot store term vector information for a field that is not indexed");
	*/

	_name        = CLStringIntern::intern( Name );
	if (duplicateValue)
		fieldsData = stringDuplicate( Value );
	else
		fieldsData = (void*)Value;
	valueType = VALUE_STRING;

	boost=1.0f;

	//config = INDEX_TOKENIZED; // default Field is tokenized and indexed

	setConfig(_config);
}

Field::Field(const TCHAR* Name, ValueArray<uint8_t>* Value, int config, bool duplicateValue):
	lazy(false)
{
	CND_PRECONDITION(Name != NULL, "Name cannot be NULL");
	CND_PRECONDITION(Value != NULL, "value cannot be NULL");

	_name        = CLStringIntern::intern( Name );

	if ( duplicateValue ){
		ValueArray<uint8_t>* tmp = _CLNEW ValueArray<uint8_t>(Value->length);
		memcpy(tmp->values, Value->values, Value->length * sizeof(uint8_t));
		fieldsData = tmp;
	}else{
		fieldsData = Value;
	}
	valueType = VALUE_BINARY;

	boost=1.0f;

	setConfig(config);
}

Field::Field(const TCHAR* Name, int config):
	lazy(false)
{
	CND_PRECONDITION(Name != NULL, "Name cannot be NULL");

	_name        = CLStringIntern::intern( Name );
	fieldsData = NULL;
	valueType = VALUE_NONE;

	boost=1.0f;

	if (config) setConfig(config);
}

Field::~Field(){
//Func - Destructor
//Pre  - true
//Post - Instance has been destroyed

	CLStringIntern::unintern(_name);
	_resetValue();
}


/*===============FIELDS=======================*/
const TCHAR* Field::name() const	{ return _name; } ///<returns reference
const TCHAR* Field::stringValue()	{ return (valueType & VALUE_STRING) ? static_cast<TCHAR*>(fieldsData) : NULL; } ///<returns reference
const ValueArray<uint8_t>* Field::binaryValue() { return (valueType & VALUE_BINARY) ? static_cast<ValueArray<uint8_t>*>(fieldsData) : NULL; } ///<returns reference
Reader* Field::readerValue()	{ return (valueType & VALUE_READER) ? static_cast<Reader*>(fieldsData) : NULL; } ///<returns reference
CL_NS(analysis)::TokenStream* Field::tokenStreamValue() { return (valueType & VALUE_TOKENSTREAM) ? static_cast<CL_NS(analysis)::TokenStream*>(fieldsData) : NULL; }

bool	Field::isStored() const 	{ return (config & STORE_YES) != 0; }
bool 	Field::isIndexed() const	{ return (config & INDEX_TOKENIZED)!=0 || (config & INDEX_UNTOKENIZED)!=0; }
bool 	Field::isTokenized() const	{ return (config & INDEX_TOKENIZED) != 0; }
bool 	Field::isCompressed() const	{ return (config & STORE_COMPRESS) != 0; }
bool 	Field::isBinary() const		{ return (valueType & VALUE_BINARY) && fieldsData!=NULL; }

bool	Field::isTermVectorStored() const { return (config & TERMVECTOR_YES) != 0; }
bool	Field::isStoreOffsetWithTermVector() const { return (config & TERMVECTOR_YES) != 0 && (config & TERMVECTOR_WITH_OFFSETS) != 0 && ((config & TERMVECTOR_WITH_OFFSETS) != TERMVECTOR_YES); }
bool	Field::isStorePositionWithTermVector() const { return (config & TERMVECTOR_YES) != 0 && (config & TERMVECTOR_WITH_POSITIONS) != 0 && ((config & TERMVECTOR_WITH_POSITIONS) != TERMVECTOR_YES); }

bool Field::getOmitNorms() const { return (config & INDEX_NONORMS) != 0; }
void Field::setOmitNorms(const bool omitNorms) {
    if ( omitNorms )
        config |= INDEX_NONORMS;
    else
        config &= ~INDEX_NONORMS;
}

bool Field::isLazy() const { return lazy; }

void Field::setValue(TCHAR* value, const bool duplicateValue) {
	_resetValue();
	if (duplicateValue)
		fieldsData = stringDuplicate( value );
	else
		fieldsData = value;
	valueType = VALUE_STRING;
}

void Field::setValue(Reader* value) {
	_resetValue();
	fieldsData = value;
	valueType = VALUE_READER;
}
void Field::setValue(ValueArray<uint8_t>* value) {
	_resetValue();
	fieldsData = value;
	valueType = VALUE_BINARY;
}

/** Expert: change the value of this field.  See <a href="#setValue(java.lang.String)">setValue(String)</a>. */
void Field::setValue(CL_NS(analysis)::TokenStream* value) {
	_resetValue();
    fieldsData = value;
	valueType = VALUE_TOKENSTREAM;
}

void Field::setBoost(const float_t boost)	{ this->boost = boost; }
float_t Field::getBoost() const				{ return boost; }

void Field::setConfig(const uint32_t x){
	uint32_t newConfig=0;

	//set storage settings
	if ( (x & STORE_YES) || (x & STORE_COMPRESS) ){
		newConfig |= STORE_YES;
		if ( x & STORE_COMPRESS )
			newConfig |= STORE_COMPRESS;
	} else
		newConfig |= STORE_NO;

	if ( (x & INDEX_NO)==0 ){
		bool index=false;

		if ( x & INDEX_TOKENIZED && x & INDEX_UNTOKENIZED )
			_CLTHROWA(CL_ERR_IllegalArgument,"it doesn't make sense to have an untokenised and tokenised field");

		if ( x & INDEX_NONORMS ){
			newConfig |= INDEX_UNTOKENIZED;
			newConfig |= INDEX_NONORMS;
			index = true;
		}
		 else if ( x & INDEX_TOKENIZED ){
			newConfig |= INDEX_TOKENIZED;
			index = true;
		}
		else if ( x & INDEX_UNTOKENIZED ){
			newConfig |= INDEX_UNTOKENIZED;
			index = true;
		}

		if ( !index )
			newConfig |= INDEX_NO;
	}else
		newConfig |= INDEX_NO;

	if ( newConfig & INDEX_NO && newConfig & STORE_NO )
		_CLTHROWA(CL_ERR_IllegalArgument,"it doesn't make sense to have a field that is neither indexed nor stored");

	//set termvector settings
	if ( (x & TERMVECTOR_NO) == 0 ){
		bool termVector=false;
		if ( x & TERMVECTOR_YES ){
			termVector=true;
		}
		if ( (x & TERMVECTOR_WITH_POSITIONS) && ((x & TERMVECTOR_WITH_POSITIONS) != TERMVECTOR_YES) ){
			newConfig |= TERMVECTOR_WITH_POSITIONS;
			termVector=true;
		}
		if ( x & TERMVECTOR_WITH_OFFSETS  && ((x & TERMVECTOR_WITH_OFFSETS) != TERMVECTOR_YES) ){
			newConfig |= TERMVECTOR_WITH_OFFSETS;
			termVector=true;
		}
		// TERMVECTOR_WITH_POSITIONS_OFFSETS is being automatically handled here

		if ( termVector ){
			if ( newConfig & INDEX_NO )
				_CLTHROWA(CL_ERR_IllegalArgument,"cannot store a term vector for fields that are not indexed.");

			newConfig |= TERMVECTOR_YES;
		}else
			newConfig |= TERMVECTOR_NO;
	}else
		newConfig |= TERMVECTOR_NO;

	config = newConfig;
}

TCHAR* Field::toString() {
    StringBuffer result;
	if (isStored()) {
      result.append( _T("stored") );
	  if (isCompressed())
		  result.append( _T("/compressed"));
	  else
		  result.append( _T("/uncompressed") );
    }
    if (isIndexed()) {
      if (result.length() > 0)
        result.append( _T(",") );
      result.append( _T("indexed") );
    }
    if (isTokenized()) {
      if (result.length() > 0)
        result.append( _T(",") );
      result.append( _T("tokenized") );
    }
    if (isTermVectorStored()) {
      if (result.length() > 0)
        result.append( _T(",") );
      result.append( _T("termVector") );
    }
    if (isStoreOffsetWithTermVector()) {
      if (result.length() > 0)
        result.appendChar( ',' );
      result.append( _T("termVectorOffsets") );
    }
    if (isStorePositionWithTermVector()) {
      if (result.length() > 0)
        result.appendChar( ',' );
      result.append( _T("termVectorPosition") );
    }
    if (isBinary()) {
      if (result.length() > 0)
        result.appendChar( ',' );
      result.append( _T("binary") );
    }
    if (getOmitNorms()) {
      result.append( _T(",omitNorms") );
    }
	if (isLazy()){
      result.append( _T(",lazy") );
    }
    result.appendChar('<');
    result.append(name());
    result.appendChar(':');

	if (! isLazy() && fieldsData != NULL) {
		if (valueType & VALUE_STRING)
			result.append(static_cast<const TCHAR*>(fieldsData));
		else if (valueType & VALUE_READER)
			result.append( _T("Reader") );
		else if (valueType & VALUE_BINARY)
			result.append( _T("Binary") );
		else
			result.append( _T("NULL") );
	}

    result.appendChar('>');
    return result.toString();
}


void Field::_resetValue() {
	if (valueType & VALUE_STRING) {
		TCHAR* t = static_cast<TCHAR*>(fieldsData);
		_CLDELETE_CARRAY(t);
	} else if (valueType & VALUE_READER) {
		Reader* r = static_cast<Reader*>(fieldsData);
		_CLDELETE(r);
	} else if (valueType & VALUE_BINARY) {
		ValueArray<uint8_t>* v = static_cast<ValueArray<uint8_t>*>(fieldsData);
		_CLDELETE(v);
	}
	valueType=VALUE_NONE;
}
const char* Field::getObjectName() const{
	return getClassName();
}
const char* Field::getClassName(){
	return "Field";
}
CL_NS_END
