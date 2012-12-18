/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_queryParser_QueryParserTokenManager_
#define _lucene_queryParser_QueryParserTokenManager_

#include "QueryParserConstants.h"

CL_NS_DEF(queryParser)

class CharStream;
class QueryToken;

class CLUCENE_EXPORT QueryParserTokenManager: public virtual QueryParserConstants
{
public:
	// TODO: PrintStream debugStream = System.out;
	// TODO: setDebugStream(PrintStream ds) { debugStream = ds; }

private:
	int32_t jjStopStringLiteralDfa_3(const int32_t pos, int64_t active0);
	int32_t jjStartNfa_3(int32_t pos, int64_t active0);
	int32_t jjStopAtPos(const int32_t pos, const int32_t kind);

	int32_t jjStartNfaWithStates_3(int32_t pos, int32_t kind, int32_t state);

	int32_t jjMoveStringLiteralDfa0_3();

	void jjCheckNAdd(const int32_t state);
	void jjAddStates(int32_t start, const int32_t end);
	void jjCheckNAddTwoStates(const int32_t state1, const int32_t state2);
	void jjCheckNAddStates(int32_t start, const int32_t end);
	void jjCheckNAddStates(const int32_t start);

	static const int64_t jjbitVec0[];
	static const int64_t jjbitVec2[];

	int32_t jjMoveNfa_3(const int32_t startState, int32_t curPos);

	int32_t jjStopStringLiteralDfa_1(const int32_t pos, const int64_t active0);

	int32_t jjStartNfa_1(int32_t pos, int64_t active0);

	int32_t jjStartNfaWithStates_1(const int32_t pos, const int32_t kind, const int32_t state);

	int32_t jjMoveStringLiteralDfa0_1();
	int32_t jjMoveStringLiteralDfa1_1(int64_t active0);
	int32_t jjMoveNfa_1(const int32_t startState, int32_t curPos);
	int32_t jjMoveStringLiteralDfa0_0();

	int32_t jjMoveNfa_0(const int32_t startState, int32_t curPos);
	int32_t jjStopStringLiteralDfa_2(const int32_t pos, const int64_t active0);
	int32_t jjStartNfa_2(int32_t pos, int64_t active0);

	int32_t jjStartNfaWithStates_2(const int32_t pos, const int32_t kind, const int32_t state);

	int32_t jjMoveStringLiteralDfa0_2();
	int32_t jjMoveStringLiteralDfa1_2(const int64_t active0);

	int32_t jjMoveNfa_2(const int32_t startState, int32_t curPos);

	static const int32_t jjnextStates[];

	static bool jjCanMove_0(const int32_t hiByte, const int32_t i1, const int32_t i2, const int64_t l1,
		const int64_t l2);

public:
	static const TCHAR* jjstrLiteralImages [];
	static const TCHAR* lexStateNames [];
	static const int32_t jjnewLexState [];
	static const int64_t jjtoToken [];
	static const int64_t jjtoSkip [];
protected:
	CharStream* input_stream;
private:
	int32_t jjrounds[36];
	int32_t jjstateSet[72];
protected:
	TCHAR curChar;
public:
	QueryParserTokenManager(CharStream* stream, const int32_t lexState = -1);
	virtual ~QueryParserTokenManager();

	void ReInit(CharStream* stream);
private:
	void ReInitRounds();
public:
	void ReInit(CharStream* stream, const int32_t lexState);
	void SwitchTo(const int32_t lexState);

protected:
	QueryToken* jjFillToken();

	int32_t curLexState;
	int32_t defaultLexState;
	int32_t jjnewStateCnt;
	uint32_t jjround;
	int32_t jjmatchedPos;
	int32_t jjmatchedKind;

public:
	QueryToken* getNextToken();

private:
	static TCHAR* getLexicalError(bool EOFSeen, int32_t lexState, int32_t errorLine, int32_t errorColumn,
		TCHAR* errorAfter, TCHAR curChar);
};
CL_NS_END
#endif
