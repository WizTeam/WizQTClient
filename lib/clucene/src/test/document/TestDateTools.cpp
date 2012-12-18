/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

void assertDateTimeEquals(CuTest* tc, const TCHAR* isoFormat, int64_t d){
    TCHAR* tmp = DateTools::getISOFormat(d);
    int res = _tcscmp(isoFormat, tmp);
    _CLDELETE_LCARRAY(tmp);
    CLUCENE_ASSERT(res == 0);
}

void testStringToDate(CuTest *tc) {
    int64_t d = DateTools::stringToTime(_T("2004"));
    assertDateTimeEquals(tc, _T("2004-01-01 00:00:00:000"), d);
    d = DateTools::stringToTime(_T("20040705"));
    assertDateTimeEquals(tc, _T("2004-07-05 00:00:00:000"), d);
    d = DateTools::stringToTime(_T("200407050910"));
    assertDateTimeEquals(tc, _T("2004-07-05 09:10:00:000"), d);
    d = DateTools::stringToTime(_T("20040705091055990"));
    assertDateTimeEquals(tc, _T("2004-07-05 09:10:55:990"), d);

    try {
        d = DateTools::stringToTime(_T("97"));    // no date
        CuFail(tc, _T("Expected an exception on DateTools::stringToTime(\"97\")"));
    } catch(...) { /* expected exception */ }
    
    try {
        d = DateTools::stringToTime(_T("200401011235009999"));    // no date
        CuFail(tc, _T("Expected an exception on DateTools::stringToTime(\"200401011235009999\")"));
    } catch(...) { /* expected exception */ }
    
    try {
        d = DateTools::stringToTime(_T("aaaa"));    // no date
        CuFail(tc, _T("Expected an exception on DateTools::stringToTime(\"aaaa\")"));
    } catch(...) { /* expected exception */ }
}

void testDateAndTimetoString(CuTest* tc) {
    int64_t cal = DateTools::getTime(2004, 2, 3, 22, 8, 56, 333);
    TCHAR* dateString;

    dateString = DateTools::timeToString(cal, DateTools::YEAR_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("2004"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-01-01 00:00:00:000"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    dateString = DateTools::timeToString(cal, DateTools::MONTH_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("200402"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-02-01 00:00:00:000"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    dateString = DateTools::timeToString(cal, DateTools::DAY_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("20040203"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-02-03 00:00:00:000"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    dateString = DateTools::timeToString(cal, DateTools::HOUR_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("2004020322"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-02-03 22:00:00:000"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    dateString = DateTools::timeToString(cal, DateTools::MINUTE_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("200402032208"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-02-03 22:08:00:000"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    dateString = DateTools::timeToString(cal, DateTools::SECOND_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("20040203220856"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-02-03 22:08:56:000"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    dateString = DateTools::timeToString(cal, DateTools::MILLISECOND_FORMAT);
    CLUCENE_ASSERT( _tcscmp(_T("20040203220856333"), dateString) == 0 );
    assertDateTimeEquals(tc, _T("2004-02-03 22:08:56:333"), DateTools::stringToTime(dateString));
    _CLDELETE_LCARRAY(dateString);

    /*
    // TODO: Allow this

    // date before 1970:
    cal.set(1961, 2, 5,   // year=1961, month=march(!), day=5
        23, 9, 51);       // hour, minute, second
    cal.set(Calendar.MILLISECOND, 444);
    dateString = DateTools.dateToString(cal.getTime(), DateTools.Resolution.MILLISECOND);
    assertDateTimeEquals("19610305230951444", dateString);
    assertDateTimeEquals("1961-03-05 23:09:51:444", isoFormat(DateTools.stringToDate(dateString)));

    dateString = DateTools.dateToString(cal.getTime(), DateTools.Resolution.HOUR);
    assertDateTimeEquals("1961030523", dateString);
    assertDateTimeEquals("1961-03-05 23:00:00:000", isoFormat(DateTools.stringToDate(dateString)));

    // timeToString:
    cal.set(1970, 0, 1, // year=1970, month=january, day=1
        0, 0, 0); // hour, minute, second
    cal.set(Calendar.MILLISECOND, 0);
    dateString = DateTools.timeToString(cal.getTime().getTime(),
        DateTools.Resolution.MILLISECOND);
    assertDateTimeEquals("19700101000000000", dateString);

    cal.set(1970, 0, 1, // year=1970, month=january, day=1
        1, 2, 3); // hour, minute, second
    cal.set(Calendar.MILLISECOND, 0);
    dateString = DateTools.timeToString(cal.getTime().getTime(),
        DateTools.Resolution.MILLISECOND);
    assertDateTimeEquals("19700101010203000", dateString);
    */
} 

CuSuite *testDateTools(void)
{
    CuSuite *suite = CuSuiteNew(_T("CLucene DateTools Test"));

    SUITE_ADD_TEST(suite, testStringToDate);
    // SUITE_ADD_TEST(suite, testStringtoTime); -- tests against a calendar object - irrelevant to clucene
    SUITE_ADD_TEST(suite, testDateAndTimetoString);

    return suite;
}
