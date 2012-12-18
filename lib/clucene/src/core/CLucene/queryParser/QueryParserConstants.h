/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_QueryParserConstants_
#define _lucene_queryParser_QueryParserConstants_

CL_NS_DEF(queryParser)

class CLUCENE_EXPORT QueryParserConstants {
public:
	enum Types {
		_EOF = 0,
		_NUM_CHAR = 1,
		_ESCAPED_CHAR = 2,
		_TERM_START_CHAR = 3,
		_TERM_CHAR = 4,
		_WHITESPACE = 5,
		AND = 7,
		OR = 8,
		NOT = 9,
		PLUS = 10,
		MINUS = 11,
		LPAREN = 12,
		RPAREN = 13,
		COLON = 14,
		STAR = 15,
		CARAT = 16,
		QUOTED = 17,
		TERM = 18,
		FUZZY_SLOP = 19,
		PREFIXTERM = 20,
		WILDTERM = 21,
		RANGEIN_START = 22,
		RANGEEX_START = 23,
		NUMBER = 24,
		RANGEIN_TO = 25,
		RANGEIN_END = 26,
		RANGEIN_QUOTED = 27,
		RANGEIN_GOOP = 28,
		RANGEEX_TO = 29,
		RANGEEX_END = 30,
		RANGEEX_QUOTED = 31,
		RANGEEX_GOOP = 32
	};

	enum LexStates {
		Boost = 0,
		RangeEx = 1,
		RangeIn = 2,
		DEFAULT = 3
	};

	static const TCHAR* tokenImage[];

protected:
	/**
	* Used to convert raw characters to their escaped version
	* when these raw version cannot be used as part of an ASCII
	* string literal, while formatting an error message.
	* Called internally only by error reporting tools.
	*/
	static TCHAR* addEscapes(TCHAR* str);
};
CL_NS_END
#endif
