/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_legacy_QueryToken_
#define _lucene_queryParser_legacy_QueryToken_

CL_NS_DEF2(queryParser,legacy)
	
	// Token class that used by QueryParser.
	class CLUCENE_EXPORT QueryToken:LUCENE_BASE
	{
	public:
		enum Types
		{
			AND_,
			OR,
			NOT,
			PLUS,
			MINUS,
			LPAREN,
			RPAREN,
			COLON,
			CARAT,
			QUOTED,
			TERM,
			SLOP,
			FUZZY,
			PREFIXTERM,
			WILDTERM,
			RANGEIN,
			RANGEEX,
			NUMBER,
			EOF_,
			UNKNOWN_
		};


        #ifdef LUCENE_TOKEN_WORD_LENGTH
		TCHAR Value[LUCENE_TOKEN_WORD_LENGTH+1];
        #else
		TCHAR* Value;
        #endif

		int32_t Start;
		int32_t End;
		QueryToken::Types Type;

		// Initializes a new instance of the Token class.
		QueryToken(const TCHAR* value, const int32_t start, const int32_t end, const Types type);

		// Initializes a new instance of the Token class.
		QueryToken(const TCHAR* value, const Types type);

		// Initializes a new instance of the Token class.
		QueryToken(Types type);
		
		// Initializes an empty instance of the Token class.
		QueryToken();

		~QueryToken();
				
		void set(const TCHAR* value, const int32_t start, const int32_t end, const Types type);
		void set(const TCHAR* value, const Types type);
		void set(Types type);
	};
CL_NS_END2
#endif
