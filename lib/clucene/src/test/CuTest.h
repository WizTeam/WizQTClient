/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef CU_TEST_H
#define CU_TEST_H

/* CuString */

TCHAR* CuWstrAlloc(int size);
TCHAR* CuWstrCopy(const TCHAR* old);

#define CU_ALLOC(TYPE)		((TYPE*) malloc(sizeof(TYPE)))
#define CU_TDUP(dest,src) dest=((TCHAR*)malloc(sizeof(TCHAR)*_tcslen(src)+sizeof(TCHAR)));_tcscpy(dest,src);

#define HUGE_STRING_LEN	8192
#define STRING_MAX		256
#define STRING_INC		256

#define CLUCENE_ASSERT(x) CuAssert(tc,_T("Assert Failed: ") _T(#x),x)

typedef struct
{
	int length;
	int size;
	TCHAR* buffer;
} CuString;

void CuStringInit(CuString* str);
CuString* CuStringNew(void);
void CuStringFree(CuString* str);
void CuStringRead(CuString* str, TCHAR* path);
void CuStringAppend(CuString* str, const TCHAR* text);
void CuStringAppendChar(CuString* str, TCHAR ch);
void CuStringAppendFormat(CuString* str, const TCHAR* format, ...);
void CuStringResize(CuString* str, int newSize);

/* CuTest */

typedef struct CuTest CuTest;

typedef void (*TestFunction)(CuTest *);

struct CuTest
{
	TCHAR* name;
	TestFunction function;
        int notimpl;
	int failed;
	int ran;
	TCHAR* message;
//	jmp_buf *jumpBuf;
};


void CuInit(int argc, char *argv[]);
void CuTestInit(CuTest* t, const TCHAR* name, TestFunction function);
CuTest* CuTestNew(const TCHAR* name, TestFunction function);
void CuTestDelete(CuTest* tst);
void CuFail(CuTest* tc, const TCHAR* format, ...);
void CuFail(CuTest* tc, CLuceneError& e);
void CuMessage(CuTest* tc, const TCHAR* message,...);
void CuMessageV(CuTest* tc, const TCHAR* format, va_list& argp);
void CuMessageA(CuTest* tc, const char* format, ...);
void CuNotImpl(CuTest* tc, const TCHAR* message);
void CuAssert(CuTest* tc, const TCHAR* message, int condition);
void CuAssertTrue(CuTest* tc, int condition, const TCHAR* msg = NULL);
void CuAssertEquals(CuTest* tc, const int32_t expected, const int32_t actual, const TCHAR* msg = NULL);
void CuAssertStrEquals(CuTest* tc, const TCHAR* preMessage, const TCHAR* expected, const TCHAR* actual);
void CuAssertStrEquals(CuTest* tc, const TCHAR* preMessage, const TCHAR* expected, TCHAR* actual, bool bDelActual = false);
void CuAssertIntEquals(CuTest* tc, const TCHAR* preMessage, int expected, int actual);
void CuAssertSizeEquals(CuTest* tc, const TCHAR* preMessage, int expected, int actual);
void CuAssertPtrEquals(CuTest* tc, const TCHAR* preMessage, const void* expected, const void* actual);
void CuAssertPtrNotNull(CuTest* tc, const TCHAR* preMessage, const void* pointer);

void CuTestRun(CuTest* tc);

/* CuSuite */

#define MAX_TEST_CASES	1024	

#define SUITE_ADD_TEST(SUITE,TEST)	CuSuiteAdd(SUITE, CuTestNew(_T(#TEST), TEST))

/*
 * Macros used to make porting of Java Lucene tests easier. Assumes CuTest exists in the scope as tc
 */
#define assertTrue(CND)                                 CuAssertTrue(tc, CND)
#define assertTrueMsg(MSG, CND)                         CuAssertTrue(tc, CND, MSG)
#define assertEquals(EXPECTED, ACTUAL)                  CuAssertEquals(tc, EXPECTED, ACTUAL)
#define assertEqualsMsg(MSG, EXPECTED, ACTUAL)          CuAssertEquals(tc, EXPECTED, ACTUAL, MSG)

extern char clucene_data_location[1024];

typedef struct
{
	TCHAR *name;
	int count;
	CuTest* list[MAX_TEST_CASES]; 
	int failCount;
	int notimplCount;
	uint64_t timeTaken;
} CuSuite;


void CuSuiteInit(CuSuite* testSuite, const TCHAR* name);
CuSuite* CuSuiteNew(const TCHAR* name);
void CuSuiteDelete(CuSuite* suite);
void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase);
void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2);
void CuSuiteRun(CuSuite* testSuite);
void CuSuiteSummary(CuSuite* testSuite, CuString* summary, bool times);
void CuSuiteOverView(CuSuite* testSuite, CuString* details);
void CuSuiteDetails(CuSuite* testSuite, CuString* details);

typedef struct
{
	TCHAR *name;
	int count;
	CuSuite* list[MAX_TEST_CASES]; 
} CuSuiteList;


struct unittest {
    const char *testname;
    CuSuite *(*func)(void);
};

CuSuiteList* CuSuiteListNew(const TCHAR* name);
void CuSuiteListDelete(CuSuiteList* lst);
void CuSuiteListAdd(CuSuiteList* testSuite, CuSuite *testCase);
void CuSuiteListRun(CuSuiteList* testSuite);
void CuSuiteListRunWithSummary(CuSuiteList* testSuite, bool verbose, bool times);
//void CuSuiteListSummary(CuSuiteList* testSuite, CuString* summary);
/* Print details of test suite results; returns total number of
 * tests which failed. */
int CuSuiteListDetails(CuSuiteList* testSuite, CuString* details);
#endif /* CU_TEST_H */
