/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "CLucene/_ApiHeader.h"
#include "Terms.h"
#include "Term.h"

CL_NS_DEF(index)

TermDocs::~TermDocs(){
}

TermEnum::~TermEnum(){
}

bool TermEnum::skipTo(Term* target){
	do {
		if (!next())
			return false;
	} while (target->compareTo(term(false)) > 0);
	return true;
}

TermPositions::~TermPositions(){
}

CL_NS_END
