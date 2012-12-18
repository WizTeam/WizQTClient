/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

	class TestTermInfo: LUCENE_BASE {
	public:
      int32_t docFreq;
      Term* term;
      
      TestTermInfo(Term* t, int32_t df) {
        term = _CL_POINTER(t);
        docFreq = df;
      }
      ~TestTermInfo(){
         _CLDECDELETE(term);
      }
    };

	class TermInfoQueue: public PriorityQueue<TestTermInfo*,CL_NS(util)::Deletor::Object<TestTermInfo> > {
	public:
        TermInfoQueue(int32_t size) {
            initialize(size,true);
        }
        bool lessThan(TestTermInfo* A, TestTermInfo* B) {
            return A->docFreq < B->docFreq;
          
        }
    };

  void _TestHighFreqTerms(const char* index, size_t numTerms) {

		IndexReader* reader = IndexReader::open(index);
    
        TermInfoQueue* tiq = _CLNEW TermInfoQueue(100);
		TermEnum* terms = reader->terms();
        int32_t c=0;
        int32_t minFreq = 0;
        while (terms->next()) {
          if (terms->docFreq() > minFreq) {
            Term* term = terms->term(false);
			tiq->put(_CLNEW TestTermInfo(term, terms->docFreq()));
            c++;
            if (tiq->size() >= numTerms) {		  // if tiq overfull
               TestTermInfo* tti=tiq->pop();
               _CLLDELETE(tti);				  // remove lowest in tiq
               c--;
               minFreq = ((TestTermInfo*)tiq->top())->docFreq; // reset minFreq
            }
          }
        }
    
        while (tiq->size() != 0) {
          TestTermInfo* termInfo = (TestTermInfo*)tiq->pop();
          _CLLDELETE(termInfo);
          c--;
        }

        terms->close();
        _CLDELETE(terms);
        _CLDELETE(tiq);

        reader->close();
        _CLDELETE( reader );

		//CuMessageA(tc,"%d milliseconds\n",(int32_t)(Misc::currentTimeMillis()-start));
  }
  void TestHighFreqTerms(CuTest *tc){
	char loc[1024];
	strcpy(loc, clucene_data_location);
	strcat(loc, "/reuters-21578-index");

	CuAssert(tc,_T("Index does not exist"),Misc::dir_Exists(loc));
  	_TestHighFreqTerms(loc,100);
  }

CuSuite *testhighfreq(void)
{
	CuSuite *suite = CuSuiteNew(_T("CLucene High Frequencies Test"));

    SUITE_ADD_TEST(suite, TestHighFreqTerms);
    return suite; 
}
