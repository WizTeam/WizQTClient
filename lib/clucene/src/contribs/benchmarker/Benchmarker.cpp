/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "Benchmarker.h"
#include "Unit.h"

void Benchmarker::Add(Unit* unit){
	tests.push_back(unit);
}
Benchmarker::Benchmarker(void)
{
	reset();
}
void Benchmarker::reset(){
	timerTotal.reset();
	testsCountTotal=0;
	testsCountSuccess=0;
	testsRunTotal=0;
	testsRunSuccess=0;
}
bool Benchmarker::run(){
	timerTotal.start();
	printf( ">> running tests...\n" );
	for ( int i=0;i<tests.size();i++ ){
		Unit* unit = tests[i];
		unit->start(this);
		unit->stop();
	}
	printf( "\n>> benchmarker ran a total of %d test cases(%d successes) in %d ms\n",
		testsCountTotal,testsCountSuccess,
		(int32_t)timerTotal.interval() );
	timerTotal.stop();

  return testsCountSuccess > 0;
}
