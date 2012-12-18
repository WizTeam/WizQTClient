/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "WildcardQuery.h"
#include "TermQuery.h"
#include "WildcardTermEnum.h"
#include "Similarity.h"
#include "CLucene/index/Term.h"
#include "CLucene/util/BitSet.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/index/IndexReader.h"

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_DEF(search)


WildcardQuery::WildcardQuery(Term* term): 
MultiTermQuery( term ){
	//Func - Constructor
	//Pre  - term != NULL
	//Post - Instance has been created
	termContainsWildcard = (_tcschr(term->text(), _T('*')) != NULL || _tcschr(term->text(), _T('?')) != NULL);
}

WildcardQuery::~WildcardQuery(){
	//Func - Destructor
	//Pre  - true
	//Post - true

}

const char* WildcardQuery::getObjectName() const{
	//Func - Returns the string "WildcardQuery"
	//Pre  - true
	//Post - The string "WildcardQuery" has been returned
	return getClassName();
}

const char* WildcardQuery::getClassName(){
	return "WildcardQuery";
}


FilteredTermEnum* WildcardQuery::getEnum(IndexReader* reader) {
	return _CLNEW WildcardTermEnum(reader, getTerm(false));
}

WildcardQuery::WildcardQuery(const WildcardQuery& clone):
MultiTermQuery(clone)
{
}

Query* WildcardQuery::clone() const{
	return _CLNEW WildcardQuery(*this);
}
size_t WildcardQuery::hashCode() const{
	//todo: we should give the query a seeding value... but
	//need to do it for all hascode functions
	return Similarity::floatToByte(getBoost()) ^ getTerm()->hashCode();
}
bool WildcardQuery::equals(Query* other) const{
	if (!(other->instanceOf(WildcardQuery::getClassName())))
		return false;

	WildcardQuery* tq = (WildcardQuery*)other;
	return (this->getBoost() == tq->getBoost())
		&& getTerm()->equals(tq->getTerm());
}


Query* WildcardQuery::rewrite(CL_NS(index)::IndexReader* reader) {
	if (termContainsWildcard)
		return MultiTermQuery::rewrite(reader);

	return _CLNEW TermQuery( getTerm(false) );
}


WildcardFilter::WildcardFilter( Term* term )
{
	this->term = _CL_POINTER(term);
}

WildcardFilter::~WildcardFilter()
{
	_CLDECDELETE(term);
}

WildcardFilter::WildcardFilter( const WildcardFilter& copy ) : 
term( _CL_POINTER(copy.term) )
{
}

Filter* WildcardFilter::clone() const {
	return _CLNEW WildcardFilter(*this );
}


TCHAR* WildcardFilter::toString()
{
	//Instantiate a stringbuffer buffer to store the readable version temporarily
	CL_NS(util)::StringBuffer buffer;
	//check if field equal to the field of prefix
	if( term->field() != NULL ) {
		//Append the field of prefix to the buffer
		buffer.append(term->field());
		//Append a colon
		buffer.append(_T(":") );
	}
	//Append the text of the prefix
	buffer.append(term->text());

	//Convert StringBuffer buffer to TCHAR block and return it
	return buffer.toString();
}


/** Returns a BitSet with true for documents which should be permitted in
search results, and false for those that should not. */
BitSet* WildcardFilter::bits( IndexReader* reader )
{
	BitSet* bts = _CLNEW BitSet( reader->maxDoc() );

	WildcardTermEnum termEnum (reader, term);
	if (termEnum.term(false) == NULL)
		return bts;

	TermDocs* termDocs = reader->termDocs();
	try{
		do{
			termDocs->seek(&termEnum);

			while (termDocs->next()) {
				bts->set(termDocs->doc());
			}
		}while(termEnum.next());
	} _CLFINALLY(
		termDocs->close();
	_CLDELETE(termDocs);
	termEnum.close();
	)

		return bts;
}

CL_NS_END
