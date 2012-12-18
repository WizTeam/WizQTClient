/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef TEST_H
#define TEST_H
#include "CLucene.h"
#include "CLucene/_clucene-config.h"
#include "CLucene/config/repl_tchar.h"
#include "CLucene/config/repl_wchar.h"
#include "CLucene/debug/_condition.h"
#include "CLucene/util/StringBuffer.h"
#include "CLucene/util/Misc.h"

#include "CLucene/store/RAMDirectory.h"
#include "CLucene/store/Lock.h"
#include "CLucene/index/TermVector.h"
#include "CLucene/queryParser/MultiFieldQueryParser.h"

#include <stdio.h>
#include <iostream>
#include <string.h>
#include <stdio.h>

using namespace std;

#define LUCENE_INT64_MAX_SHOULDBE _ILONGLONG(0x7FFFFFFFFFFFFFFF)
#define LUCENE_INT64_MIN_SHOULDBE (-LUCENE_INT64_MAX_SHOULDBE - _ILONGLONG(1) )

CL_NS_USE(index)
CL_NS_USE(util)
CL_NS_USE(store)
CL_NS_USE(search)
CL_NS_USE(document)
CL_NS_USE(queryParser)
CL_NS_USE(analysis)
CL_NS_USE2(analysis,standard)

#include "CuTest.h"

CuSuite *testatomicupdates(void);
CuSuite *testRAMDirectory(void);
CuSuite *testindexwriter(void);
CuSuite *testIndexModifier(void);
CuSuite *testindexreader(void);
CuSuite *testIndexSearcher(void);
CuSuite *testAddIndexesNoOptimize(void);
CuSuite *teststore(void);
CuSuite *testanalysis(void);
CuSuite *testanalyzers(void);
CuSuite *testhighfreq(void);
CuSuite *testhighlight(void);
CuSuite *testpriorityqueue(void);
CuSuite *testQueryParser(void);
CuSuite *testMultiFieldQueryParser(void);
CuSuite *testqueries(void);
CuSuite *testConstantScoreQueries(void);
CuSuite *testsearch(void);
CuSuite *testtermvector(void);
CuSuite *testsort(void);
CuSuite *testduplicates(void);
CuSuite *testRangeFilter(void);
CuSuite *testdatefilter(void);
CuSuite *testwildcard(void);
CuSuite *testdebug(void);
CuSuite *testutf8(void);
CuSuite *testreuters(void);
CuSuite *testdocument(void);
CuSuite *testField(void);
CuSuite *testNumberTools(void);
CuSuite *testDateTools(void);
CuSuite *testBoolean(void);
CuSuite *testBitSet(void);
CuSuite *testExtractTerms(void);
CuSuite *testSpanQueries(void);
CuSuite *testStringBuffer(void);
CuSuite *testTermVectorsReader(void);

#ifdef TEST_CONTRIB_LIBS
CuSuite *testGermanAnalyzer(void);
#endif

class English{
public:
    static void IntToEnglish(int32_t i, CL_NS(util)::StringBuffer* result);
    static TCHAR* IntToEnglish(int32_t i);
    static void IntToEnglish(int32_t i, TCHAR* buf, int32_t buflen);
};


class TCharCompare{
public:
	bool operator()( const TCHAR* val1, const TCHAR* val2 ) const{
		if ( val1==val2)
			return false;
		bool ret = (_tcscmp( val1,val2 ) < 0);
		return ret;
	}
};
class CharCompare{
public:
	bool operator()( const char* val1, const char* val2 ) const{
		if ( val1==val2)
			return false;
		bool ret = (strcmp( val1,val2 ) < 0);
		return ret;
	}
};

template<typename _K, typename _T, typename _Comparator=TCharCompare>
class StringMap : public std::map<_K,_T,_Comparator>{
    bool delKey;
public:
    StringMap(bool delKey){
        this->delKey = delKey;
    }

    void remove(_K val){
        std::iterator<_K,_T,_Comparator> itr;
        itr = this->find(val);
        if ( itr == this->end() )
            return;
        _K v = itr->first;
        this->erase(itr);
        if ( delKey ){
             _CLDELETE_CARRAY(v);
        }
    }
    virtual ~StringMap(){
        while ( this->begin() != this->end() ){
            _K v = this->begin()->first;
            this->erase(this->begin());
            if ( delKey ){
                _CLDELETE_CARRAY(v);
            }
        }
    }
    void add(_K k, _T v){
        this->insert ( std::pair<_K,_T>(k,v) );
    }
};

void TestAssertIndexReaderEquals(CuTest *tc,  IndexReader* reader1, IndexReader* reader2);


extern unittest tests[];

#define CLUCENE_DATA_LOCATION1 "../../src/test/data/"
#define CLUCENE_DATA_LOCATION2 "../src/test/data/"
#define CLUCENE_DATA_LOCATION3 "../../../src/test/data/"
#define CLUCENE_DATA_LOCATIONENV "srcdir"

extern const char* cl_tempDir;

#endif
