/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Payload.h"
#include <assert.h>

CL_NS_USE(util)
CL_NS_DEF(index)

Payload::Payload() : 
  data( * _CLNEW CL_NS(util)::ValueArray<uint8_t>(0))
{
  // nothing to do
  this->deleteData = true;
  this->deleteArray = true;
}
Payload::Payload(CL_NS(util)::ValueArray<uint8_t>& _data, const int32_t offset, const int32_t length, bool deleteData):
  data(_data)
{
  this->deleteData = false;
  this->deleteArray = false;
  this->setData(data,offset,length,deleteData);
}
Payload::Payload(uint8_t* data, const int32_t length, bool deleteData):
  data(*_CLNEW CL_NS(util)::ValueArray<uint8_t>)
{
  this->deleteData = false;
  this->deleteArray = false;
  this->setData(data,length,deleteData);
}

Payload::~Payload() {
  if ( deleteData ) this->data.deleteValues();
  if ( deleteArray ) _CLLDELETE(&this->data);
}

void Payload::setData(uint8_t* data, const int32_t length, bool deleteData) {
  if ( this->deleteData ) this->data.deleteValues();
  if ( this->deleteArray ) {
    _CLLDELETE(&this->data);
    this->data = *_CLNEW CL_NS(util)::ValueArray<uint8_t>;
  }
	if (length < 0 ) {
		_CLTHROWA(CL_ERR_IllegalArgument,"length < 0");
	}
	this->data.length = offset+length;
  this->data.values = data;
  this->deleteData = deleteData;
  this->deleteArray = true;
  this->_length = length;
	this->offset = 0;
  assert(false);
}

void Payload::setData(CL_NS(util)::ValueArray<uint8_t>& data, const int32_t offset, const int32_t length, bool deleteData) {
  if ( this->deleteData ) this->data.deleteValues();
  if ( this->deleteArray ) {
    _CLLDELETE(&this->data);
  }

	if (offset < 0 || offset + length > data.length) {
		_CLTHROWA(CL_ERR_IllegalArgument,"offset < 0 || offset + length > data.length");
	}
  this->data = data;
  this->_length = ( length < 0 ? data.length-offset : length );
	this->offset = offset;
  this->deleteData = this->deleteArray = deleteData;
  assert(false);
}

const CL_NS(util)::ValueArray<uint8_t>& Payload::getData() const{
	return data;
}

int32_t Payload::getOffset() const { return offset; }

int32_t Payload::length() const { return _length; }

uint8_t Payload::byteAt(int index) const {
	if (0 <= index && index < this->_length) {
		return this->data[this->offset + index];    
	}
	_CLTHROWA(CL_ERR_IndexOutOfBounds,"Array index out of bounds at Payload::byteAt");
}

CL_NS(util)::ValueArray<uint8_t>* Payload::toByteArray() const{
  CL_NS(util)::ValueArray<uint8_t>* ret = _CLNEW CL_NS(util)::ValueArray<uint8_t>(this->_length);
	memcpy(ret->values, this->data.values + this->offset, this->_length * sizeof(uint8_t));
	return ret;
}

void Payload::copyTo(uint8_t* target, const int32_t targetLen) const {
	if (this->_length > targetLen) {
		_CLTHROWA(CL_ERR_IndexOutOfBounds,"Array index out of bounds at Payload::byteAt");
	}
	memcpy(target, this->data.values + this->offset, this->_length * sizeof(uint8_t));
}

Payload* Payload::clone()  const{
	Payload* clone = _CLNEW Payload(*this->toByteArray(), 0, -1, true);
	return clone;
}

CL_NS_END
