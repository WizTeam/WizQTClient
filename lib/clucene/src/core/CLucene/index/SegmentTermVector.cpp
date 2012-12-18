/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_FieldInfos.h"
#include "_TermVector.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/Array.h"

CL_NS_USE(util)
CL_NS_DEF(index)

ValueArray<int32_t> SegmentTermPositionVector::EMPTY_TERM_POS;

SegmentTermVector::SegmentTermVector(const TCHAR* _field,
    ArrayBase<TCHAR*>* _terms, ArrayBase<int32_t>* _termFreqs) {
	this->field = STRDUP_TtoT(_field); // TODO: Try and avoid this dup (using intern'ing perhaps?)
	this->terms = _terms;
	this->termFreqs = _termFreqs;
}

SegmentTermVector::~SegmentTermVector(){
  _CLDELETE_LCARRAY(field);
  _CLDELETE(terms);
  _CLDELETE(termFreqs);
}
TermPositionVector* SegmentTermVector::__asTermPositionVector(){
	return NULL;
}

const TCHAR* SegmentTermVector::getField() {
	return field;
}

TCHAR* SegmentTermVector::toString() const{
	StringBuffer sb;
	sb.appendChar('{');
	sb.append(field);
	sb.append(_T(": "));

	int32_t i=0;
	while ( terms && terms->values[i] != NULL ){
		if (i>0)
			sb.append(_T(", "));
		sb.append(terms->values[i]);
		sb.appendChar('/');

		sb.appendInt((*termFreqs)[i]);
	}
	sb.appendChar('}');
	return sb.toString();
}

int32_t SegmentTermVector::size() {
	if ( terms == NULL )
		return 0;

	return terms->length;
}

const CL_NS(util)::ArrayBase<const TCHAR*>* SegmentTermVector::getTerms() {
	return (CL_NS(util)::ArrayBase<const TCHAR*>*)terms;
}

const ArrayBase<int32_t>* SegmentTermVector::getTermFrequencies() {
	return termFreqs;
}

int32_t SegmentTermVector::binarySearch(const ArrayBase<TCHAR*>& a, const TCHAR* key) const
{
	int32_t low = 0;
	int32_t hi = a.length - 1;
	int32_t mid = 0;
	while (low <= hi)
	{
		mid = (low + hi) >> 1;

		int32_t c = _tcscmp(a[mid],key);
		if (c==0)
			return mid;
		else if (c > 0)
			hi = mid - 1;
		else // This gets the insertion point right on the last loop.
			low = ++mid;
	}
	return -mid - 1;
}

int32_t SegmentTermVector::indexOf(const TCHAR* termText) {
	if(terms == NULL)
		return -1;
	int32_t res = binarySearch(*terms, termText);
	return res >= 0 ? res : -1;
}

ArrayBase<int32_t>* SegmentTermVector::indexesOf(const CL_NS(util)::ArrayBase<TCHAR*>& termNumbers, const int32_t start, const int32_t len) {
	// TODO: there must be a more efficient way of doing this.
	//       At least, we could advance the lower bound of the terms array
	//       as we find valid indexes. Also, it might be possible to leverage
	//       this even more by starting in the middle of the termNumbers array
	//       and thus dividing the terms array maybe in half with each found index.
	ArrayBase<int32_t>* ret = _CLNEW ValueArray<int32_t>(len);
	for (int32_t i=0; i<len; ++i) {
	  ret->values[i] = indexOf(termNumbers[start+ i]);
	}
	return ret;
}


SegmentTermPositionVector::SegmentTermPositionVector(const TCHAR* field,
  ArrayBase<TCHAR*>* terms, ArrayBase<int32_t>* termFreqs,
    ArrayBase< ArrayBase<int32_t>* >* _positions,
    ArrayBase< ArrayBase<TermVectorOffsetInfo*>* >* _offsets)
	: SegmentTermVector(field,terms,termFreqs),
  positions(_positions),
  offsets(_offsets)
{
}
SegmentTermPositionVector::~SegmentTermPositionVector(){
	_CLLDELETE(offsets);
	_CLLDELETE(positions);
}

ArrayBase<int32_t>* SegmentTermPositionVector::indexesOf(const ArrayBase<TCHAR*>& termNumbers, const int32_t start, const int32_t len)
{
  return SegmentTermVector::indexesOf(termNumbers, start, len);
}

TermPositionVector* SegmentTermPositionVector::__asTermPositionVector(){
	return this;
}

const ArrayBase<TermVectorOffsetInfo*>* SegmentTermPositionVector::getOffsets(const size_t index) {
	if(offsets == NULL)
		return NULL;
	if (index < offsets->length)
		return offsets->values[index];
	else
		return TermVectorOffsetInfo_EMPTY_OFFSET_INFO;
}

const ArrayBase<int32_t>* SegmentTermPositionVector::getTermPositions(const size_t index) {
	if(positions == NULL)
		return NULL;

	if (index < positions->length)
		return positions->values[index];
	else
		return &EMPTY_TERM_POS;
}

CL_NS_END

