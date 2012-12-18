/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "test.h"
#include "CLucene/util/StringBuffer.h"
#include <stdexcept>

CL_NS_USE(util)

void testStringBufferConstruct(CuTest *tc) {
  StringBuffer sb;
  CuAssertEquals(tc, 0, sb.length());

  StringBuffer sb1(10);
  CuAssertEquals(tc, 0, sb1.length());

  StringBuffer sb2(_T("test"));
  CuAssertEquals(tc, 4, sb2.length());
  CuAssertStrEquals(tc, _T(""), _T("test"), sb2.getBuffer());
}

void testStringBufferCharAt(CuTest *tc) {
  StringBuffer sb(_T("test abcd"));

  CuAssertTrue(tc, sb.charAt(0) == _T('t'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(1) == _T('e'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(2) == _T('s'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(3) == _T('t'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(4) == _T(' '), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(5) == _T('a'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(6) == _T('b'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(7) == _T('c'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(8) == _T('d'), _T("unexpected character"));

  sb.setCharAt(4, _T('-'));

  CuAssertTrue(tc, sb.charAt(0) == _T('t'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(1) == _T('e'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(2) == _T('s'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(3) == _T('t'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(4) == _T('-'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(5) == _T('a'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(6) == _T('b'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(7) == _T('c'), _T("unexpected character"));
  CuAssertTrue(tc, sb.charAt(8) == _T('d'), _T("unexpected character"));
}

void testStringBufferClear(CuTest *tc) {
  StringBuffer sb(_T("test abcd wxyz"));
  sb.clear();
  CuAssertEquals(tc, 0, sb.length());
  CuAssertStrEquals(tc, _T(""), _T("\0"), sb.getBuffer());
}

void testStringBufferInsert(CuTest *tc) {
  StringBuffer sb(_T("test abcd"));

  sb.insert(5, _T('X'));
  CuAssertStrEquals(tc, _T(""), _T("test Xabcd"), sb.getBuffer());

  sb.insert(7, _T("YY"));
  CuAssertStrEquals(tc, _T(""), _T("test XaYYbcd"), sb.getBuffer());

  sb.insert(7, _T(""));
  CuAssertStrEquals(tc, _T(""), _T("test XaYYbcd"), sb.getBuffer());

  sb.insert(12, _T("ZZZ"));
  CuAssertStrEquals(tc, _T(""), _T("test XaYYbcdZZZ"), sb.getBuffer());

  sb.insert(15, _T('_'));
  CuAssertStrEquals(tc, _T(""), _T("test XaYYbcdZZZ_"), sb.getBuffer());

  sb.insert(0, _T('_'));
  CuAssertStrEquals(tc, _T(""), _T("_test XaYYbcdZZZ_"), sb.getBuffer());

  sb.insert(0, _T("123"));
  CuAssertStrEquals(tc, _T(""), _T("123_test XaYYbcdZZZ_"), sb.getBuffer());
}

void testStringBufferDelete(CuTest *tc) {
  StringBuffer sb(_T("test abcd"));

  sb.deleteCharAt(4);
  CuAssertStrEquals(tc, _T(""), _T("testabcd"), sb.getBuffer());

  sb.deleteChars(4, 7);
  CuAssertStrEquals(tc, _T(""), _T("testd"), sb.getBuffer());

  sb.deleteChars(3, 3);
  CuAssertStrEquals(tc, _T(""), _T("testd"), sb.getBuffer());
}

void testSubstringEquals(CuTest *tc) {
  StringBuffer sb(_T("test abcd"));

  CuAssertTrue(tc, sb.substringEquals(3, 6, _T("t a")));
  CuAssertTrue(tc, sb.substringEquals(3, 6, _T("t a"), 3));
  CuAssertTrue(tc, !sb.substringEquals(3, 6, _T("t-a"), 3));
}

CuSuite *testStringBuffer(void) {
    CuSuite *suite = CuSuiteNew(_T("CLucene StringBuffer Test"));

    SUITE_ADD_TEST(suite, testStringBufferConstruct);
    SUITE_ADD_TEST(suite, testStringBufferCharAt);
    SUITE_ADD_TEST(suite, testStringBufferClear);
    SUITE_ADD_TEST(suite, testStringBufferInsert);
    SUITE_ADD_TEST(suite, testStringBufferDelete);
    SUITE_ADD_TEST(suite, testSubstringEquals);

    return suite;
}
