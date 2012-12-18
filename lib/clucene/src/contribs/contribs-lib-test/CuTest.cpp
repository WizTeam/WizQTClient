/*------------------------------------------------------------------------------
 * Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
 *
 * Distributable under the terms of either the Apache License (Version 2.0) or
 * the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CuTest.h"
#include <stdlib.h>
#include <assert.h>

static int verbose = 0;
static int messyPrinting = 0;

void CuInit(int argc, char *argv[]) {
    int i;

    for (i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-v")) {
            verbose = 1;
        }
        if (!strcmp(argv[i], "-p")) {
            messyPrinting = 1;
        }
    }
}

/*-------------------------------------------------------------------------*
 * CuTcs
 *-------------------------------------------------------------------------*/
TCHAR* CuStrAlloc(int size) {
    TCHAR* n = (TCHAR*) malloc(sizeof (TCHAR) * (size));
    return n;
}

TCHAR* CuTcsCopy(TCHAR* old) {
    int len = _tcslen(old);
    TCHAR* n = CuStrAlloc(len + 1);
    _tcscpy(n, old);
    return n;
}

TCHAR* CuTcsCat(TCHAR* orig, TCHAR* add) {
    int len = _tcslen(orig) + _tcslen(add);
    TCHAR* n = CuStrAlloc(len + 1);
    _tcscpy(n, orig);
    _tcscat(n, add);
    return n;
}

/*-------------------------------------------------------------------------*
 * CuString
 *-------------------------------------------------------------------------*/

TCHAR* CuTcsAlloc(int size) {
    TCHAR* n = (TCHAR*) malloc(sizeof (TCHAR) * (size));
    return n;
}

TCHAR* CuTcsCopy(const TCHAR* old) {
    int len = _tcslen(old);
    TCHAR* n = CuTcsAlloc(len + 1);
    _tcscpy(n, old);
    return n;
}

/*-------------------------------------------------------------------------*
 * CuString
 *-------------------------------------------------------------------------*/

void CuStringInit(CuString* str) {
    str->length = 0;
    str->size = STRING_MAX;
    str->buffer = (TCHAR*) malloc(sizeof (TCHAR) * str->size);
    str->buffer[0] = '\0';
}

CuString* CuStringNew(void) {
    CuString* str = (CuString*) malloc(sizeof (CuString));
    str->length = 0;
    str->size = STRING_MAX;
    str->buffer = (TCHAR*) malloc(sizeof (TCHAR) * str->size);
    str->buffer[0] = '\0';
    return str;
}

void CuStringFree(CuString* str) {
    free(str->buffer);
    free(str);
}

void CuStringResize(CuString* str, int newSize) {
    str->buffer = (TCHAR*) realloc(str->buffer, sizeof (TCHAR) * newSize);
    str->size = newSize;
}

void CuStringAppend(CuString* str, const TCHAR* text) {
    int length = _tcslen(text);
    if (str->length + length + 1 >= str->size)
        CuStringResize(str, str->length + length + 1 + STRING_INC);
    str->length += length;
    _tcscat(str->buffer, text);
}

void CuStringAppendChar(CuString* str, TCHAR ch) {
    TCHAR text[2];
    text[0] = ch;
    text[1] = '\0';
    CuStringAppend(str, text);
}

void CuStringAppendFormat(CuString* str, const TCHAR* format, ...) {
    TCHAR buf[HUGE_STRING_LEN];
    va_list argp;
    va_start(argp, format);
    _vsntprintf(buf, HUGE_STRING_LEN, format, argp);
    va_end(argp);
    CuStringAppend(str, buf);
}

void CuStringRead(CuString *str, TCHAR *path) {
    path = NULL;
    CU_TDUP(path, str->buffer);
}

/*-------------------------------------------------------------------------*
 * CuTest
 *-------------------------------------------------------------------------*/

void CuTestInit(CuTest* t, const TCHAR* name, TestFunction function) {
    t->name = CuTcsCopy(name);
    t->notimpl = 0;
    t->failed = 0;
    t->ran = 0;
    t->message = NULL;
    t->function = function;
    //	t->jumpBuf = NULL;
}

CuTest* CuTestNew(const TCHAR* name, TestFunction function) {
    CuTest* tc = CU_ALLOC(CuTest);
    CuTestInit(tc, name, function);
    return tc;
}

void CuTestDelete(CuTest* tst) {
    free(tst->name);
    if (tst->message != NULL)
        free(tst->message);
    free(tst);
}

void CuNotImpl(CuTest* tc, const TCHAR* message) {
    CuString* newstr = CuStringNew();
    CuStringAppend(newstr, message);
    CuStringAppend(newstr, _T(" not implemented on this platform"));
    tc->notimpl = 1;
    CuMessage(tc, newstr->buffer);
    CuStringFree(newstr);
    //	if (tc->jumpBuf != 0) longjmp(*(tc->jumpBuf), 0);
}

void CuFail(CuTest* tc, const TCHAR* format, ...) {
    tc->failed = 1;

    TCHAR buf[HUGE_STRING_LEN];
    va_list argp;
    va_start(argp, format);
    _vsntprintf(buf, HUGE_STRING_LEN, format, argp);
    va_end(argp);

    //	CuMessage(tc,buf);
    _CLTHROWT(CL_ERR_Runtime, buf);
}

void CuMessageV(CuTest* tc, const TCHAR* format, va_list& argp) {
    TCHAR buf[HUGE_STRING_LEN];
    _vsntprintf(buf, HUGE_STRING_LEN, format, argp);

    TCHAR* old = tc->message;
    if (messyPrinting) {
        _tprintf(_T("%s"), buf);
    } else {
        if (old == NULL) {
            tc->message = CuTcsCopy(buf);
        } else {
            tc->message = CuTcsCat(old, buf);
            free(old);
        }
    }
}

void CuMessage(CuTest* tc, const TCHAR* format, ...) {
    va_list argp;
    va_start(argp, format);
    CuMessageV(tc, format, argp);
    va_end(argp);
}

void CuMessageA(CuTest* tc, const char* format, ...) {
    va_list argp;
    char buf[HUGE_STRING_LEN];
    TCHAR tbuf[HUGE_STRING_LEN];
    va_start(argp, format);
    vsprintf(buf, format, argp);
    va_end(argp);

    TCHAR* old = tc->message;
    STRCPY_AtoT(tbuf, buf, HUGE_STRING_LEN);
    if (messyPrinting) {
        _tprintf(_T("%s"), buf);
    } else {
        if (old == NULL) {
            tc->message = CuTcsCopy(tbuf);
        } else {
            tc->message = CuTcsCat(old, tbuf);
            free(old);
        }
    }
}

void CuAssert(CuTest* tc, const TCHAR* message, int condition) {
    if (condition) return;
    CuFail(tc, message);
}

void CuAssertTrue(CuTest* tc, int condition) {
    if (condition) return;
    CuFail(tc, _T("assert failed"));
}

void CuAssertStrEquals(CuTest* tc, const TCHAR* preMessage, const TCHAR* expected, const TCHAR* actual) {
    CuString* message;
    if (_tcscmp(expected, actual) == 0) return;
    message = CuStringNew();
    CuStringAppend(message, preMessage);
    CuStringAppend(message, _T(" : "));
    CuStringAppend(message, _T("expected\n---->\n"));
    CuStringAppend(message, expected);
    CuStringAppend(message, _T("\n<----\nbut saw\n---->\n"));
    CuStringAppend(message, actual);
    CuStringAppend(message, _T("\n<----"));
    CuFail(tc, message->buffer);
    CuStringFree(message);
}

void CuAssertIntEquals(CuTest* tc, const TCHAR* preMessage, int expected, int actual) {
    TCHAR buf[STRING_MAX];
    if (expected == actual) return;
    _sntprintf(buf, STRING_MAX, _T("%s : expected <%d> but was <%d>"), preMessage, expected, actual);
    CuFail(tc, buf);
}

void CuAssertPtrEquals(CuTest* tc, const TCHAR* preMessage, const void* expected, const void* actual) {
    TCHAR buf[STRING_MAX];
    if (expected == actual) return;
    _sntprintf(buf, STRING_MAX, _T("%s : expected pointer <%p> but was <%p>"), preMessage, expected, actual);
    CuFail(tc, buf);
}

void CuAssertPtrNotNull(CuTest* tc, const TCHAR* preMessage, const void* pointer) {
    TCHAR buf[STRING_MAX];
    if (pointer != NULL) return;
    _sntprintf(buf, STRING_MAX, _T("%s : null pointer unexpected, but was <%p>"), preMessage, pointer);
    CuFail(tc, buf);
}

void CuTestRun(CuTest* tc) {
    //	jmp_buf buf;
    //	tc->jumpBuf = &buf;
    //	if (setjmp(buf) == 0)
    //	{
    tc->ran = 1;
    (tc->function)(tc);
    //	}
    //	tc->jumpBuf = 0;
}

/*-------------------------------------------------------------------------*
 * CuSuite
 *-------------------------------------------------------------------------*/

void CuSuiteInit(CuSuite* testSuite, const TCHAR *name) {
    testSuite->name = NULL;
    CU_TDUP(testSuite->name, name);
    testSuite->count = 0;
    testSuite->failCount = 0;
    testSuite->notimplCount = 0;
    testSuite->timeTaken = 0;
}

CuSuite* CuSuiteNew(const TCHAR *name) {
    CuSuite* testSuite = CU_ALLOC(CuSuite);
    CuSuiteInit(testSuite, name);
    return testSuite;
}

void CuSuiteDelete(CuSuite* suite) {
    free(suite->name);
    for (int i = 0; i < suite->count; i++) {
        CuTestDelete(suite->list[i]);
    }
    free(suite);
}

void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase) {
    assert(testSuite->count < MAX_TEST_CASES);
    testSuite->list[testSuite->count] = testCase;
    testSuite->count++;
}

void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2) {
    int i;
    for (i = 0; i < testSuite2->count; ++i) {
        CuTest* testCase = testSuite2->list[i];
        CuSuiteAdd(testSuite, testCase);
    }
}

void CuSuiteRun(CuSuite* testSuite) {
    int i;
    uint64_t start = Misc::currentTimeMillis();
    for (i = 0; i < testSuite->count; ++i) {
        CuTest* testCase = testSuite->list[i];
        try {
            CuTestRun(testCase);
        } catch (CLuceneError& err) {
            testCase->failed = 1;
            CuMessage(testCase, err.twhat());
        }
        testSuite->timeTaken = Misc::currentTimeMillis() - start;
        if (testCase->failed) {
            testSuite->failCount += 1;
        }
        if (testCase->notimpl) {
            testSuite->notimplCount += 1;
        }
    }
}

void CuSuiteSummary(CuSuite* testSuite, CuString* summary, bool times) {
    int i;
    for (i = 0; i < testSuite->count; ++i) {
        CuTest* testCase = testSuite->list[i];
        CuStringAppend(summary, testCase->failed ? _T("F") :
                testCase->notimpl ? _T("N") : _T("."));
    }
    if (times) {
        int bufferLen = 25 - summary->length - 10;
        for (int i = 0; i < bufferLen; i++)
            CuStringAppend(summary, _T(" "));
        CuStringAppendFormat(summary, _T(" - %dms"), testSuite->timeTaken);
    }
    CuStringAppend(summary, _T("\n"));
}

void CuSuiteOverView(CuSuite* testSuite, CuString* details) {
    CuStringAppendFormat(details, _T("%d %s run:  %d passed, %d failed, ")
    _T("%d not implemented.\n"),
            testSuite->count,
            testSuite->count == 1 ? "test" : "tests",
            testSuite->count - testSuite->failCount -
            testSuite->notimplCount,
            testSuite->failCount, testSuite->notimplCount);
}

void CuSuiteDetails(CuSuite* testSuite, CuString* details) {
    int i;
    int failCount = 0;

    if (testSuite->failCount != 0 && verbose) {
        CuStringAppendFormat(details, _T("\nFailed tests in %s:\n"), testSuite->name);
        for (i = 0; i < testSuite->count; ++i) {
            CuTest* testCase = testSuite->list[i];
            if (testCase->failed) {
                failCount++;
                CuStringAppendFormat(details, _T("%d) %s: %s\n"),
                        failCount, testCase->name, testCase->message);
            }
        }
    }
    if (testSuite->notimplCount != 0 && verbose) {
        CuStringAppendFormat(details, _T("\nNot Implemented tests in %s:\n"), testSuite->name);
        for (i = 0; i < testSuite->count; ++i) {
            CuTest* testCase = testSuite->list[i];
            if (testCase->notimpl) {
                failCount++;
                CuStringAppendFormat(details, _T("%d) %s: %s\n"),
                        failCount, testCase->name, testCase->message);
            }
        }
    }
}

/*-------------------------------------------------------------------------*
 * CuSuiteList
 *-------------------------------------------------------------------------*/

CuSuiteList* CuSuiteListNew(const TCHAR *name) {
    CuSuiteList* testSuite = CU_ALLOC(CuSuiteList);
    testSuite->name = NULL;
    CU_TDUP(testSuite->name, name);
    testSuite->count = 0;
    return testSuite;
}

void CuSuiteListDelete(CuSuiteList* lst) {
    free(lst->name);
    for (int i = 0; i < lst->count; i++) {
        CuSuiteDelete(lst->list[i]);
    }
    free(lst);
}

void CuSuiteListAdd(CuSuiteList *suites, CuSuite *origsuite) {
    assert(suites->count < MAX_TEST_CASES);
    suites->list[suites->count] = origsuite;
    suites->count++;
}

void CuSuiteListRun(CuSuiteList* testSuite) {
    int i;
    for (i = 0; i < testSuite->count; ++i) {
        CuSuite* testCase = testSuite->list[i];
        CuSuiteRun(testCase);
    }
}

static const TCHAR *genspaces(int i) {
    TCHAR *str = (TCHAR*) malloc((i + 1) * sizeof (TCHAR));
    for (int j = 0; j < i; j++)
        str[j] = _T(' ');
    str[i] = '\0';
    return str;
}

void CuSuiteListRunWithSummary(CuSuiteList* testSuite, bool verbose, bool times) {
    int i;

    _tprintf(_T("%s:\n"), testSuite->name);
    for (i = 0; i < testSuite->count; ++i) {
        bool hasprinted = false;
        CuSuite* testCase = testSuite->list[i];
        CuString *str = CuStringNew();

        size_t len = _tcslen(testCase->name);
        const TCHAR* spaces = len > 31 ? NULL : genspaces(31 - len);
        _tprintf(_T("    %s:%s"), testCase->name, len > 31 ? _T("") : spaces);
        free((void*) spaces);
        fflush(stdout);

        CuSuiteRun(testCase);
        if (verbose) {
            for (int i = 0; i < testCase->count; i++) {
                if (testCase->list[i]->ran) {
                    if (testCase->list[i]->message != NULL) {
                        if (!hasprinted)
                            printf("\n");
                        _tprintf(_T("      %s:\n"), testCase->list[i]->name);

                        TCHAR* msg = testCase->list[i]->message;
                        bool nl = true;
                        //write out message, indenting on new lines
                        while (*msg != '\0') {
                            if (nl) {
                                printf("        ");
                                nl = false;
                            }
                            if (*msg == '\n')
                                nl = true;
                            putc(*msg, stdout);

                            msg++;
                        }

                        if (testCase->list[i]->message[_tcslen(testCase->list[i]->message) - 1] != '\n')
                            printf("\n");
                        hasprinted = true;
                    }
                }
            }
        }
        CuSuiteSummary(testCase, str, times);
        if (hasprinted)
            _tprintf(_T("    Result: %s\n"), str->buffer);
        else
            _tprintf(_T(" %s"), str->buffer);

        CuStringFree(str);
    }
    _tprintf(_T("\n"));
}

int CuSuiteListDetails(CuSuiteList* testSuite, CuString* details) {
    int i;
    int failCount = 0;
    int notImplCount = 0;
    int count = 0;

    for (i = 0; i < testSuite->count; ++i) {
        failCount += testSuite->list[i]->failCount;
        notImplCount += testSuite->list[i]->notimplCount;
        count += testSuite->list[i]->count;
    }
    CuStringAppendFormat(details, _T("%d %s run:  %d passed, %d failed, ")
    _T("%d not implemented.\n"),
            count,
            count == 1 ? _T("test") : _T("tests"),
            count - failCount - notImplCount,
            failCount, notImplCount);

    if (failCount != 0 && verbose) {
        for (i = 0; i < testSuite->count; ++i) {
            CuString *str = CuStringNew();
            CuSuite* testCase = testSuite->list[i];
            if (testCase->failCount) {
                CuSuiteDetails(testCase, str);
                CuStringAppend(details, str->buffer);
            }
            CuStringFree(str);
        }
    }
    if (notImplCount != 0 && verbose) {
        for (i = 0; i < testSuite->count; ++i) {
            CuString *str = CuStringNew();
            CuSuite* testCase = testSuite->list[i];
            if (testCase->notimplCount) {
                CuSuiteDetails(testCase, str);
                CuStringAppend(details, str->buffer);
            }
            CuStringFree(str);
        }
    }
    return failCount;
}

