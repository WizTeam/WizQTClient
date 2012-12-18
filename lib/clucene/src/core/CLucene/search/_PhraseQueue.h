/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_PhraseQueue_
#define _lucene_search_PhraseQueue_


#include "CLucene/util/PriorityQueue.h"
#include "_PhrasePositions.h"

CL_NS_DEF(search)
	class PhraseQueue: public CL_NS(util)::PriorityQueue<PhrasePositions*,
		CL_NS(util)::Deletor::Object<PhrasePositions> > {
	public:
		PhraseQueue(const int32_t size) {
			initialize(size,false);
		}
		virtual ~PhraseQueue(){
		}

	protected:
		bool lessThan(PhrasePositions* pp1, PhrasePositions* pp2) {
			if (pp1->doc == pp2->doc){
        if (pp1->position == pp2->position)
          // same doc and pp.position, so decide by actual term positions.
          // rely on: pp.position == tp.position - offset.
          return pp1->offset < pp2->offset;
        else
          return pp1->position < pp2->position;
			}else
				return pp1->doc < pp2->doc;
		}
	};
CL_NS_END
#endif
