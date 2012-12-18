/*------------------------------------------------------------------------------
* Copyright (C) 2003-2010 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "test.h"
#include "CLucene/util/CLStreams.h"
#include "CLucene/analysis/Analyzers.h"
#include "CLucene/analysis/de/GermanStemmer.h"
#include "CLucene/analysis/de/GermanStemFilter.h"
#include "CLucene/analysis/standard/StandardTokenizer.h"

CL_NS_USE(util)
CL_NS_USE(analysis)
CL_NS_USE2(analysis,de)

  void check(CuTest* tc, const TCHAR* input, const TCHAR* expected) {
    StandardTokenizer* tokenStream = new StandardTokenizer(new StringReader(input));
    GermanStemFilter filter(tokenStream, true);
    Token t;
    if (filter.next(&t) == NULL)
      CuFail(tc, _T("Token expected!"));
    CuAssertStrEquals(tc, _T(""), expected, t.termBuffer());
    filter.close();
  }

  void testStemming(CuTest *tc) {
    try {
      // read test cases from external file:
      char path[CL_MAX_PATH];
      strcpy(path, clucene_data_location);
      strcat(path, "/contribs-lib/analysis/de/data.txt");
      CuAssert(tc, _T("File with test data does not exist"), Misc::dir_Exists(path));
      FileReader reader(path, "UTF-8");
      TCHAR buffer[1024];
      while (true) {
        int32_t len = reader.readLine(buffer, 1024);
        if (len == 0)
          break;
        Misc::wordTrim(buffer);
        if (_tcslen(buffer) == 0 || buffer[0] == _T('#'))
          continue;    // ignore comments and empty lines
        const TCHAR* pos = _tcsstr(buffer, _T(";"));
        TCHAR part0[1024], part1[1024];
        if (pos != NULL) {
          _tcsncpy(part0, buffer, pos - buffer);
          _tcscpy(part1, pos + 1);
          part0[pos - buffer] = '\0';
          check(tc, part0, part1);
        } else {
          check(tc, buffer, _T(""));
        }
      }
    } catch (CLuceneError &e) {
       CuFail(tc, e);
    }
  }

CuSuite *testGermanAnalyzer() {
  CuSuite *suite = CuSuiteNew(_T("CLucene GermanAnalyzer Test"));
  SUITE_ADD_TEST(suite, testStemming);
  return suite;
}
