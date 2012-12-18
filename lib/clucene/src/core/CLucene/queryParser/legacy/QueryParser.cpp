/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "QueryParser.h"

#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/search/SearchHeader.h"
#include "CLucene/search/BooleanClause.h"
#include "CLucene/search/Query.h"
#include "CLucene/index/Term.h"
#include "QueryToken.h"

#include "_TokenList.h"
#include "_Lexer.h"

CL_NS_USE(util)
CL_NS_USE(index)
CL_NS_USE(analysis)
CL_NS_USE(search)

CL_NS_DEF2(queryParser,legacy)

    QueryParser::QueryParser(const TCHAR* _field, Analyzer* _analyzer) : QueryParserBase(_analyzer){
    //Func - Constructor.
	//       Instantiates a QueryParser for the named field _field
	//Pre  - _field != NULL
	//Post - An instance has been created

		if ( _field )
			field = STRDUP_TtoT(_field);
		else
			field = NULL;
		tokens = NULL;
		lowercaseExpandedTerms = true;
	}

	QueryParser::~QueryParser() {
	//Func - Destructor
	//Pre  - true
	//Post - The instance has been destroyed

        _CLDELETE_CARRAY(field);
	}

    //static
    Query* QueryParser::parse(const TCHAR* query, const TCHAR* field, Analyzer* analyzer){
    //Func - Returns a new instance of the Query class with a specified query, field and
    //       analyzer values.
    //Pre  - query != NULL and holds the query to parse
	//       field != NULL and holds the default field for query terms
	//       analyzer holds a valid reference to an Analyzer and is used to
	//       find terms in the query text
	//Post - query has been parsed and an instance of Query has been returned

		CND_PRECONDITION(query != NULL, "query is NULL");
        CND_PRECONDITION(field != NULL, "field is NULL");

		QueryParser parser(field, analyzer);
		return parser.parse(query);
	}

    Query* QueryParser::parse(const TCHAR* query){
	//Func - Returns a parsed Query instance
	//Pre  - query != NULL and contains the query value to be parsed
	//Post - Returns a parsed Query Instance

        CND_PRECONDITION(query != NULL, "query is NULL");

		//Instantie a Stringer that can read the query string
        BufferedReader* r = _CLNEW StringReader(query);

		//Check to see if r has been created properly
		CND_CONDITION(r != NULL, "Could not allocate memory for StringReader r");

		//Pointer for the return value
		Query* ret = NULL;

		try{
			//Parse the query managed by the StringReader R and return a parsed Query instance
			//into ret
			ret = parse(r);
		}_CLFINALLY (
			_CLDELETE(r);
		);

		return ret;
	}

	Query* QueryParser::parse(BufferedReader* reader){
	//Func - Returns a parsed Query instance
	//Pre  - reader contains a valid reference to a Reader and manages the query string
	//Post - A parsed Query instance has been returned or

		//instantiate the TokenList tokens
		TokenList _tokens;
		this->tokens = &_tokens;

		//Instantiate a lexer
		Lexer lexer(this, reader);

		//tokens = lexer.Lex();
		//Lex the tokens
		lexer.Lex(tokens);

		//Peek to the first token and check if is an EOF
		if (tokens->peek()->Type == QueryToken::EOF_){
			// The query string failed to yield any tokens.  We discard the
			// TokenList tokens and raise an exceptioin.
			QueryToken* token = this->tokens->extract();
			_CLDELETE(token);
		   _CLTHROWA(CL_ERR_Parse, "No query given.");
		}

		//Return the parsed Query instance
		Query* ret = MatchQuery(field);
		this->tokens = NULL;
		return ret;
	}

	int32_t QueryParser::MatchConjunction(){
	//Func - matches for CONJUNCTION
	//       CONJUNCTION ::= <AND> | <OR>
	//Pre  - tokens != NULL
	//Post - if the first token is an AND or an OR then
	//       the token is extracted and deleted and CONJ_AND or CONJ_OR is returned
	//       otherwise CONJ_NONE is returned

        CND_PRECONDITION(tokens != NULL, "tokens is NULL");

		switch(tokens->peek()->Type){
			case QueryToken::AND_ :
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return CONJ_AND;
			case QueryToken::OR   :
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return CONJ_OR;
			default :
				return CONJ_NONE;
		}
	}

	int32_t QueryParser::MatchModifier(){
	//Func - matches for MODIFIER
	//       MODIFIER ::= <PLUS> | <MINUS> | <NOT>
	//Pre  - tokens != NULL
	//Post - if the first token is a PLUS the token is extracted and deleted and MOD_REQ is returned
	//       if the first token is a MINUS or NOT the token is extracted and deleted and MOD_NOT is returned
	//       otherwise MOD_NONE is returned
		CND_PRECONDITION(tokens != NULL, "tokens is NULL");

		switch(tokens->peek()->Type){
			case QueryToken::PLUS :
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return MOD_REQ;
			case QueryToken::MINUS :
			case QueryToken::NOT   :
				//Delete the first token of tokenlist
				ExtractAndDeleteToken();
				return MOD_NOT;
			default :
				return MOD_NONE;
		}
	}

	Query* QueryParser::MatchQuery(const TCHAR* field){
	//Func - matches for QUERY
	//       QUERY ::= [MODIFIER] QueryParser::CLAUSE (<CONJUNCTION> [MODIFIER] CLAUSE)*
	//Pre  - field != NULL
	//Post -

		CND_PRECONDITION(tokens != NULL, "tokens is NULL");

		vector<BooleanClause*> clauses;

		Query* q = NULL;

		int32_t mods = MOD_NONE;
		int32_t conj = CONJ_NONE;

		//match for MODIFIER
		mods = MatchModifier();

		//match for CLAUSE
		q = MatchClause(field);
		AddClause(clauses, CONJ_NONE, mods, q);

		// match for CLAUSE*
		while(true){
			QueryToken* p = tokens->peek();
			if(p->Type == QueryToken::EOF_){
				QueryToken* qt = MatchQueryToken(QueryToken::EOF_);
				_CLDELETE(qt);
				break;
			}

			if(p->Type == QueryToken::RPAREN){
				//MatchQueryToken(QueryToken::RPAREN);
				break;
			}

			//match for a conjuction (AND OR NOT)
			conj = MatchConjunction();
			//match for a modifier
			mods = MatchModifier();

			q = MatchClause(field);
			if ( q != NULL )
				AddClause(clauses, conj, mods, q);
		}

		// finalize query
		if(clauses.size() == 1){ //bvk: removed this && firstQuery != NULL
			BooleanClause* c = clauses[0];
			Query* q = c->getQuery();

			//Condition check to be sure clauses[0] is valid
			CND_CONDITION(c != NULL, "c is NULL");

			//Tell the boolean clause not to delete its query
			c->deleteQuery=false;
			//Clear the clauses list
			clauses.clear();
			_CLDELETE(c);

			return q;
		}else{
			return GetBooleanQuery(clauses);
		}
	}

	Query* QueryParser::MatchClause(const TCHAR* field){
	//Func - matches for CLAUSE
	//       CLAUSE ::= [TERM <COLONQueryParser::>] ( TERM | (<LPAREN> QUERY <RPAREN>))
	//Pre  - field != NULL
	//Post -

		Query* q = NULL;
		const TCHAR* sfield = field;
		TCHAR* tmp = NULL;

		QueryToken *DelToken = NULL;

		//match for [TERM <COLON>]
		QueryToken* term = tokens->extract();
		if(term->Type == QueryToken::TERM && tokens->peek()->Type == QueryToken::COLON){
			DelToken = MatchQueryToken(QueryToken::COLON);

			CND_CONDITION(DelToken != NULL,"DelToken is NULL");
			_CLDELETE(DelToken);

			tmp = STRDUP_TtoT(term->Value);
			discardEscapeChar(tmp);
			sfield = tmp;
			_CLDELETE(term);
		}else{
			tokens->push(term);
			term = NULL;
		}

		// match for
		// TERM | (<LPAREN> QUERY <RPAREN>)
		if(tokens->peek()->Type == QueryToken::LPAREN){
			DelToken = MatchQueryToken(QueryToken::LPAREN);

			CND_CONDITION(DelToken != NULL,"DelToken is NULL");
			_CLDELETE(DelToken);

			q = MatchQuery(sfield);
			//DSR:2004.11.01:
			//If exception is thrown while trying to match trailing parenthesis,
			//need to prevent q from leaking.

			try{
			   DelToken = MatchQueryToken(QueryToken::RPAREN);

			   CND_CONDITION(DelToken != NULL,"DelToken is NULL");
			   _CLDELETE(DelToken);

			}catch(...) {
				_CLDELETE(q);
				throw;
			}
		}else{
			q = MatchTerm(sfield);
		}

	  _CLDELETE_CARRAY(tmp);
	  return q;
	}


	Query* QueryParser::MatchTerm(const TCHAR* field){
	//Func - matches for TERM
	//       TERM ::= TERM | PREFIXTERM | WILDTERM | NUMBER
	//                [ <FUZZY> ] [ <CARAT> <NUMBER> [<FUZZY>]]
	//			      | (<RANGEIN> | <RANGEEX>) [<CARAT> <NUMBER>]
	//			      | <QUOTED> [SLOP] [<CARAT> <NUMBER>]
	//Pre  - field != NULL
	//Post -

		QueryToken* term = NULL;
		QueryToken* slop = NULL;
		QueryToken* boost = NULL;

		bool prefix = false;
		bool wildcard = false;
		bool fuzzy = false;
		bool rangein = false;
		Query* q = NULL;

		term = tokens->extract();
		QueryToken* DelToken = NULL; //Token that is about to be deleted

		switch(term->Type){
			case QueryToken::TERM:
			case QueryToken::NUMBER:
			case QueryToken::PREFIXTERM:
			case QueryToken::WILDTERM:
			{ //start case
				//Check if type of QueryToken term is a prefix term
				if(term->Type == QueryToken::PREFIXTERM){
					prefix = true;
				}
				//Check if type of QueryToken term is a wildcard term
				if(term->Type == QueryToken::WILDTERM){
					wildcard = true;
				}
				//Peek to see if the type of the next token is fuzzy term
				if(tokens->peek()->Type == QueryToken::FUZZY){
					DelToken = MatchQueryToken(QueryToken::FUZZY);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL");
					_CLDELETE(DelToken);

					fuzzy = true;
				}
				if(tokens->peek()->Type == QueryToken::CARAT){
					DelToken = MatchQueryToken(QueryToken::CARAT);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL");
					_CLDELETE(DelToken);

					boost = MatchQueryToken(QueryToken::NUMBER);

					if(tokens->peek()->Type == QueryToken::FUZZY){
					   DelToken = MatchQueryToken(QueryToken::FUZZY);

					   CND_CONDITION(DelToken !=NULL, "DelToken is NULL");
					   _CLDELETE(DelToken);

					   fuzzy = true;
				   }
				} //end if type==CARAT

				discardEscapeChar(term->Value); //clean up
				if(wildcard){
					q = GetWildcardQuery(field,term->Value);
					break;
				}else if(prefix){
					//Create a PrefixQuery
					term->Value[_tcslen(term->Value)-1] = 0; //discard the *
					q = GetPrefixQuery(field,term->Value);
					break;
				}else if(fuzzy){
					//Create a FuzzyQuery

					//Check if the last char is a ~
					if(term->Value[_tcslen(term->Value)-1] == '~'){
						//remove the ~
						term->Value[_tcslen(term->Value)-1] = '\0';
					}

					q = GetFuzzyQuery(field,term->Value);
					break;
				}else{
					q = GetFieldQuery(field, term->Value);
					break;
				}
			}


			case QueryToken::RANGEIN:
			case QueryToken::RANGEEX:{
				if(term->Type == QueryToken::RANGEIN){
					rangein = true;
				}

				if(tokens->peek()->Type == QueryToken::CARAT){
					DelToken = MatchQueryToken(QueryToken::CARAT);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL");
					_CLDELETE(DelToken);

					boost = MatchQueryToken(QueryToken::NUMBER);
				}

				TCHAR* noBrackets = term->Value + 1;
				noBrackets[_tcslen(noBrackets)-1] = 0;
				q = ParseRangeQuery(field, noBrackets, rangein);
				break;
			}


			case QueryToken::QUOTED:{
				if(tokens->peek()->Type == QueryToken::SLOP){
					slop = MatchQueryToken(QueryToken::SLOP);
				}

				if(tokens->peek()->Type == QueryToken::CARAT){
					DelToken = MatchQueryToken(QueryToken::CARAT);

					CND_CONDITION(DelToken !=NULL, "DelToken is NULL");
					_CLDELETE(DelToken);

					boost = MatchQueryToken(QueryToken::NUMBER);
				}

				//remove the quotes
				TCHAR* quotedValue = term->Value+1;
				quotedValue[_tcslen(quotedValue)-1] = '\0';

				int32_t islop = phraseSlop;
				if(slop != NULL ){
				   try {
             islop = _ttoi(slop->Value+1);
				   }catch(...){
					   //ignored
				   }
				}

				q = GetFieldQuery(field, quotedValue, islop);
   				_CLDELETE(slop);
			}
			
			default:
			  break;
		} // end of switch

		_CLDELETE(term);


		if( q!=NULL && boost != NULL ){
			float_t f = 1.0F;
			try {
				f = _tcstod(boost->Value, NULL);
			}catch(...){
				//ignored
			}
			_CLDELETE(boost);

			q->setBoost( f);
		}

		return q;
	}

	QueryToken* QueryParser::MatchQueryToken(QueryToken::Types expectedType){
	//Func - matches for QueryToken of the specified type and returns it
	//       otherwise Exception throws
	//Pre  - tokens != NULL
	//Post -

		CND_PRECONDITION(tokens != NULL,"tokens is NULL");

		if(tokens->count() == 0){
			throwParserException(_T("Error: Unexpected end of program"),' ',0,0);
		}

	  //Extract a token form the TokenList tokens
	  QueryToken* t = tokens->extract();
	  //Check if the type of the token t matches the expectedType
	  if (expectedType != t->Type){
		  TCHAR buf[200];
		  _sntprintf(buf,200,_T("Error: Unexpected QueryToken: %d, expected: %d"),t->Type,expectedType);
		  _CLDELETE(t);
		  throwParserException(buf,' ',0,0);
		}

	  //Return the matched token
	  return t;
	}

	void QueryParser::ExtractAndDeleteToken(void){
	//Func - Extracts the first token from the Tokenlist tokenlist
	//       and destroys it
	//Pre  - true
	//Post - The first token has been extracted and destroyed

		CND_PRECONDITION(tokens != NULL, "tokens is NULL");

		//Extract the token from the TokenList tokens
		QueryToken* t = tokens->extract();
		//Condition Check Token may not be NULL
		CND_CONDITION(t != NULL, "Token is NULL");
		//Delete Token
		_CLDELETE(t);
	}

CL_NS_END2
