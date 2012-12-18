/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "CLucene/_ApiHeader.h"
#include "QueryParserTokenManager.h"
#include "_CharStream.h"
#include "_FastCharStream.h"
#include "QueryToken.h"

#include "CLucene/util/StringBuffer.h"

CL_NS_DEF(queryParser)

const int64_t QueryParserTokenManager::jjbitVec2[]={0x0L, 0x0L, _ILONGLONG(0xffffffffffffffff), _ILONGLONG(0xffffffffffffffff)};
const int64_t QueryParserTokenManager::jjbitVec0[] = {
	_ILONGLONG(0xfffffffffffffffe), _ILONGLONG(0xffffffffffffffff), _ILONGLONG(0xffffffffffffffff), _ILONGLONG(0xffffffffffffffff)
};
const int32_t QueryParserTokenManager::jjnextStates[]={
		15, 17, 18, 29, 32, 23, 33, 30, 20, 21, 32, 23, 33, 31, 34, 27,
		2, 4, 5, 0, 1
};
const TCHAR* QueryParserTokenManager::jjstrLiteralImages[]={
		_T(""), NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, _T("\53"), _T("\55)"), _T("\50)"),
		_T("\51"), _T("\72"), _T("\52"), _T("\136"), NULL, NULL, NULL, NULL, NULL, _T("\133"), _T("\173"), NULL,
		_T("\124\117"), _T("\135"), NULL, NULL, _T("\124\117"), _T("\175"), NULL, NULL
};
const TCHAR* QueryParserTokenManager::lexStateNames [] = {
		_T("Boost"),
		_T("RangeEx"),
		_T("RangeIn"),
		_T("DEFAULT")
};
const int32_t QueryParserTokenManager::jjnewLexState [] = {
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, -1, -1, -1, -1, -1, 2, 1, 3,
		-1, 3, -1, -1, -1, 3, -1, -1
};
const int64_t QueryParserTokenManager::jjtoToken [] = {
		_ILONGLONG(0x1ffffff81)
};
const int64_t QueryParserTokenManager::jjtoSkip [] = {
		_ILONGLONG(0x40)
};

QueryParserTokenManager::QueryParserTokenManager(CharStream* stream, const int32_t lexState) :
	input_stream(stream), curChar(0), curLexState(3), defaultLexState(3),jjnewStateCnt(0),jjround(0),
	jjmatchedPos(0),jjmatchedKind(0)
{
	if (lexState > -1)
		SwitchTo(lexState);
}
QueryParserTokenManager::~QueryParserTokenManager()
{
	_CLLDELETE(input_stream);
}

int32_t QueryParserTokenManager::jjStopStringLiteralDfa_3(const int32_t /*pos*/, int64_t /*active0*/)
{
	return -1;
}

int32_t QueryParserTokenManager::jjStartNfa_3(int32_t pos, int64_t active0)
{
	return jjMoveNfa_3(jjStopStringLiteralDfa_3(pos, active0), pos + 1);
}

int32_t QueryParserTokenManager::jjStopAtPos(const int32_t pos, const int32_t kind)
{
	jjmatchedKind = kind;
	jjmatchedPos = pos;
	return pos + 1;
}

int32_t QueryParserTokenManager::jjStartNfaWithStates_3(int32_t pos, int32_t kind, int32_t state)
{
	jjmatchedKind = kind;
	jjmatchedPos = pos;
	try { curChar = input_stream->readChar(); }
	catch(CLuceneError& e) {
		if (e.number() != CL_ERR_IO)
		{
			throw e;
		}
		else
			return pos + 1;
	}
	return jjMoveNfa_3(state, pos + 1);
}

int32_t QueryParserTokenManager::jjMoveStringLiteralDfa0_3()
{
	switch(curChar)
	{
	case 40:
		return jjStopAtPos(0, 12);
	case 41:
		return jjStopAtPos(0, 13);
	case 42:
		return jjStartNfaWithStates_3(0, 15, 36);
	case 43:
		return jjStopAtPos(0, 10);
	case 45:
		return jjStopAtPos(0, 11);
	case 58:
		return jjStopAtPos(0, 14);
	case 91:
		return jjStopAtPos(0, 22);
	case 94:
		return jjStopAtPos(0, 16);
	case 123:
		return jjStopAtPos(0, 23);
	default :
		return jjMoveNfa_3(0, 0);
	}
}

void QueryParserTokenManager::jjCheckNAdd(const int32_t state)
{
	if (jjrounds[state] != jjround)
	{
		jjstateSet[jjnewStateCnt++] = state;
		jjrounds[state] = jjround;
	}
}

void QueryParserTokenManager::jjAddStates(int32_t start, const int32_t end)
{
	do {
		jjstateSet[jjnewStateCnt++] = jjnextStates[start];
	} while (start++ != end);
}

void QueryParserTokenManager::jjCheckNAddTwoStates(const int32_t state1, const int32_t state2)
{
	jjCheckNAdd(state1);
	jjCheckNAdd(state2);
}

void QueryParserTokenManager::jjCheckNAddStates(int32_t start, const int32_t end)
{
	do {
		jjCheckNAdd(jjnextStates[start]);
	} while (start++ != end);
}

void QueryParserTokenManager::jjCheckNAddStates(const int32_t start)
{
	jjCheckNAdd(jjnextStates[start]);
	jjCheckNAdd(jjnextStates[start + 1]);
}

int32_t QueryParserTokenManager::jjMoveNfa_3(const int32_t startState, int32_t curPos)
{
	int32_t startsAt = 0;
	jjnewStateCnt = 36;
	int32_t i = 1;
	jjstateSet[0] = startState;
	int32_t kind = 0x7fffffff;
	for (;;)
	{
		if (++jjround == 0x7fffffff)
			ReInitRounds();
		if (curChar < 64)
		{
				uint64_t l = (uint64_t) (((uint64_t)1L) << ((int32_t)curChar));
			do
			{
			   switch(jjstateSet[--i])
			   {
			   case 36:
			   case 25:
				   if (( _ILONGLONG(0xfbfffcf8ffffd9ff) & l) == 0L)
					   break;
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 0:
				   if (( _ILONGLONG(0xfbffd4f8ffffd9ff) & l) != 0L)
				   {
					   if (kind > 21)
						   kind = 21;
					   jjCheckNAddTwoStates(25, 26);
				   }
				   else if ((_ILONGLONG(0x100002600) & l) != 0L)
				   {
					   if (kind > 6)
						   kind = 6;
				   }
				   else if (curChar == 34)
					   jjCheckNAddStates(0, 2);
				   else if (curChar == 33)
				   {
					   if (kind > 9)
						   kind = 9;
				   }
				   if ((_ILONGLONG(0x7bffd0f8ffffd9ff) & l) != 0L)
				   {
					   if (kind > 18)
						   kind = 18;
					   jjCheckNAddStates(3, 7);
				   }
				   else if (curChar == 42)
				   {
					   if (kind > 20)
						   kind = 20;
				   }
				   if (curChar == 38)
					   jjstateSet[jjnewStateCnt++] = 4;
				   break;
			   case 4:
				   if (curChar == 38 && kind > 7)
					   kind = 7;
				   break;
			   case 5:
				   if (curChar == 38)
					   jjstateSet[jjnewStateCnt++] = 4;
				   break;
			   case 13:
				   if (curChar == 33 && kind > 9)
					   kind = 9;
				   break;
			   case 14:
			   case 16:
				   if (curChar == 34)
					   jjCheckNAddStates(0, 2);
				   break;
			   case 15:
				   if ((_ILONGLONG(0xfffffffbffffffff) & l) != 0L)
					   jjCheckNAddStates(0, 2);
				   break;
			   case 18:
				   if (curChar == 34 && kind > 17)
					   kind = 17;
				   break;
			   case 20:
				   if ((_ILONGLONG(0x3ff000000000000) & l) == 0L)
					   break;
				   if (kind > 19)
					   kind = 19;
				   jjAddStates(8, 9);
				   break;
			   case 21:
				   if (curChar == 46)
					   jjCheckNAdd(22);
				   break;
			   case 22:
				   if ((_ILONGLONG(0x3ff000000000000) & l) == 0L)
					   break;
				   if (kind > 19)
					   kind = 19;
				   jjCheckNAdd(22);
				   break;
			   case 23:
				   if (curChar == 42 && kind > 20)
					   kind = 20;
				   break;
			   case 24:
				   if ((_ILONGLONG(0xfbffd4f8ffffd9ff) & l) == 0L)
					   break;
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 27:
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 28:
				   if ((_ILONGLONG(0x7bffd0f8ffffd9ff) & l) == 0L)
					   break;
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddStates(3, 7);
				   break;
			   case 29:
				   if ((_ILONGLONG(0x7bfff8f8ffffd9ff) & l) == 0L)
					   break;
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddTwoStates(29, 30);
				   break;
			   case 31:
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddTwoStates(29, 30);
				   break;
			   case 32:
				   if ((_ILONGLONG(0x7bfff8f8ffffd9ff) & l) != 0L)
					   jjCheckNAddStates(10, 12);
				   break;
			   case 34:
				   jjCheckNAddStates(10, 12);
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		else if (curChar < 128)
		{
			uint64_t l = ((uint64_t)1L) << (curChar & 63);

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   case 36:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) != 0L)
				   {
					   if (kind > 21)
						   kind = 21;
					   jjCheckNAddTwoStates(25, 26);
				   }
				   else if (curChar == 92)
					   jjCheckNAddTwoStates(27, 27);
				   break;
			   case 0:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) != 0L)
				   {
					   if (kind > 18)
						   kind = 18;
					   jjCheckNAddStates(3, 7);
				   }
				   else if (curChar == 92)
					   jjCheckNAddStates(13, 15);
				   else if (curChar == 126)
				   {
					   if (kind > 19)
						   kind = 19;
					   jjstateSet[jjnewStateCnt++] = 20;
				   }
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) != 0L)
				   {
					   if (kind > 21)
						   kind = 21;
					   jjCheckNAddTwoStates(25, 26);
				   }
				   if (curChar == 78)
					   jjstateSet[jjnewStateCnt++] = 11;
				   else if (curChar == 124)
					   jjstateSet[jjnewStateCnt++] = 8;
				   else if (curChar == 79)
					   jjstateSet[jjnewStateCnt++] = 6;
				   else if (curChar == 65)
					   jjstateSet[jjnewStateCnt++] = 2;
				   break;
			   case 1:
				   if (curChar == 68 && kind > 7)
					   kind = 7;
				   break;
			   case 2:
				   if (curChar == 78)
					   jjstateSet[jjnewStateCnt++] = 1;
				   break;
			   case 3:
				   if (curChar == 65)
					   jjstateSet[jjnewStateCnt++] = 2;
				   break;
			   case 6:
				   if (curChar == 82 && kind > 8)
					   kind = 8;
				   break;
			   case 7:
				   if (curChar == 79)
					   jjstateSet[jjnewStateCnt++] = 6;
				   break;
			   case 8:
				   if (curChar == 124 && kind > 8)
					   kind = 8;
				   break;
			   case 9:
				   if (curChar == 124)
					   jjstateSet[jjnewStateCnt++] = 8;
				   break;
			   case 10:
				   if (curChar == 84 && kind > 9)
					   kind = 9;
				   break;
			   case 11:
				   if (curChar == 79)
					   jjstateSet[jjnewStateCnt++] = 10;
				   break;
			   case 12:
				   if (curChar == 78)
					   jjstateSet[jjnewStateCnt++] = 11;
				   break;
			   case 15:
				   jjAddStates(0, 2);
				   break;
			   case 17:
				   if (curChar == 92)
					   jjstateSet[jjnewStateCnt++] = 16;
				   break;
			   case 19:
				   if (curChar != 126)
					   break;
				   if (kind > 19)
					   kind = 19;
				   jjstateSet[jjnewStateCnt++] = 20;
				   break;
			   case 24:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) == 0L)
					   break;
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 25:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) == 0L)
					   break;
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 26:
				   if (curChar == 92)
					   jjCheckNAddTwoStates(27, 27);
				   break;
			   case 27:
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 28:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) == 0L)
					   break;
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddStates(3, 7);
				   break;
			   case 29:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) == 0L)
					   break;
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddTwoStates(29, 30);
				   break;
			   case 30:
				   if (curChar == 92)
					   jjCheckNAddTwoStates(31, 31);
				   break;
			   case 31:
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddTwoStates(29, 30);
				   break;
			   case 32:
				   if ((_ILONGLONG(0x97ffffff87ffffff) & l) != 0L)
					   jjCheckNAddStates(10, 12);
				   break;
			   case 33:
				   if (curChar == 92)
					   jjCheckNAddTwoStates(34, 34);
				   break;
			   case 34:
				   jjCheckNAddStates(10, 12);
				   break;
			   case 35:
				   if (curChar == 92)
					   jjCheckNAddStates(13, 15);
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		else
		{
			int32_t hiByte = (int32_t)(curChar >> 8);
			int32_t i1 = hiByte >> 6;
			uint64_t l1 = ((uint64_t)1L) << (hiByte & 63);
			int32_t i2 = (curChar & 0xff) >> 6;
			uint64_t l2 = ((uint64_t)1L) << (curChar & 63);

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   case 36:
			   case 25:
			   case 27:
				   if (!jjCanMove_0(hiByte, i1, i2, l1, l2))
					   break;
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 0:
				   if (jjCanMove_0(hiByte, i1, i2, l1, l2))
				   {
					   if (kind > 21)
						   kind = 21;
					   jjCheckNAddTwoStates(25, 26);
				   }
				   if (jjCanMove_0(hiByte, i1, i2, l1, l2))
				   {
					   if (kind > 18)
						   kind = 18;
					   jjCheckNAddStates(3, 7);
				   }
				   break;
			   case 15:
				   if (jjCanMove_0(hiByte, i1, i2, l1, l2))
					   jjAddStates(0, 2);
				   break;
			   case 24:
				   if (!jjCanMove_0(hiByte, i1, i2, l1, l2))
					   break;
				   if (kind > 21)
					   kind = 21;
				   jjCheckNAddTwoStates(25, 26);
				   break;
			   case 28:
				   if (!jjCanMove_0(hiByte, i1, i2, l1, l2))
					   break;
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddStates(3, 7);
				   break;
			   case 29:
			   case 31:
				   if (!jjCanMove_0(hiByte, i1, i2, l1, l2))
					   break;
				   if (kind > 18)
					   kind = 18;
				   jjCheckNAddTwoStates(29, 30);
				   break;
			   case 32:
			   case 34:
				   if (jjCanMove_0(hiByte, i1, i2, l1, l2))
					   jjCheckNAddStates(10, 12);
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		if (kind != 0x7fffffff)
		{
			jjmatchedKind = kind;
			jjmatchedPos = curPos;
			kind = 0x7fffffff;
		}
		++curPos;
		if ((i = jjnewStateCnt) == (startsAt = 36 - (jjnewStateCnt = startsAt)))
			return curPos;
		try { curChar = input_stream->readChar(); }
		catch(CLuceneError& e) {
			if (e.number() != CL_ERR_IO)
			{
				throw e;
			}
			else
				return curPos;
		}
	}
}

int32_t QueryParserTokenManager::jjStopStringLiteralDfa_1(const int32_t pos, const int64_t active0)
{
	switch (pos)
	{
	case 0:
		if ((active0 & 0x20000000L) != 0L)
		{
			jjmatchedKind = 32;
			return 6;
		}
		return -1;
	default :
		return -1;
	}
}

int32_t QueryParserTokenManager::jjStartNfa_1(int32_t pos, int64_t active0)
{
	return jjMoveNfa_1(jjStopStringLiteralDfa_1(pos, active0), pos + 1);
}

int32_t QueryParserTokenManager::jjStartNfaWithStates_1(const int32_t pos, const int32_t kind, const int32_t state)
{
	jjmatchedKind = kind;
	jjmatchedPos = pos;
	try { curChar = input_stream->readChar(); }
	catch(CLuceneError& e) {
		if (e.number() != CL_ERR_IO)
		{
			throw e;
		}
		else
			return pos + 1;
	}
	return jjMoveNfa_1(state, pos + 1);
}

int32_t QueryParserTokenManager::jjMoveStringLiteralDfa0_1()
{
	switch(curChar)
	{
	case 84:
		return jjMoveStringLiteralDfa1_1(0x20000000L);
	case 125:
		return jjStopAtPos(0, 30);
	default :
		return jjMoveNfa_1(0, 0);
	}
}

int32_t QueryParserTokenManager::jjMoveStringLiteralDfa1_1(int64_t active0)
{
	try { curChar = input_stream->readChar(); }
	catch(CLuceneError& e) {
		if (e.number() != CL_ERR_IO)
		{
			throw e;
		}
		else {
			jjStopStringLiteralDfa_1(0, active0);
			return 1;
		}
	}
	switch(curChar)
	{
	case 79:
		if ((active0 & 0x20000000L) != 0L)
			return jjStartNfaWithStates_1(1, 29, 6);
		break;
	default :
		break;
	}
	return jjStartNfa_1(0, active0);
}

int32_t QueryParserTokenManager::jjMoveNfa_1(const int32_t startState, int32_t curPos)
{
	int32_t startsAt = 0;
	jjnewStateCnt = 7;
	int32_t i = 1;
	jjstateSet[0] = startState;
	int32_t kind = 0x7fffffff;
	for (;;)
	{
		if (++jjround == 0x7fffffff)
			ReInitRounds();
		if (curChar < 64)
		{
			uint64_t l = ((uint64_t)1L) << curChar;

			do
			{
				switch(jjstateSet[--i])
				{
				case 0:
					if ((_ILONGLONG(0xfffffffeffffffff) & l) != 0L)
					{
						if (kind > 32)
							kind = 32;
						jjCheckNAdd(6);
					}
					if ((_ILONGLONG(0x100002600) & l) != 0L)
					{
						if (kind > 6)
							kind = 6;
					}
					else if (curChar == 34)
						jjCheckNAddTwoStates(2, 4);
					break;
				case 1:
					if (curChar == 34)
						jjCheckNAddTwoStates(2, 4);
					break;
				case 2:
					if ((_ILONGLONG(0xfffffffbffffffff) & l) != 0L)
						jjCheckNAddStates(16, 18);
					break;
				case 3:
					if (curChar == 34)
						jjCheckNAddStates(16, 18);
					break;
				case 5:
					if (curChar == 34 && kind > 31)
						kind = 31;
					break;
				case 6:
					if ((_ILONGLONG(0xfffffffeffffffff) & l) == 0L)
						break;
					if (kind > 32)
						kind = 32;
					jjCheckNAdd(6);
					break;
				default : break;
				}
			} while(i != startsAt);
		}
		else if (curChar < 128)
		{
			uint64_t l = ((uint64_t)1L) << (curChar & 63);

			do
			{
				switch(jjstateSet[--i])
				{
				case 0:
				case 6:
					if ((_ILONGLONG(0xdfffffffffffffff) & l) == 0L)
						break;
					if (kind > 32)
						kind = 32;
					jjCheckNAdd(6);
					break;
				case 2:
					jjAddStates(16, 18);
					break;
				case 4:
					if (curChar == 92)
						jjstateSet[jjnewStateCnt++] = 3;
					break;
				default : break;
				}
			} while(i != startsAt);
		}
		else
		{
			int32_t hiByte = (int32_t)(curChar >> 8);
			int32_t i1 = hiByte >> 6;
			uint64_t l1 = ((uint64_t)1L) << (hiByte & 63);
			int32_t i2 = (curChar & 0xff) >> 6;
			uint64_t l2 = ((uint64_t)1L) << (curChar & 63);

			do
			{
				switch(jjstateSet[--i])
				{
				case 0:
				case 6:
					if (!jjCanMove_0(hiByte, i1, i2, l1, l2))
						break;
					if (kind > 32)
						kind = 32;
					jjCheckNAdd(6);
					break;
				case 2:
					if (jjCanMove_0(hiByte, i1, i2, l1, l2))
						jjAddStates(16, 18);
					break;
				default : break;
				}
			} while(i != startsAt);
		}
		if (kind != 0x7fffffff)
		{
			jjmatchedKind = kind;
			jjmatchedPos = curPos;
			kind = 0x7fffffff;
		}
		++curPos;
		if ((i = jjnewStateCnt) == (startsAt = 7 - (jjnewStateCnt = startsAt)))
			return curPos;
		try { curChar = input_stream->readChar(); }
		catch(CLuceneError& e) {
			if (e.number() != CL_ERR_IO)
			{
				throw e;
			}
			else
				return curPos;
		}
	}
}

int32_t QueryParserTokenManager::jjMoveStringLiteralDfa0_0()
{
	return jjMoveNfa_0(0, 0);
}

int32_t QueryParserTokenManager::jjMoveNfa_0(const int32_t startState, int32_t curPos)
{
	int32_t startsAt = 0;
	jjnewStateCnt = 3;
	int32_t i = 1;
	jjstateSet[0] = startState;
	int32_t kind = 0x7fffffff;
	for (;;)
	{
		if (++jjround == 0x7fffffff)
			ReInitRounds();
		if (curChar < 64)
		{
			uint64_t l = ((uint64_t)1L) << curChar;

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   case 0:
				   if ((_ILONGLONG(0x3ff000000000000) & l) == 0L)
					   break;
				   if (kind > 24)
					   kind = 24;
				   jjAddStates(19, 20);
				   break;
			   case 1:
				   if (curChar == 46)
					   jjCheckNAdd(2);
				   break;
			   case 2:
				   if ((_ILONGLONG(0x3ff000000000000) & l) == 0L)
					   break;
				   if (kind > 24)
					   kind = 24;
				   jjCheckNAdd(2);
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		else if (curChar < 128)
		{
			/*
            uint64_t l = ((uint64_t)1L) << (curChar & 63);
			do
		   {
			   switch(jjstateSet[--i])
			   {
			   default : break;
			   }
		   } while(i != startsAt);*/
			i = startsAt;
		}
		else
		{
			/*
            int32_t hiByte = (int)(curChar >> 8);
			int32_t i1 = hiByte >> 6;
			uint64_t l1 = ((uint64_t)1L) << (hiByte & 63);
			int32_t i2 = (curChar & 0xff) >> 6;
			uint64_t l2 = ((uint64_t)1L) << (curChar & 63);

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   default : break;
			   }
		   } while(i != startsAt);*/
		   i = startsAt;
		}
		if (kind != 0x7fffffff)
		{
			jjmatchedKind = kind;
			jjmatchedPos = curPos;
			kind = 0x7fffffff;
		}
		++curPos;
		if ((i = jjnewStateCnt) == (startsAt = 3 - (jjnewStateCnt = startsAt)))
			return curPos;
		try { curChar = input_stream->readChar(); }
		catch(CLuceneError& e) {
			if (e.number() != CL_ERR_IO)
			{
				throw e;
			}
			else
				return curPos;
		}
	}
}

int32_t QueryParserTokenManager::jjStopStringLiteralDfa_2(const int32_t pos, const int64_t active0)
{
	switch (pos)
	{
	case 0:
		if ((active0 & 0x2000000L) != 0L)
		{
			jjmatchedKind = 28;
			return 6;
		}
		return -1;
	default :
		return -1;
	}
}

int32_t QueryParserTokenManager::jjStartNfa_2(int32_t pos, int64_t active0)
{
	return jjMoveNfa_2(jjStopStringLiteralDfa_2(pos, active0), pos + 1);
}

int32_t QueryParserTokenManager::jjStartNfaWithStates_2(const int32_t pos, const int32_t kind, const int32_t state)
{
	jjmatchedKind = kind;
	jjmatchedPos = pos;
	try { curChar = input_stream->readChar(); }
	catch(CLuceneError& e) {
		if (e.number() != CL_ERR_IO)
		{
			throw e;
		}
		else
			return pos+1;
	}
	return jjMoveNfa_2(state, pos + 1);
}

int32_t QueryParserTokenManager::jjMoveStringLiteralDfa0_2()
{
	switch(curChar)
	{
	case 84:
		return jjMoveStringLiteralDfa1_2(0x2000000L);
	case 93:
		return jjStopAtPos(0, 26);
	default :
		return jjMoveNfa_2(0, 0);
	}
}

int32_t QueryParserTokenManager::jjMoveStringLiteralDfa1_2(const int64_t active0)
{
	try { curChar = input_stream->readChar(); }
	catch(CLuceneError& e) {
		if (e.number() != CL_ERR_IO)
		{
			throw e;
		}
		else {
			jjStopStringLiteralDfa_2(0, active0);
			return 1;
		}
	}
	switch(curChar)
	{
	case 79:
		if ((active0 & 0x2000000L) != 0L)
			return jjStartNfaWithStates_2(1, 25, 6);
		break;
	default :
		break;
	}
	return jjStartNfa_2(0, active0);
}

int32_t QueryParserTokenManager::jjMoveNfa_2(const int32_t startState, int32_t curPos)
{
	int32_t startsAt = 0;
	jjnewStateCnt = 7;
	int32_t i = 1;
	jjstateSet[0] = startState;
	int32_t kind = 0x7fffffff;
	for (;;)
	{
		if (++jjround == 0x7fffffff)
			ReInitRounds();
		if (curChar < 64)
		{
			uint64_t l = ((uint64_t)1L) << curChar;

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   case 0:
				   if ((_ILONGLONG(0xfffffffeffffffff) & l) != 0L)
				   {
					   if (kind > 28)
						   kind = 28;
					   jjCheckNAdd(6);
				   }
				   if ((_ILONGLONG(0x100002600) & l) != 0L)
				   {
					   if (kind > 6)
						   kind = 6;
				   }
				   else if (curChar == 34)
					   jjCheckNAddTwoStates(2, 4);
				   break;
			   case 1:
				   if (curChar == 34)
					   jjCheckNAddTwoStates(2, 4);
				   break;
			   case 2:
				   if ((_ILONGLONG(0xfffffffbffffffff) & l) != 0L)
					   jjCheckNAddStates(16, 18);
				   break;
			   case 3:
				   if (curChar == 34)
					   jjCheckNAddStates(16, 18);
				   break;
			   case 5:
				   if (curChar == 34 && kind > 27)
					   kind = 27;
				   break;
			   case 6:
				   if ((_ILONGLONG(0xfffffffeffffffff) & l) == 0L)
					   break;
				   if (kind > 28)
					   kind = 28;
				   jjCheckNAdd(6);
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		else if (curChar < 128)
		{
			uint64_t l = ((uint64_t)1L) << (curChar & 63);

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   case 0:
			   case 6:
				   if ((_ILONGLONG(0xffffffffdfffffff) & l) == 0L)
					   break;
				   if (kind > 28)
					   kind = 28;
				   jjCheckNAdd(6);
				   break;
			   case 2:
				   jjAddStates(16, 18);
				   break;
			   case 4:
				   if (curChar == 92)
					   jjstateSet[jjnewStateCnt++] = 3;
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		else
		{
			int32_t hiByte = (int32_t)(curChar >> 8);
			int32_t i1 = hiByte >> 6;
			uint64_t l1 = ((uint64_t)1L) << (hiByte & 63);
			int32_t i2 = (curChar & 0xff) >> 6;
			uint64_t l2 = ((uint64_t)1L) << (curChar & 63);

			do
		   {
			   switch(jjstateSet[--i])
			   {
			   case 0:
			   case 6:
				   if (!jjCanMove_0(hiByte, i1, i2, l1, l2))
					   break;
				   if (kind > 28)
					   kind = 28;
				   jjCheckNAdd(6);
				   break;
			   case 2:
				   if (jjCanMove_0(hiByte, i1, i2, l1, l2))
					   jjAddStates(16, 18);
				   break;
			   default : break;
			   }
		   } while(i != startsAt);
		}
		if (kind != 0x7fffffff)
		{
			jjmatchedKind = kind;
			jjmatchedPos = curPos;
			kind = 0x7fffffff;
		}
		++curPos;
		if ((i = jjnewStateCnt) == (startsAt = 7 - (jjnewStateCnt = startsAt)))
			return curPos;
		try { curChar = input_stream->readChar(); }
		catch(CLuceneError& e) {
			if (e.number() != CL_ERR_IO)
			{
				throw e;
			}
			else
				return curPos;
		}
	}
}

/*static*/
bool QueryParserTokenManager::jjCanMove_0(const int32_t hiByte, const int32_t i1, const int32_t i2, const int64_t l1, const int64_t l2)
{
	switch(hiByte)
	{
	case 0:
		return ((jjbitVec2[i2] & l2) != 0L);
	default :
		if ((jjbitVec0[i1] & l1) != 0L)
			return true;
		return false;
	}
}

void QueryParserTokenManager::ReInit(CharStream* stream)
{
	jjmatchedPos = jjnewStateCnt = 0;
	curLexState = defaultLexState;
	_CLLDELETE(input_stream);
	input_stream = stream;
	ReInitRounds();
}

void QueryParserTokenManager::ReInitRounds()
{
	jjround = 0x80000001;
	for (int32_t i = 36; i-- > 0;)
		jjrounds[i] = 0x80000000;
}

void QueryParserTokenManager::ReInit(CharStream* stream, const int32_t lexState)
{
	ReInit(stream);
	SwitchTo(lexState);
}

void QueryParserTokenManager::SwitchTo(const int32_t lexState)
{
	if (lexState >= 4 || lexState < 0) {
		TCHAR err[CL_MAX_PATH];
		// TODO: use TokenMgrError::INVALID_LEXICAL_STATE?
		_sntprintf(err,CL_MAX_PATH,_T("Error: Ignoring invalid lexical state : %d.  State unchanged."), lexState);
		_CLTHROWA(CL_ERR_TokenMgr,err);
	}
	else
		curLexState = lexState;
}

QueryToken* QueryParserTokenManager::jjFillToken(){
	QueryToken* t = QueryToken::newToken(jjmatchedKind);
	t->kind = jjmatchedKind;
	const TCHAR* im = jjstrLiteralImages[jjmatchedKind];
	t->image = (im == NULL) ? input_stream->GetImage() : stringDuplicate(im);
	t->beginLine = input_stream->getBeginLine();
	t->beginColumn = input_stream->getBeginColumn();
	t->endLine = input_stream->getEndLine();
	t->endColumn = input_stream->getEndColumn();
	return t;
}

QueryToken* QueryParserTokenManager::getNextToken() {
	QueryToken* matchedToken = NULL;
	int32_t curPos = 0;

	for (;;) {
		try
		{
			curChar = input_stream->BeginToken();
		}
		_CLCATCH_ERR_ELSE(CL_ERR_IO, { /*else*/
			jjmatchedKind = 0;
			matchedToken = jjFillToken();
			return matchedToken;
		});

		switch(curLexState){
		case 0:
			jjmatchedKind = 0x7fffffff;
			jjmatchedPos = 0;
			curPos = jjMoveStringLiteralDfa0_0();
			break;
		case 1:
			jjmatchedKind = 0x7fffffff;
			jjmatchedPos = 0;
			curPos = jjMoveStringLiteralDfa0_1();
			break;
		case 2:
			jjmatchedKind = 0x7fffffff;
			jjmatchedPos = 0;
			curPos = jjMoveStringLiteralDfa0_2();
			break;
		case 3:
			jjmatchedKind = 0x7fffffff;
			jjmatchedPos = 0;
			curPos = jjMoveStringLiteralDfa0_3();
			break;
		}
		if (jjmatchedKind != 0x7fffffff){
			if (jjmatchedPos + 1 < curPos)
				input_stream->backup(curPos - jjmatchedPos - 1);
			if ((jjtoToken[jjmatchedKind >> 6] & ((uint64_t)(1L << (jjmatchedKind & 63)))) != (uint64_t)0L)
			{
				matchedToken = jjFillToken();
				if (jjnewLexState[jjmatchedKind] != -1)
					curLexState = jjnewLexState[jjmatchedKind];
				return matchedToken;
			}
			else
			{
				if (jjnewLexState[jjmatchedKind] != -1)
					curLexState = jjnewLexState[jjmatchedKind];
				continue;
			}
		}
		int32_t error_line = input_stream->getEndLine();
		int32_t error_column = input_stream->getEndColumn();
		TCHAR* error_after = NULL;
		bool EOFSeen = false;
		try { input_stream->readChar(); input_stream->backup(1); }
		_CLCATCH_ERR_ELSE(CL_ERR_IO, { /*else*/
			EOFSeen = true;
			if (curPos <= 1) {
				error_after = _CL_NEWARRAY(TCHAR,2);
				error_after[0] = _T(' ');
				error_after[1] = 0;
			} else {
				error_after = input_stream->GetImage();
			}
			if (curChar == _T('\n') || curChar == _T('\r')) {
				error_line++;
				error_column = 0;
			}
			else
				error_column++;
		});
		if (!EOFSeen) {
			input_stream->backup(1);
			if (curPos <= 1) {
				error_after = _CL_NEWARRAY(TCHAR,2);
				error_after[0] = _T(' ');
				error_after[1] = 0;
			} else {
				error_after = input_stream->GetImage();
			}
		}
		// TODO: TokenMgrError.LEXICAL_ERROR ?
		TCHAR* err = getLexicalError(EOFSeen, curLexState, error_line, error_column, error_after, curChar);
		_CLDELETE_LCARRAY(error_after);
		_CLTHROWT_DEL(CL_ERR_TokenMgr,err);
	}
}

TCHAR* QueryParserTokenManager::getLexicalError(bool EOFSeen, int32_t /*lexState*/, int32_t errorLine,
												int32_t errorColumn, TCHAR* errorAfter, TCHAR curChar)
{
	TCHAR* tmp = NULL;
	CL_NS(util)::StringBuffer sb(100);
	sb.append(_T("Lexical error at line "));
	sb.appendInt(errorLine);
	sb.append(_T(", column "));
	sb.appendInt(errorColumn);
	sb.append(_T(".  Encountered: "));
	if (EOFSeen){
		sb.append(_T("<EOF> "));
	}else{
		sb.appendChar(_T('"'));
		sb.appendChar(curChar); // TODO: addEscapes ?
		sb.appendChar(_T('"'));
		sb.append(_T(" ("));
		sb.appendInt((int32_t)curChar);
		sb.append(_T("), "));
	}
	sb.append(_T("after : \""));

	tmp = addEscapes(errorAfter);
	sb.append(tmp);
	_CLDELETE_LCARRAY(tmp);

	sb.appendChar(_T('"'));
	return sb.giveBuffer();
}

CL_NS_END // QueryParserTokenManager
