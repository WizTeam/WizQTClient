/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_legacy_TokenList_
#define _lucene_queryParser_legacy_TokenList_


//#include "QueryToken.h"
CL_NS_DEF2(queryParser,legacy)

class QueryToken;

	// Represents a list of the tokens.
	class TokenList:LUCENE_BASE
	{
	private:
		CL_NS(util)::CLVector<QueryToken*> tokens; //todo:,CL_NS(util)::Deletor::Object<QueryToken>
    public:
		TokenList();
		~TokenList();

		void add(QueryToken* token);

		void push(QueryToken* token);

		QueryToken* peek();

		QueryToken* extract();

		int32_t count() const;
	};
CL_NS_END2
#endif
