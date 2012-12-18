/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#pragma once

int BenchmarkDocumentWriter(Timer*);
int BenchmarkTermDocs(Timer* timerCase);

class TestCLString:public Unit
{
protected:
	void runTests(){
		this->runTest("BenchmarkDocumentWriter",BenchmarkDocumentWriter,10);
		//this->runTest("BenchmarkTermDocs",BenchmarkTermDocs,100);
	}
public:
	const char* getName(){
		return "TestCLString";
	}
};
