/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Equators.h"
#include "CLucene/util/Misc.h"

CL_NS_DEF(util)

bool Equals::Int32::operator()( const int32_t val1, const int32_t val2 ) const{
	return (val1)==(val2);
}

bool Equals::Char::operator()( const char* val1, const char* val2 ) const{
	if ( val1 == val2 )
		return true;
	return (strcmp( val1,val2 ) == 0);
}

#ifdef _UCS2
bool Equals::WChar::operator()( const wchar_t* val1, const wchar_t* val2 ) const{
	if ( val1 == val2 )
		return true;
	return (_tcscmp( val1,val2 ) == 0);
}
#endif

AbstractDeletor::~AbstractDeletor(){
}

////////////////////////////////////////////////////////////////////////////////
// Comparors
////////////////////////////////////////////////////////////////////////////////
int32_t compare(Comparable* o1, Comparable* o2){
  if ( o1 == NULL && o2 == NULL )
    return 0;
  else if ( o1 == NULL )
    return 1;
  else if ( o2 == NULL )
    return -1;
  else
    return o1->compareTo(o2);
}
NamedObject::~NamedObject(){
}
bool NamedObject::instanceOf(const char* other) const{
  const char* t = this->getObjectName();
	if ( t==other || strcmp( t, other )==0 )
		return true;
	else
		return false;
}

int32_t Compare::Int32::getValue() const{ return value; }
Compare::Int32::Int32(int32_t val){
	value = val;
}
Compare::Int32::Int32(){
	value = 0;
}
const char* Compare::Int32::getClassName(){
	return "Compare::Int32::getClassName";
}
const char* Compare::Int32::getObjectName() const{
	return getClassName();
}
int32_t Compare::Int32::compareTo(NamedObject* o){
	if ( o->getObjectName() != Int32::getClassName() ) return -1;

	Int32* other = (Int32*)o;
	if (value == other->value)
		return 0;
	// Returns just -1 or 1 on inequality; doing math might overflow.
	return value > other->value ? 1 : -1;
}

bool Compare::Int32::operator()( int32_t t1, int32_t t2 ) const{
	return t1 > t2 ? true : false;
}
size_t Compare::Int32::operator()( int32_t t ) const{
	return t;
}


float_t Compare::Float::getValue() const{
	return value;
}
Compare::Float::Float(float_t val){
	value = val;
}
const char* Compare::Float::getClassName(){
	return "Compare::Float::getClassName";
}
const char* Compare::Float::getObjectName() const{
	return getClassName();
}
int32_t Compare::Float::compareTo(NamedObject* o){
	if ( o->getObjectName() != Float::getClassName() ) return -1;
	Float* other = (Float*)o;
	if (value == other->value)
		return 0;
	// Returns just -1 or 1 on inequality; doing math might overflow.
	return value > other->value ? 1 : -1;
}


bool Compare::Char::operator()( const char* val1, const char* val2 ) const{
	if ( val1==val2)
		return false;
	return (strcmp( val1,val2 ) < 0);
}
size_t Compare::Char::operator()( const char* val1) const{
	return CL_NS(util)::Misc::ahashCode(val1);
}
const char* Compare::Char::getValue() const{ return s; }

Compare::Char::Char(){
	s=NULL;
}
 Compare::Char::Char(const char* str){
	this->s = str;
}
const char* Compare::Char::getClassName(){
	return "Compare::Char::getClassName";
}
const char* Compare::Char::getObjectName() const{
	return getClassName();
}
int32_t Compare::Char::compareTo(NamedObject* o){
	if ( o->getObjectName() != Char::getClassName() ) return -1;
	Char* os = (Char*)o;
	return strcmp(s,os->s);
}

#ifdef _UCS2
bool Compare::WChar::operator()( const wchar_t* val1, const wchar_t* val2 ) const{
	if ( val1==val2)
		return false;
	bool ret = (_tcscmp( val1,val2 ) < 0);
	return ret;
}
size_t Compare::WChar::operator()( const wchar_t* val1) const{
	return CL_NS(util)::Misc::whashCode(val1);
}

const wchar_t* Compare::WChar::getValue() const{ return s; }

Compare::WChar::WChar(){
	s=NULL;
}
 Compare::WChar::WChar(const wchar_t* str){
	this->s = str;
}
const char* Compare::WChar::getClassName(){
	return "Compare::WChar::getClassName";
}
const char* Compare::WChar::getObjectName() const{
	return getClassName();
}
int32_t Compare::WChar::compareTo(NamedObject* o){
	if ( o->getObjectName() != WChar::getClassName() ) return -1;
	TChar* os = (TChar*)o;
	return _tcscmp(s,os->s);
}

#endif


CL_NS_END
