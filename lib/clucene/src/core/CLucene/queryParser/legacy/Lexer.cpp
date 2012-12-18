/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "QueryParser.h"
#include "_TokenList.h"
#include "QueryToken.h"
#include "_Lexer.h"

#include "CLucene/util/CLStreams.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/_FastCharStream.h"

CL_NS_USE(util)

CL_NS_DEF2(queryParser,legacy)
Lexer::Lexer(QueryParserBase* queryparser, const TCHAR* query) {
   //Func - Constructor
   //Pre  - query != NULL and contains the query string
   //Post - An instance of Lexer has been created

	this->queryparser = queryparser;

   CND_PRECONDITION(query != NULL, "query is NULL");

   //The InputStream of Reader must be destroyed in the destructor
   delSR = true;

   StringReader *r = _CLNEW StringReader(query);

   //Check to see if r has been created properly
   CND_CONDITION(r != NULL, "Could not allocate memory for StringReader r");

   //Instantie a FastCharStream instance using r and assign it to reader
   reader = _CLNEW FastCharStream(r);

   //Check to see if reader has been created properly
   CND_CONDITION(reader != NULL, "Could not allocate memory for FastCharStream reader");

   //The InputStream of Reader must be destroyed in the destructor
   delSR = true;

}


Lexer::Lexer(QueryParserBase* queryparser, BufferedReader* source) {
   //Func - Constructor
   //       Initializes a new instance of the Lexer class with the specified
   //       TextReader to lex.
   //Pre  - Source contains a valid reference to a Reader
   //Post - An instance of Lexer has been created using source as the reader

	this->queryparser = queryparser;

   //Instantie a FastCharStream instance using r and assign it to reader
   reader = _CLNEW FastCharStream(source);

   //Check to see if reader has been created properly
   CND_CONDITION(reader != NULL, "Could not allocate memory for FastCharStream reader");

   //The InputStream of Reader must not be destroyed in the destructor
   delSR  = false;
}


Lexer::~Lexer() {
   //Func - Destructor
   //Pre  - true
   //Post - if delSR was true the InputStream input of reader has been deleted
   //       The instance of Lexer has been destroyed

   if (delSR) {
      _CLDELETE(reader->input);
   }

   _CLDELETE(reader);
}


void Lexer::Lex(TokenList *tokenList) {
   //Func - Breaks the input stream onto the tokens list tokens
   //Pre  - tokens != NULL and contains a TokenList in which the tokens can be stored
   //Post - The tokens have been added to the TokenList tokens

   CND_PRECONDITION(tokenList != NULL, "tokens is NULL");

   //Get all the tokens
   while(true) {
      //Add the token to the tokens list
	  
	  //Get the next token
	  QueryToken* token = _CLNEW QueryToken;
	  if ( !GetNextToken(token) ){
		_CLDELETE(token);
		break;
	  }
      tokenList->add(token);
   }

   //The end has been reached so create an EOF_ token
   //Add the final token to the TokenList _tokens
   tokenList->add(_CLNEW QueryToken( QueryToken::EOF_));
}


bool Lexer::GetNextToken(QueryToken* token) {
   while(!reader->Eos()) {
      int ch = reader->GetNext();

	  if ( ch == -1 )
		break;

      // skipping whitespaces
      if( _istspace(ch)!=0 ) {
         continue;
      }
      TCHAR buf[2] = {ch,'\0'};
      switch(ch) {
         case '+':
            token->set(buf, QueryToken::PLUS);
            return true;
         case '-':
            token->set(buf, QueryToken::MINUS);
            return true;
         case '(':
            token->set(buf, QueryToken::LPAREN);
            return true;
         case ')':
            token->set(buf, QueryToken::RPAREN);
            return true;
         case ':':
            token->set(buf, QueryToken::COLON);
            return true;
         case '!':
            token->set(buf, QueryToken::NOT);
            return true;
         case '^':
            token->set(buf, QueryToken::CARAT);
            return true;
         case '~':
            if( _istdigit( reader->Peek() )!=0 ) {
				TCHAR number[LUCENE_MAX_FIELD_LEN];
                ReadIntegerNumber(ch, number,LUCENE_MAX_FIELD_LEN);
                token->set(number, QueryToken::SLOP);
                return true;
            }else{
                token->set(buf, QueryToken::FUZZY);
                return true;
            }
			break;
         case '"':
			 return ReadQuoted(ch, token);
         case '[':
            return ReadInclusiveRange(ch, token);
         case '{':
            return ReadExclusiveRange(ch, token);
         case ']':
         case '}':
         case '*':
            queryparser->throwParserException( _T("Unrecognized char %d at %d::%d."), 
               ch, reader->Column(), reader->Line() );
            return false;
         default:
            return ReadTerm(ch, token);

   // end of swith
      }

   }
   return false;
}


void Lexer::ReadIntegerNumber(const TCHAR ch, TCHAR* buf, int buflen) {
   int bp=0;
   buf[bp++] = ch;

   int c = reader->Peek();
   while( c!=-1 && _istdigit(c)!=0 && bp<buflen-1 ) {
      buf[bp++] = reader->GetNext();
      c = reader->Peek();
   }
   buf[bp++] = 0;
}


bool Lexer::ReadInclusiveRange(const TCHAR prev, QueryToken* token) {
   int ch = prev;
   StringBuffer range;
   range.appendChar(ch);

   while(!reader->Eos()) {
      ch = reader->GetNext();
	  if ( ch == -1 )
		break;
      range.appendChar(ch);

      if(ch == ']'){
         token->set(range.getBuffer(), QueryToken::RANGEIN);
         return true;
      }
   }
   queryparser->throwParserException(_T("Unterminated inclusive range! %d %d::%d"),' ',
      reader->Column(),reader->Column());
   return false;
}


bool Lexer::ReadExclusiveRange(const TCHAR prev, QueryToken* token) {
   int ch = prev;
   StringBuffer range;
   range.appendChar(ch);

   while(!reader->Eos()) {
      ch = reader->GetNext();

	  if (ch==-1)
		break;
	  range.appendChar(ch);

      if(ch == '}'){
         token->set(range.getBuffer(), QueryToken::RANGEEX);
        return true;
      }
   }
   queryparser->throwParserException(_T("Unterminated exclusive range! %d %d::%d"),' ',
      reader->Column(),reader->Column() );
   return false;
}

bool Lexer::ReadQuoted(const TCHAR prev, QueryToken* token) {
   int ch = prev;
   StringBuffer quoted;
   quoted.appendChar(ch);

   while(!reader->Eos()) {
      ch = reader->GetNext();

	  if (ch==-1)
		break;

      quoted.appendChar(ch);

      if(ch == '"'){
         token->set(quoted.getBuffer(), QueryToken::QUOTED);
         return true;
      }
   }
   queryparser->throwParserException(_T("Unterminated string! %d %d::%d"),' ',
      reader->Column(),reader->Column());
   return false;
}


bool Lexer::ReadTerm(const TCHAR prev, QueryToken* token) {
   int ch = prev;
   bool completed = false;
   int32_t asteriskCount = 0;
   bool hasQuestion = false;

   StringBuffer val;
   TCHAR buf[3]; //used for readescaped

   while(true) {
      switch(ch) {
		  case -1:
			  break;
         case '\\':
         {
            if ( ReadEscape(ch, buf) )
                val.append( buf );
			else
				return false;
         }
         break;

         case LUCENE_WILDCARDTERMENUM_WILDCARD_STRING:
            asteriskCount++;
            val.appendChar(ch);
            break;
         case LUCENE_WILDCARDTERMENUM_WILDCARD_CHAR:
            hasQuestion = true;
            val.appendChar(ch);
            break;
         case '\n':
         case '\t':
         case ' ':
         case '+':
         case '-':
         case '!':
         case '(':
         case ')':
         case ':':
         case '^':
         case '[':
         case ']':
         case '{':
         case '}':
         case '~':
         case '"':
            // create new QueryToken
            reader->UnGet();
            completed = true;
            break;
         default:
            val.appendChar(ch);
            break;
   // end of switch
      }

      if(completed || ch==-1 || reader->Eos() )
         break;
      else
         ch = reader->GetNext();
   }

   // create new QueryToken
   if(hasQuestion)
      token->set(val.getBuffer(), QueryToken::WILDTERM);
   else if(asteriskCount == 1 && val.getBuffer()[val.length() - 1] == '*')
      token->set(val.getBuffer(), QueryToken::PREFIXTERM);
   else if(asteriskCount > 0)
      token->set(val.getBuffer(), QueryToken::WILDTERM);
   else if( _tcsicmp(val.getBuffer(), _T("AND"))==0 || _tcscmp(val.getBuffer(), _T("&&"))==0 )
      token->set(val.getBuffer(), QueryToken::AND_);
   else if( _tcsicmp(val.getBuffer(), _T("OR"))==0 || _tcscmp(val.getBuffer(), _T("||"))==0)
      token->set(val.getBuffer(), QueryToken::OR);
   else if( _tcsicmp(val.getBuffer(), _T("NOT"))==0 )
      token->set(val.getBuffer(), QueryToken::NOT);
   else {
      bool isnum = true;
      int32_t nlen=val.length();
      for (int32_t i=0;i<nlen;++i) {
         TCHAR ch=val.getBuffer()[i];
         if ( _istalpha(ch) ) {
            isnum=false;
            break;
         }
      }

      if ( isnum )
         token->set(val.getBuffer(), QueryToken::NUMBER);
      else
         token->set(val.getBuffer(), QueryToken::TERM);
   }
   return true;
}


bool Lexer::ReadEscape(TCHAR prev, TCHAR* buf) {
   TCHAR ch = prev;
   int bp=0;
   buf[bp++] = ch;

   ch = reader->GetNext();
   int32_t idx = _tcscspn( buf, _T("\\+-!():^[]{}\"~*") );
   if(idx == 0) {
    buf[bp++] = ch;
    buf[bp++]=0;
    return true;
   }
   queryparser->throwParserException(_T("Unrecognized escape sequence at %d %d::%d"), ' ',
      reader->Column(),reader->Line());
   return false;
}


CL_NS_END2
