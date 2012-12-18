/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Compare.h"
#include "SearchHeader.h"

CL_NS_DEF(search)

ScoreDocComparators::ScoreDocComparators(){}
ScoreDocComparators::~ScoreDocComparators(){
}

int32_t ScoreDocComparators::Relevance::compare (struct ScoreDoc* i, struct ScoreDoc* j) {
	if (i->score > j->score) return -1;
	if (i->score < j->score) return 1;
	return 0;
}
CL_NS(util)::Comparable* ScoreDocComparators::Relevance::sortValue (struct ScoreDoc* i) {
	return _CLNEW CL_NS(util)::Compare::Float (i->score);
}
int32_t ScoreDocComparators::Relevance::sortType() {
	return SortField::DOCSCORE;
}



ScoreDocComparators::IndexOrder::IndexOrder():
	ScoreDocComparator()
{

}
int32_t ScoreDocComparators::IndexOrder::compare (struct ScoreDoc* i, struct ScoreDoc* j) {
	if (i->doc < j->doc) return -1;
	if (i->doc > j->doc) return 1;
	return 0;
}
CL_NS(util)::Comparable* ScoreDocComparators::IndexOrder::sortValue (struct ScoreDoc* i) {
	return _CLNEW CL_NS(util)::Compare::Int32(i->doc);
}
int32_t ScoreDocComparators::IndexOrder::sortType() {
	return SortField::DOC;
}



ScoreDocComparators::String::String(FieldCache::StringIndex* index, int32_t len)
{
	this->length = len;
	this->index = index;
}

int32_t ScoreDocComparators::String::compare (struct ScoreDoc* i, struct ScoreDoc* j) {
	CND_PRECONDITION(i->doc<length, "i->doc>=length")
	CND_PRECONDITION(j->doc<length, "j->doc>=length")
	if (index->order[i->doc] < index->order[j->doc]) return -1;
	if (index->order[i->doc] > index->order[j->doc]) return 1;
	return 0;
}

CL_NS(util)::Comparable* ScoreDocComparators::String::sortValue (struct ScoreDoc* i) {
	return _CLNEW CL_NS(util)::Compare::TChar(index->lookup[index->order[i->doc]]);
}

int32_t ScoreDocComparators::String::sortType() {
	return SortField::STRING;
}


ScoreDocComparators::Int32::Int32(int32_t* fieldOrder, int32_t len)
{
	this->fieldOrder = fieldOrder;
	this->length = len;
}


int32_t ScoreDocComparators::Int32::compare (struct ScoreDoc* i, struct ScoreDoc* j) {
	CND_PRECONDITION(i->doc<length, "i->doc>=length")
	CND_PRECONDITION(j->doc<length, "j->doc>=length")
	if (fieldOrder[i->doc] < fieldOrder[j->doc]) return -1;
	if (fieldOrder[i->doc] > fieldOrder[j->doc]) return 1;
	return 0;
}

CL_NS(util)::Comparable* ScoreDocComparators::Int32::sortValue (struct ScoreDoc* i) {
	CND_PRECONDITION(i->doc<length, "i->doc>=length")
	return _CLNEW CL_NS(util)::Compare::Int32(fieldOrder[i->doc]);
}

int32_t ScoreDocComparators::Int32::sortType() {
	return SortField::INT;
}

ScoreDocComparators::Float::Float(float_t* fieldOrder, int32_t len)
{
	this->fieldOrder = fieldOrder;
	this->length = len;
}

int32_t ScoreDocComparators::Float::compare (struct ScoreDoc* i, struct ScoreDoc* j) {
	CND_PRECONDITION(i->doc<length, "i->doc>=length")
	CND_PRECONDITION(j->doc<length, "j->doc>=length")
	if (fieldOrder[i->doc] < fieldOrder[j->doc]) return -1;
	if (fieldOrder[i->doc] > fieldOrder[j->doc]) return 1;
	return 0;
}

CL_NS(util)::Comparable* ScoreDocComparators::Float::sortValue (struct ScoreDoc* i) {
	CND_PRECONDITION(i->doc<length, "i->doc>=length")
	return _CLNEW CL_NS(util)::Compare::Float(fieldOrder[i->doc]);
}

int32_t ScoreDocComparators::Float::sortType() {
	return SortField::FLOAT;
}

CL_NS_END
