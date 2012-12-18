/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "CLucene/index/Term.h"
#include "CLucene/index/Terms.h"
#include "CLucene/index/IndexReader.h"
#include "Similarity.h"
#include "PrefixQuery.h"
#include "BooleanClause.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "CLucene/util/BitSet.h"
#include "CLucene/util/StringBuffer.h"

CL_NS_USE(util)
CL_NS_USE(index)
CL_NS_DEF(search)

  PrefixQuery::PrefixQuery(Term* Prefix){
  //Func - Constructor.
  //       Constructs a query for terms starting with prefix
  //Pre  - Prefix != NULL 
  //Post - The instance has been created

      //Get a pointer to Prefix
      prefix = _CL_POINTER(Prefix);
  }

  PrefixQuery::PrefixQuery(const PrefixQuery& clone):Query(clone){
	prefix = _CL_POINTER(clone.prefix);
  }
  Query* PrefixQuery::clone() const{
	  return _CLNEW PrefixQuery(*this);
  }

  Term* PrefixQuery::getPrefix(bool pointer){
	if ( pointer )
		return _CL_POINTER(prefix);
	else
		return prefix;
  }

  PrefixQuery::~PrefixQuery(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed.
    
      //Delete prefix by finalizing it
      _CLDECDELETE(prefix);
  }


	/** Returns a hash code value for this object.*/
	size_t PrefixQuery::hashCode() const {
		return Similarity::floatToByte(getBoost()) ^ prefix->hashCode();
	}

  const char* PrefixQuery::getObjectName()const{
  //Func - Returns the name "PrefixQuery" 
  //Pre  - true
  //Post - The string "PrefixQuery" has been returned

      return getClassName();
  }
  const char* PrefixQuery::getClassName(){
  //Func - Returns the name "PrefixQuery" 
  //Pre  - true
  //Post - The string "PrefixQuery" has been returned

      return "PrefixQuery";
  }

  bool PrefixQuery::equals(Query * other) const{
	  if (!(other->instanceOf(PrefixQuery::getClassName())))
            return false;

        PrefixQuery* rq = (PrefixQuery*)other;
		bool ret = (this->getBoost() == rq->getBoost())
			&& (this->prefix->equals(rq->prefix));

		return ret;
  }

   Query* PrefixQuery::rewrite(IndexReader* reader){
    BooleanQuery* query = _CLNEW BooleanQuery( true );
    TermEnum* enumerator = reader->terms(prefix);
    Term* lastTerm = NULL;
    try {
      const TCHAR* prefixText = prefix->text();
      const TCHAR* prefixField = prefix->field();
      const TCHAR* tmp;
      size_t i;
      size_t prefixLen = prefix->textLength();
      do {
        lastTerm = enumerator->term();
        if (lastTerm != NULL &&
          lastTerm->field() == prefixField ) // interned comparison
        {

          //now see if term->text() starts with prefixText
          size_t termLen = lastTerm->textLength();
          if ( prefixLen>termLen )
            break; //the prefix is longer than the term, can't be matched

          tmp = lastTerm->text();

          //check for prefix match in reverse, since most change will be at the end
          for ( i=prefixLen-1;i!=-1;--i ){
              if ( tmp[i] != prefixText[i] ){
                  tmp=NULL;//signals inequality
                  break;
              }
          }
          if ( tmp == NULL )
              break;

          TermQuery* tq = _CLNEW TermQuery(lastTerm);	  // found a match
          tq->setBoost(getBoost());                // set the boost
          query->add(tq,true,false, false);		  // add to query
        } else
          break;
		_CLDECDELETE(lastTerm);
      } while (enumerator->next());
    }_CLFINALLY(
      enumerator->close();
	  _CLDELETE(enumerator);
	  _CLDECDELETE(lastTerm);
	);
	_CLDECDELETE(lastTerm);


	//if we only added one clause and the clause is not prohibited then
	//we can just return the query
	if (query->getClauseCount() == 1) {                    // optimize 1-clause queries
		BooleanClause* c=0;
		query->getClauses(&c);

		if (!c->prohibited) {			  // just return clause
			c->deleteQuery=false;
			Query* ret = c->getQuery();

			_CLDELETE(query);
			return ret;
        }
	}

    return query;
  }

  Query* PrefixQuery::combine(CL_NS(util)::ArrayBase<Query*>* queries) {
	  return Query::mergeBooleanQueries(queries);
  }

  TCHAR* PrefixQuery::toString(const TCHAR* field) const{
  //Func - Creates a user-readable version of this query and returns it as as string
  //Pre  - field != NULL
  //Post - a user-readable version of this query has been returned as as string

    //Instantiate a stringbuffer buffer to store the readable version temporarily
    CL_NS(util)::StringBuffer buffer;
    //check if field equal to the field of prefix
    if( field==NULL ||
        _tcscmp(prefix->field(),field) != 0 ) {
        //Append the field of prefix to the buffer
        buffer.append(prefix->field());
        //Append a colon
        buffer.append(_T(":") );
    }
    //Append the text of the prefix
    buffer.append(prefix->text());
	//Append a wildchar character
    buffer.append(_T("*"));
	//if the boost factor is not eaqual to 1
    if (getBoost() != 1.0f) {
		//Append ^
        buffer.append(_T("^"));
		//Append the boost factor
        buffer.appendFloat( getBoost(),1);
    }
	//Convert StringBuffer buffer to TCHAR block and return it
    return buffer.toString();
  }





//todo: this needs to be exposed, but java is still a bit confused about how...
class PrefixFilter::PrefixGenerator{
  const Term* prefix;
public:
  PrefixGenerator(const Term* prefix){
    this->prefix = prefix;
  }
  virtual ~PrefixGenerator(){
  }

  virtual void handleDoc(int doc) = 0;

  void generate(IndexReader* reader) {
    TermEnum* enumerator = reader->terms(prefix);
    TermDocs* termDocs = reader->termDocs();
    const TCHAR* prefixText = prefix->text();
    const TCHAR* prefixField = prefix->field();
    const TCHAR* tmp;
    size_t i;
    size_t prefixLen = prefix->textLength();
    Term* term = NULL;

    try{
      do{
          term = enumerator->term(false);
          if (term != NULL &&
              term->field() == prefixField // interned comparison
          ){
              //now see if term->text() starts with prefixText
              size_t termLen = term->textLength();
              if ( prefixLen>termLen )
                  break; //the prefix is longer than the term, can't be matched

              tmp = term->text();

              //check for prefix match in reverse, since most change will be at the end
              for ( i=prefixLen-1;i!=-1;--i ){
                  if ( tmp[i] != prefixText[i] ){
                      tmp=NULL;//signals inequality
                      break;
                  }
              }
              if ( tmp == NULL )
                  break;

            termDocs->seek(enumerator);
            while (termDocs->next()) {
              handleDoc(termDocs->doc());
            }
          }
      }while(enumerator->next());
    } _CLFINALLY(
        termDocs->close();
        _CLDELETE(termDocs);
        enumerator->close();
      _CLDELETE(enumerator);
    )
  }
};

class DefaultPrefixGenerator: public PrefixFilter::PrefixGenerator{
public:
  BitSet* bts;
  DefaultPrefixGenerator(BitSet* bts, const Term* prefix):
    PrefixGenerator(prefix)
  {
    this->bts = bts;
  }
  virtual ~DefaultPrefixGenerator(){
  }
  void handleDoc(int doc) {
    bts->set(doc);
  }
};

PrefixFilter::PrefixFilter( Term* prefix )
{
	this->prefix = _CL_POINTER(prefix);
}

PrefixFilter::~PrefixFilter()
{
	_CLDECDELETE(prefix);
}

PrefixFilter::PrefixFilter( const PrefixFilter& copy ) : 
	Filter(),
	prefix( _CL_POINTER(copy.prefix) )
{
}

Filter* PrefixFilter::clone() const {
	return _CLNEW PrefixFilter(*this );
}

TCHAR* PrefixFilter::toString()
{
	//Instantiate a stringbuffer buffer to store the readable version temporarily
    CL_NS(util)::StringBuffer buffer;
    buffer.append(_T("PrefixFilter("));
    buffer.append(prefix->field());
    buffer.append(_T(")"));

	//Convert StringBuffer buffer to TCHAR block and return it
    return buffer.toString();
}

/** Returns a BitSet with true for documents which should be permitted in
search results, and false for those that should not. */
BitSet* PrefixFilter::bits( IndexReader* reader )
{
	BitSet* bts = _CLNEW BitSet( reader->maxDoc() );
  DefaultPrefixGenerator gen(bts, prefix);
  gen.generate(reader);
	return bts;
}

CL_NS(index)::Term* PrefixFilter::getPrefix() const { return prefix; }

CL_NS_END
