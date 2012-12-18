/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#pragma once

class Benchmarker
{
	lucene::util::CLVector<Unit*> tests;
public:
	Timer timerTotal;
	int testsCountTotal;
	int testsCountSuccess;
	int testsRunTotal;
	int testsRunSuccess;

	Benchmarker(void);
	void Add(Unit* unit);
	bool run();
	void reset();
};
