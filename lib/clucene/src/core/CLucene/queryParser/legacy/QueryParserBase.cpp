/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "QueryParser.h"

#include "CLucene/search/TermQuery.h"
#include "CLucene/search/PhraseQuery.h"
#include "CLucene/search/RangeQuery.h"
#include "CLucene/search/FuzzyQuery.h"
#include "CLucene/search/WildcardQuery.h"
#include "CLucene/search/PrefixQuery.h"
#include "CLucene/search/BooleanQuery.h"

#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/search/BooleanClause.h"
#include "CLucene/search/Query.h"
#include "CLucene/index/Term.h"
#include "QueryToken.h"

#include "_TokenList.h"
#include "_Lexer.h"

CL_NS_USE(search)
CL_NS_USE(util)
CL_NS_USE(analysis)
CL_NS_USE(index)

CL_NS_DEF2(queryParser,legacy)

QueryParserBase::QueryParserBase(Analyzer* analyzer){
//Func - Constructor
//Pre  - true
//Post - instance has been created with PhraseSlop = 0
  this->analyzer = analyzer;
  this->defaultOperator = OR_OPERATOR;
  this->phraseSlop = 0;
  this->lowercaseExpandedTerms = true;
}

QueryParserBase::~QueryParserBase(){
//Func - Destructor
//Pre  - true
//Post - The instance has been destroyed
}


void QueryParserBase::discardEscapeChar(TCHAR* source) const{
  int len = _tcslen(source);
  for (int i = 0; i < len; i++) {
    if (source[i] == '\\' && source[i+1] != '\0' ) {
      _tcscpy(source+i,source+i+1);
      len--;
    }
  }
}

void QueryParserBase::AddClause(std::vector<BooleanClause*>& clauses, int32_t conj, int32_t mods, Query* q){
//Func - Adds the next parsed clause.
//Pre  -
//Post -

  bool required, prohibited;

  // If this term is introduced by AND, make the preceding term required,
  // unless it's already prohibited.
  const uint32_t nPreviousClauses = clauses.size();
  if (nPreviousClauses > 0 && conj == CONJ_AND) {
      BooleanClause* c = clauses[nPreviousClauses-1];
      if (!c->prohibited)
    c->required = true;
  }

  if (nPreviousClauses > 0 && defaultOperator == AND_OPERATOR && conj == CONJ_OR) {
      // If this term is introduced by OR, make the preceding term optional,
      // unless it's prohibited (that means we leave -a OR b but +a OR b-->a OR b)
      // notice if the input is a OR b, first term is parsed as required; without
      // this modification a OR b would parse as +a OR b
      BooleanClause* c = clauses[nPreviousClauses-1];
    if (!c->prohibited){
    c->required = false;
    c->prohibited = false;
    }
  }

  // We might have been passed a NULL query; the term might have been
  // filtered away by the analyzer.
  if (q == NULL)
  return;

  if (defaultOperator == OR_OPERATOR) {
      // We set REQUIRED if we're introduced by AND or +; PROHIBITED if
      // introduced by NOT or -; make sure not to set both.
      prohibited = (mods == MOD_NOT);
      required = (mods == MOD_REQ);
      if (conj == CONJ_AND && !prohibited) {
    required = true;
      }
  } else {
      // We set PROHIBITED if we're introduced by NOT or -; We set REQUIRED
      // if not PROHIBITED and not introduced by OR
      prohibited = (mods == MOD_NOT);
      required = (!prohibited && conj != CONJ_OR);
  }

  if ( required && prohibited )
    throwParserException( _T("Clause cannot be both required and prohibited"), ' ',0,0);
  clauses.push_back(_CLNEW BooleanClause(q,true, required, prohibited));
}

void QueryParserBase::throwParserException(const TCHAR* message, TCHAR ch, int32_t col, int32_t line )
{
  TCHAR msg[1024];
  _sntprintf(msg,1024,message,ch,col,line);
  _CLTHROWT (CL_ERR_Parse, msg );
}


Query* QueryParserBase::GetFieldQuery(const TCHAR* field, TCHAR* queryText, int32_t slop){
  Query* ret = GetFieldQuery(field,queryText);
  if ( ret && ret->instanceOf(PhraseQuery::getClassName()) )
    ((PhraseQuery*)ret)->setSlop(slop);

  return ret;
}

Query* QueryParserBase::GetFieldQuery(const TCHAR* field, TCHAR* queryText){
//Func - Returns a query for the specified field.
//       Use the analyzer to get all the tokens, and then build a TermQuery,
//       PhraseQuery, or nothing based on the term count
//Pre  - field != NULL
//       analyzer contains a valid reference to an Analyzer
//       queryText != NULL and contains the query
//Post - A query instance has been returned for the specified field

  CND_PRECONDITION(field != NULL, "field is NULL");
  CND_PRECONDITION(queryText != NULL, "queryText is NULL");

  //Instantiate a stringReader for queryText
  StringReader reader(queryText);
  TokenStream* source = analyzer->tokenStream(field, &reader);
  CND_CONDITION(source != NULL,"source is NULL");

  StringArrayWithDeletor v;

  Token t;
  int positionCount = 0;
  bool severalTokensAtSamePosition = false;

  //Get the tokens from the source
  try{
    while (source->next(&t)){
      v.push_back(STRDUP_TtoT(t.termBuffer()));

      if (t.getPositionIncrement() != 0)
        positionCount += t.getPositionIncrement();
      else
        severalTokensAtSamePosition = true;
    }
  }catch(CLuceneError& err){
    if ( err.number() != CL_ERR_IO ) {
      _CLLDELETE(source);
      throw err;
    }
  }
  _CLDELETE(source);

  //Check if there are any tokens retrieved
  if (v.size() == 0){
    return NULL;
  }else{
    if (v.size() == 1){
      Term* t = _CLNEW Term(field, v[0]);
      Query* ret = _CLNEW TermQuery( t );
      _CLDECDELETE(t);
      return ret;
    }else{
      if (severalTokensAtSamePosition) {
        if (positionCount == 1) {
          // no phrase query:
          BooleanQuery* q = _CLNEW BooleanQuery( true ); //todo: disableCoord=true here, but not implemented in BooleanQuery
          StringArray::iterator itr = v.begin();
          while ( itr != v.end() ){
            Term* t = _CLNEW Term(field, *itr);
            q->add(_CLNEW TermQuery(t),true, false,false);//should occur...
            _CLDECDELETE(t);
            ++itr;
          }
          return q;
        }else {
          _CLTHROWA(CL_ERR_UnsupportedOperation, "MultiPhraseQuery NOT Implemented");
        }
      }else{
        PhraseQuery* q = _CLNEW PhraseQuery;
        q->setSlop(phraseSlop);

        StringArrayWithDeletor::iterator itr = v.begin();
        while ( itr != v.end() ){
          const TCHAR* data = *itr;
          Term* t = _CLNEW Term(field, data);
          q->add(t);
          _CLDECDELETE(t);
          ++itr;
        }
        return q;
      }
    }
  }
}

void QueryParserBase::setLowercaseExpandedTerms(bool lowercaseExpandedTerms){
  this->lowercaseExpandedTerms = lowercaseExpandedTerms;
}
bool QueryParserBase::getLowercaseExpandedTerms() const {
  return lowercaseExpandedTerms;
}
void QueryParserBase::setDefaultOperator(int oper){
  this->defaultOperator=oper;
}
int QueryParserBase::getDefaultOperator() const{
  return defaultOperator;
}


Query* QueryParserBase::ParseRangeQuery(const TCHAR* field, TCHAR* queryText, bool inclusive)
{
  //todo: this must be fixed, [-1--5] (-1 to -5) should yield a result, but won't parse properly
  //because it uses an analyser, should split it up differently...

  // Use the analyzer to get all the tokens.  There should be 1 or 2.
  StringReader reader(queryText);
  TokenStream* source = analyzer->tokenStream(field, &reader);

  TCHAR* terms[2];
  terms[0]=NULL;terms[1]=NULL;
  Token t;
  bool tret=false;
  bool from=true;
  do
  {
    try{
      tret = (source->next(&t) != NULL);
    }catch (CLuceneError& err){
      if ( err.number() == CL_ERR_IO )
        tret=false;
      else
        throw err;
    }
    if (tret)
    {
      if ( !from && _tcscmp(t.termBuffer(),_T("TO"))==0 )
        continue;


      TCHAR* tmp = STRDUP_TtoT(t.termBuffer());
      discardEscapeChar(tmp);
      terms[from? 0 : 1] = tmp;

      if (from)
        from = false;
      else
        break;
    }
  }while(tret);
  if ((terms[0] == NULL) || (terms[1] == NULL)) {
    _CLTHROWA(CL_ERR_Parse, "No range given.");
  }
  Query* ret = GetRangeQuery(field, terms[0], terms[1],inclusive);
  _CLDELETE_CARRAY(terms[0]);
  _CLDELETE_CARRAY(terms[1]);
  _CLDELETE(source);

  return ret;
}

Query* QueryParserBase::GetPrefixQuery(const TCHAR* field, TCHAR* termStr){
//Pre  - field != NULL and field contains the name of the field that the query will use
//       termStr != NULL and is the  token to use for building term for the query
//       (WITH or WITHOUT a trailing '*' character!)
//Post - A PrefixQuery instance has been returned

  CND_PRECONDITION(field != NULL,"field is NULL");
  CND_PRECONDITION(termStr != NULL,"termStr is NULL");

  if ( lowercaseExpandedTerms )
    _tcslwr(termStr);

  Term* t = _CLNEW Term(field, termStr);
  CND_CONDITION(t != NULL,"Could not allocate memory for term t");

  Query *q = _CLNEW PrefixQuery(t);
  CND_CONDITION(q != NULL,"Could not allocate memory for PrefixQuery q");

  _CLDECDELETE(t);
  return q;
}

Query* QueryParserBase::GetFuzzyQuery(const TCHAR* field, TCHAR* termStr){
//Func - Factory method for generating a query (similar to getPrefixQuery}). Called when parser parses
//       an input term token that has the fuzzy suffix (~) appended.
//Pre  - field != NULL and field contains the name of the field that the query will use
//       termStr != NULL and is the  token to use for building term for the query
//       (WITH or WITHOUT a trailing '*' character!)
//Post - A FuzzyQuery instance has been returned

  CND_PRECONDITION(field != NULL,"field is NULL");
  CND_PRECONDITION(termStr != NULL,"termStr is NULL");

  if ( lowercaseExpandedTerms )
    _tcslwr(termStr);

  Term* t = _CLNEW Term(field, termStr);
  CND_CONDITION(t != NULL,"Could not allocate memory for term t");

  Query *q = _CLNEW FuzzyQuery(t);
  CND_CONDITION(q != NULL,"Could not allocate memory for FuzzyQuery q");

  _CLDECDELETE(t);
  return q;
}


Query* QueryParserBase::GetWildcardQuery(const TCHAR* field, TCHAR* termStr){
  CND_PRECONDITION(field != NULL,"field is NULL");
  CND_PRECONDITION(termStr != NULL,"termStr is NULL");

  if ( lowercaseExpandedTerms )
    _tcslwr(termStr);

  Term* t = _CLNEW Term(field, termStr);
  CND_CONDITION(t != NULL,"Could not allocate memory for term t");
  Query* q = _CLNEW WildcardQuery(t);
  _CLDECDELETE(t);

  return q;
}

Query* QueryParserBase::GetBooleanQuery(std::vector<CL_NS(search)::BooleanClause*>& clauses ) {
  return GetBooleanQuery( clauses, false );
}

Query* QueryParserBase::GetBooleanQuery(std::vector<CL_NS(search)::BooleanClause*>& clauses, bool disableCoord){
  if ( clauses.size() == 0 )
    return NULL;

  BooleanQuery* query = _CLNEW BooleanQuery( disableCoord );
  //Condition check to see if query has been allocated properly
  CND_CONDITION(query != NULL, "No memory could be allocated for query");

  //iterate through all the clauses
  for( uint32_t i=0;i<clauses.size();i++ ){
    //Condition check to see if clauses[i] is valid
    CND_CONDITION(clauses[i] != NULL, "clauses[i] is NULL");
    //Add it to query
    query->add(clauses[i]);
  }
  return query;
}


CL_NS(search)::Query* QueryParserBase::GetRangeQuery(const TCHAR* field, TCHAR* part1, TCHAR* part2, bool inclusive){
  //todo: does jlucene handle rangequeries differntly? if we are using
  //a certain type of analyser, the terms may be filtered out, which
  //is not necessarily what we want.
  if (lowercaseExpandedTerms) {
    _tcslwr(part1);
    _tcslwr(part2);
  }
  //todo: should see if we can parse the strings as dates... currently we leave that up to the end-developer...
  Term* t1 = _CLNEW Term(field,part1);
  Term* t2 = _CLNEW Term(field,part2);
  Query* ret = _CLNEW RangeQuery(t1, t2, inclusive);
  _CLDECDELETE(t1);
  _CLDECDELETE(t2);

  return ret;
}

CL_NS_END2
