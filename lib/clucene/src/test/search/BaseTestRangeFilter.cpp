/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include "BaseTestRangeFilter.h"

#include <limits>

static const size_t getMaxIntLength()
{
    TCHAR buf[40];
    _itot(std::numeric_limits<int>::max(), buf, 10);
    return _tcslen(buf);
}

BaseTestRangeFilter::BaseTestRangeFilter(CuTest* _tc)
        : index(new RAMDirectory())
        , maxR(std::numeric_limits<int>::min()), minR(std::numeric_limits<int>::max())
        , minId(0), maxId(10000), intLength(getMaxIntLength())
        , tc(_tc)
{
    srand ( 101 ); // use a set seed to test is deterministic
    build();
}

BaseTestRangeFilter::~BaseTestRangeFilter()
{
    index->close();
    _CLLDECDELETE(index);
}
    
std::tstring BaseTestRangeFilter::pad(int n)
{
    std::tstring b;
    if (n < 0) {
        b = _T("-");
        n = std::numeric_limits<int>::max() + n + 1;
    }
    else
        b = _T("0");

    TCHAR buf[40];
    _itot(n, buf, 10);
    for (size_t i = _tcslen(buf); i <= intLength; i++) {
        b += _T("0");
    }
    b += buf;
    
    return b;
}
    
void BaseTestRangeFilter::build()
{
    try
    {    
        /* build an index */
        SimpleAnalyzer a;
        IndexWriter* writer = _CLNEW IndexWriter(index,
                                             &a, T);

        for (int32_t d = minId; d <= maxId; d++) {
            Document doc;
            std::tstring paddedD = pad(d);
            doc.add(* _CLNEW Field(_T("id"),paddedD.c_str(), Field::STORE_YES | Field::INDEX_UNTOKENIZED));
            int r= rand();
            if (maxR < r) {
                maxR = r;
            }
            if (r < minR) {
                minR = r;
            }
            std::tstring paddedR = pad(r);
            doc.add( * _CLNEW Field(_T("rand"),paddedR.c_str(), Field::STORE_YES | Field::INDEX_UNTOKENIZED));
            doc.add( * _CLNEW Field(_T("body"),_T("body"), Field::STORE_YES | Field::INDEX_UNTOKENIZED));
            writer->addDocument(&doc);
        }
        
        writer->optimize();
        writer->close();
        _CLLDELETE(writer);
    } catch (CLuceneError&) {
        _CLTHROWA(CL_ERR_Runtime, "can't build index");
    }
}

void BaseTestRangeFilter::testPad() 
{
    const int32_t tests[] = {
        -9999999, -99560, -100, -3, -1, 0, 3, 9, 10, 1000, 999999999
    };
    const size_t testsLen = 11;

    for (size_t i = 0; i < testsLen - 1; i++) {
        const int32_t a = tests[i];
        const int32_t b = tests[i+1];
        const TCHAR* aa = pad(a).c_str();
        const TCHAR* bb = pad(b).c_str();

        StringBuffer label(50);
        label << a << _T(':') << aa << _T(" vs ") << b << _T(':') << bb;

        std::tstring tmp(_T("length of "));
        tmp += label.getBuffer();
        assertEqualsMsg(tmp.c_str(), _tcslen(aa), _tcslen(bb));

        tmp = _T("compare less than ");
        tmp += label.getBuffer();
        assertTrueMsg(tmp.c_str(), _tcscmp(aa, bb) < 0);
    }
}

// EOF
