/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_TokenList.h"

//#include "CLucene/util/VoidMap.h"
#include "CLucene/util/VoidList.h"
#include "QueryToken.h"

CL_NS_DEF2(queryParser,legacy)
  
    TokenList::TokenList(){
    //Func - Constructor
    //Pre  - true
    //Post - Instance has been created
    }

    TokenList::~TokenList(){
    //Func - Destructor
	//Pre  - true
	//Post - The tokenlist has been destroyed

        tokens.clear();
    }

	void TokenList::add(QueryToken* token){
    //Func - Adds a QueryToken token to the TokenList
	//Pre  - token != NULL
	//Post - token has been added to the token list

       CND_PRECONDITION(token != NULL, "token != NULL");

       tokens.insert(tokens.begin(),token);
    }

    void TokenList::push(QueryToken* token){
    //Func - 
	//Pre  - token != NULL
	//Post - 

      CND_PRECONDITION(token != NULL, "token is NULL");

      tokens.push_back(token);
    }

    QueryToken* TokenList::peek() {
      /* DSR:2004.11.01: Reverted my previous (circa April 2004) fix (which
      ** raised an exception if Peek was called when there were no tokens) in
      ** favor of returning the EOF token.  This solution is much better
      ** integrated with the rest of the code in the queryParser subsystem. */
      size_t nTokens = tokens.size();
      if (nTokens == 0) {
		push(_CLNEW QueryToken(QueryToken::EOF_));
        nTokens++;
      }
      return tokens[nTokens-1];
    }

    QueryToken* TokenList::extract(){
    //Func - Extract token from the TokenList
	//Pre  - true
	//Post - Retracted token has been returned

        QueryToken* token = peek();
        //Retract the current peeked token
        tokens.delete_back();

        return token;
    }

    int32_t TokenList::count() const
    {
      return tokens.size();
    }
CL_NS_END2
