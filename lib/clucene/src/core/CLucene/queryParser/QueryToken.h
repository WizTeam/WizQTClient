/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_Token_
#define _lucene_queryParser_Token_

CL_NS_DEF(queryParser)
	
/**
 * Describes the input token stream.
 */
class CLUCENE_EXPORT QueryToken{
public:
	QueryToken();
	~QueryToken();

	/**
	* An integer that describes the kind of this token.  This numbering
	* system is determined by JavaCCParser, and a table of these numbers is
	* stored in the file ...Constants.java.
	*/
	int32_t kind;

	/**
	* beginLine and beginColumn describe the position of the first character
	* of this token; endLine and endColumn describe the position of the
	* last character of this token.
	*/
	int32_t beginLine, beginColumn, endLine, endColumn;

	/**
	* The string image of the token.
	*/
	TCHAR* image;

	/*
	TODO: If LUCENE_TOKEN_WORD_LENGTH is still necessary, use the #defines below
	#ifdef LUCENE_TOKEN_WORD_LENGTH
	TCHAR image[LUCENE_TOKEN_WORD_LENGTH+1];
	#else
	TCHAR* image;
	#endif
	*/

	/**
	* A reference to the next regular (non-special) token from the input
	* stream.  If this is the last token from the input stream, or if the
	* token manager has not read tokens beyond this one, this field is
	* set to null.  This is true only if this token is also a regular
	* token.  Otherwise, see below for a description of the contents of
	* this field.
	*/
	QueryToken* next;

	/**
	* This field is used to access special tokens that occur prior to this
	* token, but after the immediately preceding regular (non-special) token.
	* If there are no such special tokens, this field is set to null.
	* When there are more than one such special token, this field refers
	* to the last of these special tokens, which in turn refers to the next
	* previous special token through its specialToken field, and so on
	* until the first special token (whose specialToken field is null).
	* The next fields of special tokens refer to other special tokens that
	* immediately follow it (without an intervening regular token).  If there
	* is no such token, this field is null.
	*/
	QueryToken* specialToken;

	/**
	* Returns the image.
	*/
	TCHAR* toString() const;

	/**
	* Returns a new Token object, by default. However, if you want, you
	* can create and return subclass objects based on the value of ofKind.
	* Simply add the cases to the switch for all those special cases.
	* For example, if you have a subclass of Token called IDToken that
	* you want to create if ofKind is ID, simlpy add something like :
	*
	*    case MyParserConstants.ID : return new IDToken();
	*
	* to the following switch statement. Then you can cast matchedToken
	* variable to the appropriate type and use it in your lexical actions.
	*/
	static QueryToken* newToken(const int32_t ofKind);

};
CL_NS_END
#endif

