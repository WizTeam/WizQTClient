/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "_TermInfo.h"


CL_NS_DEF(index)

TermInfo::TermInfo(){
//Func - Constructor
//Pre  - true
//Post - Instance has been created
			
	docFreq     = 0;
	freqPointer = 0;
	proxPointer = 0;
  skipOffset = 0;
}

TermInfo::~TermInfo(){
//Func - Destructor.
//Pre  - true
//Post - Instance has been destroyed
}

TermInfo::TermInfo(const int32_t df, const int64_t fp, const int64_t pp){
//Func - Constructor. 
//Pre  - df >= 0, fp >= 0 pp >= 0
//Post - An instance has been created with FreqPointer = fp, proxPointer=pp and docFreq= df

    CND_PRECONDITION(df >= 0, "df contains negative number");
    CND_PRECONDITION(fp >= 0, "fp contains negative number");
    CND_PRECONDITION(pp >= 0, "pp contains negative number");

    freqPointer = fp;
    proxPointer = pp;
	  docFreq     = df;
    skipOffset = 0;
}

TermInfo::TermInfo(const TermInfo* ti) {
//Func - Constructor. 
//       Initialises this instance by copying the values of another TermInfo ti
//Pre  - ti is a reference to another TermInfo
//       ti->docFreq >= 0
//       ti->freqPointer >= 0
//       ti->proxPointer >= 0
//Post - Values of ti have been copied to the values of this Instance.

    CND_PRECONDITION(ti->docFreq     >= 0, "ti->docFreq contains negative number");
    CND_PRECONDITION(ti->freqPointer >= 0, "ti->freqPointer contains negative number");
    CND_PRECONDITION(ti->proxPointer >= 0, "ti->proxPointer contains negative number");

	docFreq     = ti->docFreq;
	freqPointer = ti->freqPointer;
	proxPointer = ti->proxPointer;
  skipOffset  = ti->skipOffset;
}

void TermInfo::set(const int32_t df, const int64_t fp, const int64_t pp, int32_t so) {
//Func - Sets a new document frequency, a new freqPointer and a new proxPointer
//Pre  - df >= 0, fp >= 0 pp >= 0
//Post - The new document frequency, a new freqPointer and a new proxPointer
//       have been set

    CND_PRECONDITION(df >= 0, "df contains negative number");
    CND_PRECONDITION(fp >= 0, "fp contains negative number");
    CND_PRECONDITION(pp >= 0, "pp contains negative number");

	docFreq     = df;
	freqPointer = fp;
	proxPointer = pp;
    skipOffset  = so;
}

void TermInfo::set(const TermInfo* ti) {
//Func - Sets a new document frequency, a new freqPointer and a new proxPointer
//       by copying these values from another instance of TermInfo
//Pre  - ti is a reference to another TermInfo
//       ti->docFreq >= 0
//       ti->freqPointer >= 0
//       ti->proxPointer >= 0
//Post - Values of ti have been copied to the values of this Instance.

    CND_PRECONDITION(ti->docFreq     >= 0, "ti->docFreq contains negative number");
    CND_PRECONDITION(ti->freqPointer >= 0, "ti->freqPointer contains negative number");
    CND_PRECONDITION(ti->proxPointer >= 0, "ti->proxPointer contains negative number");

	docFreq     = ti->docFreq;
	freqPointer = ti->freqPointer;
	proxPointer = ti->proxPointer;
    skipOffset =  ti->skipOffset;
}
CL_NS_END
