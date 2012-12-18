/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "Unit.h"

Unit::Unit()
{
	this->bm=NULL;
	testsCountTotal=0;
	testsCountSuccess=0;
	testsRunTotal=0;
	testsRunSuccess=0;
	timerCase.reset();
	timerTotal.reset();
}

void Unit::stop(){
	timerTotal.stop();
	bm=NULL;

	printf( "> unit ran a total of %d test cases(%d successes) in %d ms\n",
		testsCountTotal,testsCountSuccess,
		(int)timerTotal.interval() );
}

void Unit::start(Benchmarker* bm){
	this->bm = bm;
	timerTotal.start();

	printf( "> running unit %s\n", getName() );
	runTests();
}
void Unit::runTest(const char* testName,LPTEST_ROUTINE func, int iterations){
	if ( bm == NULL )
		_CLTHROWA(CL_ERR_NullPointer, "Unit not started with benchmarker!");
	float avg=0; 
	int32_t min=0;
	int32_t max=0;
	int count=0;
	Timer total;
	bool success = false;

   try {
	 total.start();
	 printf("\n > running %s %d times...", testName, iterations);
	 for ( int i=0;i<iterations;i++ ){
		  timerCase.reset();
		  success = (func(&timerCase) == 0 );
		  int32_t t = timerCase.stop();
		  if ( count == 0 ){
			 min = t;
			 max = t;
			 avg = t;
		  }else{
			 if ( t < min )
				  min = t;
			 if ( t > max )
				  max = t;
			 avg = (avg + t)/2;
		  }

		  testsRunTotal++;
		  bm->testsRunTotal++;
		  if ( success ){
			 testsRunSuccess++;
			 bm->testsRunSuccess++;
		  }
		  count++;
	 }
	 success = true;
   }catch(CLuceneError& err){
	 printf("\n > error occurred: %s\n", err.what());
   }catch(...){
	 printf("\n > unexpected error occurred\n >");
   }
	testsCountTotal++;
	bm->testsCountTotal++;
	if ( success ){
		testsCountSuccess++;
		bm->testsCountSuccess++;
	}
	printf(" it took %d milliseconds",total.stop());

	if ( iterations > 1 ){
		printf("\n\tmin:%d",min);
		printf(" max:%d,",max);
		printf(" avg:%0.3f milliseconds",avg);
	}
	printf("\n");
}
