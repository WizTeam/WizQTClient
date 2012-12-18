/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Explanation.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_USE(util)
CL_NS_DEF(search)


Explanation::Explanation(float_t _value, const TCHAR* _description):value(_value),details(NULL)
{
	_tcsncpy(this->description,_description,LUCENE_SEARCH_EXPLANATION_DESC_LEN);
}

Explanation::Explanation():value(0), details(NULL)
{
	this->description[0]=0;
}

Explanation::Explanation(const Explanation& copy):details(NULL)
{
    set(copy);
}
void Explanation::set(const Explanation& copy){
    this->value = copy.value;
    _tcsncpy(description,copy.description,LUCENE_SEARCH_EXPLANATION_DESC_LEN);

    _CLDELETE(this->details);

	if (copy.details != NULL) {
		this->details = _CLNEW CL_NS(util)::CLArrayList<Explanation*,CL_NS(util)::Deletor::Object<Explanation> >(true);
		CL_NS(util)::CLArrayList<Explanation*,CL_NS(util)::Deletor::Object<Explanation> >::const_iterator itr;
		itr = copy.details->begin();
		while ( itr != copy.details->end() ){
			details->push_back( (*itr)->clone() );
			++itr;
		}
	}
}
Explanation* Explanation::clone() const{
   return _CLNEW Explanation(*this);
}

Explanation::~Explanation(){
    _CLLDELETE(this->details);
}

bool Explanation::isMatch() const {
	return (0.0f < getValue());
}

float_t Explanation::getValue() const{
   return value;
}
void Explanation::setValue(const float_t value) {
   this->value = value;
}

const TCHAR* Explanation::getDescription() const {
   return description;
}
void Explanation::setDescription(const TCHAR* description) {
   _tcsncpy(this->description,description,LUCENE_SEARCH_EXPLANATION_DESC_LEN);
}

TCHAR* Explanation::getSummary() {
	StringBuffer buf(210);
	buf.appendFloat(getValue(), 2);
	buf.append(_T(" = "));
	buf.append(getDescription());
	return buf.giveBuffer();
}

size_t Explanation::getDetailsLength() const {return (details==NULL)?0:details->size();}
void Explanation::getDetails(Explanation** ret) {
	if (details==NULL){
		ret[0]=NULL;
		return;
	}
    size_t size = details->size();
    for ( size_t i=0;i<size;i++ ){
        ret[i] = (*details)[i]->clone();
    }
    ret[size] = NULL;
}
Explanation* Explanation::getDetail(const size_t i){return (*details)[i];}

void Explanation::addDetail(Explanation* detail) {
	if (details==NULL) details = _CLNEW CL_NS(util)::CLArrayList<Explanation*,CL_NS(util)::Deletor::Object<Explanation> >(true);
	details->push_back(detail);
}

TCHAR* Explanation::toString() {
	return toString(0);
}

TCHAR* Explanation::toString(int32_t depth) {
	StringBuffer buffer;
	for (int32_t i = 0; i < depth; i++) {
		buffer.append(_T("  "));
	}
	TCHAR* tSum = getSummary();
	buffer.append(tSum);
	_CLDELETE_LCARRAY(tSum);
	buffer.appendChar(_T('\n'));

	if (details != NULL){
		for ( size_t j=0;j<details->size();j++ ){
			TCHAR* tmp = (*details)[j]->toString(depth+1);
			buffer.append(tmp);
			_CLDELETE_LCARRAY(tmp);
		}
	}
	return buffer.toString();
}

TCHAR* Explanation::toHtml() {
	StringBuffer buffer;
	TCHAR* tmp;
	buffer.append(_T("<ul>\n"));

	buffer.append(_T("<li>"));
	TCHAR* tSum = getSummary();
	buffer.append(tSum);
	_CLDELETE_LCARRAY(tSum);
	buffer.append(_T("<br />\n"));

	if (details != NULL){
		for ( size_t i=0;i<details->size();i++ ){
			tmp = (*details)[i]->toHtml();
			buffer.append(tmp);
			_CLDELETE_LCARRAY(tmp);
		}
	}
	buffer.append(_T("</li>\n"));
	buffer.append(_T("</ul>\n"));

	return buffer.toString();
}

ComplexExplanation::ComplexExplanation():Explanation(){}
ComplexExplanation::ComplexExplanation(const ComplexExplanation& copy):
  Explanation(copy)
{
   this->match = copy.match;
}
ComplexExplanation::ComplexExplanation(const bool _match, const float_t _value, const TCHAR* _description):
	Explanation(_value, _description), match(_match)
{
}
ComplexExplanation::~ComplexExplanation(){
}

bool ComplexExplanation::getMatch() const { return match; }
void ComplexExplanation::setMatch(const bool _match) { this->match = _match; }
bool ComplexExplanation::isMatch() const {return getMatch();}

TCHAR* ComplexExplanation::getSummary() {
	StringBuffer buf(220);
	buf.appendFloat(getValue(),2);
	buf.append(_T(" = "));
	buf.append(isMatch() ? _T("(MATCH) ") : _T("(NON-MATCH) "));
	buf.append(getDescription());
	return buf.giveBuffer();
}

Explanation* ComplexExplanation::clone() const{
   return _CLNEW ComplexExplanation(*this);
}

CL_NS_END
