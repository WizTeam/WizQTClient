/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "CLucene/analysis/cjk/CJKAnalyzer.h"
#include "CLucene/analysis/LanguageBasedAnalyzer.h"
#include "CLucene/snowball/SnowballFilter.h"

#include <fcntl.h>
#ifdef _CL_HAVE_IO_H
#include <io.h>
#endif
#ifdef _CL_HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef _CL_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef _CL_HAVE_DIRECT_H
#include <direct.h>
#endif
#include <errno.h>

CL_NS_USE2(analysis, cjk)
CL_NS_USE2(analysis, snowball)

void test(CuTest *tc, char* orig, Reader* reader, bool verbose, int64_t bytes) {
    StandardAnalyzer analyzer;
    TokenStream* stream = analyzer.tokenStream(NULL, reader);

    uint64_t start = Misc::currentTimeMillis();

    int32_t count = 0;
    Token t;
    char atmp[LUCENE_MAX_WORD_LEN + 1];
    TCHAR ttmp[LUCENE_MAX_WORD_LEN + 1];
    for (; stream->next(&t);) {
        if (verbose) {
            CuMessage(tc, _T("Text=%s start=%d end=%d\n"), t.termBuffer(), t.startOffset(), t.endOffset());
        }
        int len = t.termLength();

        //use the lucene strlwr function (so copy to TCHAR first then back)
        strncpy(atmp, orig + t.startOffset(), len);
        atmp[len] = 0;
        STRCPY_AtoT(ttmp, atmp, len + 1);
        _tcslwr(ttmp);

        if (_tcsncmp(t.termBuffer(), ttmp, len) != 0) {
            TCHAR err[1024];
            _sntprintf(err, 1024, _T("token '%s' didnt match original text at %d-%d"), t.termBuffer(), t.startOffset(), t.endOffset());
            CuAssert(tc, err, false);
        }

        // _CLDELETE(t);
        count++;
    }

    uint64_t end = Misc::currentTimeMillis();
    int64_t time = end - start;
    CuMessageA(tc, "%d milliseconds to extract ", time);
    CuMessageA(tc, "%d tokens\n", count);
    CuMessageA(tc, "%f microseconds/token\n", (time * 1000.0) / count);
    CuMessageA(tc, "%f megabytes/hour\n", (bytes * 1000.0 * 60.0 * 60.0) / (time * 1000000.0));

    _CLDELETE(stream);
}

void _testFile(CuTest *tc, const char* fname, bool verbose) {
    struct fileStat buf;
    fileStat(fname, &buf);
    int64_t bytes = buf.st_size;

    char* orig = _CL_NEWARRAY(char, bytes);
    {
        FILE* f = fopen(fname, "rb");
        int64_t r = fread(orig, bytes, 1, f);
        fclose(f);
    }

    CuMessageA(tc, " Reading test file containing %d bytes.\n", bytes);
    jstreams::FileReader fr(fname, "ASCII");
    const TCHAR *start;
    size_t total = 0;
    int32_t numRead;
    do {
        numRead = fr.read(start, 1, 0);
        if (numRead == -1)
            break;
        total += numRead;
    } while (numRead >= 0);

    jstreams::FileReader reader(fname, "ASCII");

    test(tc, orig, &reader, verbose, total);

    _CLDELETE_CaARRAY(orig);
}

void testFile(CuTest *tc) {
    char loc[1024];
    strcpy(loc, clucene_data_location);
    strcat(loc, "/reuters-21578/feldman-cia-worldfactbook-data.txt");
    CuAssert(tc, _T("reuters-21578/feldman-cia-worldfactbook-data.txt does not exist"), Misc::dir_Exists(loc));

    _testFile(tc, loc, false);
}

void _testCJK(CuTest *tc, const char* astr, const char** results, bool ignoreSurrogates = true) {
    SimpleInputStreamReader r(new AStringReader(astr), SimpleInputStreamReader::UTF8);

    CJKTokenizer* tokenizer = _CLNEW CJKTokenizer(&r);
    tokenizer->setIgnoreSurrogates(ignoreSurrogates);
    int pos = 0;
    Token tok;
    TCHAR tres[LUCENE_MAX_WORD_LEN];

    while (results[pos] != NULL) {
        CLUCENE_ASSERT(tokenizer->next(&tok) != NULL);

        lucene_utf8towcs(tres, results[pos], LUCENE_MAX_WORD_LEN);
        CuAssertStrEquals(tc, _T("unexpected token value"), tres, tok.termBuffer());

        pos++;
    }
    CLUCENE_ASSERT(!tokenizer->next(&tok));

    _CLDELETE(tokenizer);
}

void testCJK(CuTest *tc) {
    //utf16 test
    //we have a very large unicode character:
    //xEFFFF = utf8(F3 AF BF BF) = utf16(DB7F DFFF) = utf8(ED AD BF, ED BF BF)
    static const char* exp3[4] = {"\xED\xAD\xBF\xED\xBF\xBF\xe5\x95\xa4", "\xe5\x95\xa4\xED\xAD\xBF\xED\xBF\xBF", "", NULL};
    _testCJK(tc, "\xED\xAD\xBF\xED\xBF\xBF\xe5\x95\xa4\xED\xAD\xBF\xED\xBF\xBF", exp3, false);

    static const char* exp1[5] = {"test", "t\xc3\xbcrm", "values", NULL};
    _testCJK(tc, "test t\xc3\xbcrm values", exp1);

    static const char* exp2[6] = {"a", "\xe5\x95\xa4\xe9\x85\x92", "\xe9\x85\x92\xe5\x95\xa4", "", "x", NULL};
    _testCJK(tc, "a\xe5\x95\xa4\xe9\x85\x92\xe5\x95\xa4x", exp2);
}

void testLanguageBasedAnalyzer(CuTest* tc) {
    LanguageBasedAnalyzer a;
    CL_NS(util)::StringReader reader(_T("he abhorred accentueren"));
    reader.mark(50);
    TokenStream* ts;
    Token t;

    //test with english
    a.setLanguage(_T("English"));
    a.setStem(false);
    ts = a.tokenStream(_T("contents"), &reader);

    CLUCENE_ASSERT(ts->next(&t) != NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("he")) == 0);
    CLUCENE_ASSERT(ts->next(&t) != NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("abhorred")) == 0);
    _CLDELETE(ts);

    //now test with dutch
    reader.reset(0);
    a.setLanguage(_T("Dutch"));
    a.setStem(true);
    ts = a.tokenStream(_T("contents"), &reader);

    CLUCENE_ASSERT(ts->next(&t) != NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("he")) == 0);
    CLUCENE_ASSERT(ts->next(&t) != NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("abhorred")) == 0);
    CLUCENE_ASSERT(ts->next(&t) != NULL);
    CLUCENE_ASSERT(_tcscmp(t.termBuffer(), _T("accentuer")) == 0);
    _CLDELETE(ts);
}

CuSuite *testanalysis(void) {
    CuSuite *suite = CuSuiteNew(_T("CLucene Analysis Test"));

    SUITE_ADD_TEST(suite, testFile);
    SUITE_ADD_TEST(suite, testCJK);
    SUITE_ADD_TEST(suite, testLanguageBasedAnalyzer);

    return suite;
}
// EOF
