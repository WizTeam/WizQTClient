/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"
#include <stdlib.h>
#include <time.h>

    //test string buffer
    //StringBuffer sb;
    //sb.appendFloat(0.02f,2);
    //CuAssertStrEquals(tc, _T("appendFloat failed"), _T("0.02"), sb.getBuffer());


	class integerQueue: public CL_NS(util)::PriorityQueue<int32_t,Deletor::DummyInt32 >{
	public:
		integerQueue(int32_t count)
		{ 
			initialize(count,false);
		}
	protected:
		bool lessThan(int32_t a, int32_t b) {
			return (a < b);
		}
	};

	void _test_PriorityQueue(CuTest *tc,int32_t count){
		integerQueue pq(count);
		srand ( (unsigned)time(NULL) );
		int32_t sum = 0, sum2 = 0;
	    
		uint64_t start = CL_NS(util)::Misc::currentTimeMillis();

		for (int32_t i = 0; i < count; i++) {
			int32_t next = -rand();
			//int32_t next = (count+1)-i;
			sum += next;
			pq.put( next );
		}
		CuMessageA(tc,"%d milliseconds/", (int32_t)(CL_NS(util)::Misc::currentTimeMillis()-start));
		CuMessageA(tc,"%d puts\n",count);
		start = CL_NS(util)::Misc::currentTimeMillis();

		int32_t last = -0x7FFFFFFF;
		for (int32_t j = 0; j < count; j++) {
			int32_t next = pq.pop();
			
			if ( next < last ){
				TCHAR buf[1024];
				_sntprintf(buf,1024,_T("next < last at %d (last: %d, next: %d)"),j,last,next);
				CuAssert(tc,buf,false);
			}
			last = next;
			sum2 += last;
		}

		CuMessageA(tc,"%d milliseconds", (int32_t)(CL_NS(util)::Misc::currentTimeMillis()-start));
		CuMessageA(tc,"/%d pops\n",count);

		CLUCENE_ASSERT(sum == sum);
	}
	void testPriorityQueue(CuTest *tc){
		_test_PriorityQueue(tc,100000);
	}
	

CuSuite *testpriorityqueue(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene Priority Queue Test"));

    SUITE_ADD_TEST(suite, testPriorityQueue);

    return suite; 
}
// EOF
