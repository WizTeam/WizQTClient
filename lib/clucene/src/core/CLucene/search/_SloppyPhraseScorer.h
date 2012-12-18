/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_SloppyPhraseScorer_
#define _lucene_search_SloppyPhraseScorer_

#include "_PhraseScorer.h"

CL_NS_DEF(search)
	class SloppyPhraseScorer: public PhraseScorer {
	private:
		int32_t slop;
		PhrasePositions** repeats;
		size_t repeatsLen;
		bool checkedRepeats;
	public:
		SloppyPhraseScorer(Weight* weight, CL_NS(index)::TermPositions** tps, 
			int32_t* offsets, Similarity* similarity, 
			int32_t _slop, uint8_t* norms);
		virtual ~SloppyPhraseScorer();
	protected:
		/**
		* Score a candidate doc for all slop-valid position-combinations (matches) 
		* encountered while traversing/hopping the PhrasePositions.
		* <br> The score contribution of a match depends on the distance: 
		* <br> - highest score for distance=0 (exact match).
		* <br> - score gets lower as distance gets higher.
		* <br>Example: for query "a b"~2, a document "x a b a y" can be scored twice: 
		* once for "a b" (distance=0), and once for "b a" (distance=2).
		* <br>Pssibly not all valid combinations are encountered, because for efficiency  
		* we always propagate the least PhrasePosition. This allows to base on 
		* PriorityQueue and move forward faster. 
		* As result, for example, document "a b c b a"
		* would score differently for queries "a b c"~4 and "c b a"~4, although 
		* they really are equivalent. 
		* Similarly, for doc "a b c b a f g", query "c b"~2 
		* would get same score as "g f"~2, although "c b"~2 could be matched twice.
		* We may want to fix this in the future (currently not, for performance reasons).
		*/
		float_t phraseFreq();
	private:
		typedef CL_NS(util)::CLHashMap<PhrasePositions*, 
			const void*, CL_NS(util)::Compare::Void<PhrasePositions>, 
			CL_NS(util)::Equals::Void<PhrasePositions> >
			PhrasePositionsMap;
		static int comparePhrasePositions(const void* x, const void* y){
			return static_cast<const PhrasePositions*>(y)->offset - static_cast<const PhrasePositions*>(x)->offset;
		}

		/**
		* Init PhrasePositions in place.
		* There is a one time initializatin for this scorer:
		* <br>- Put in repeats[] each pp that has another pp with same position in the doc.
		* <br>- Also mark each such pp by pp.repeats = true.
		* <br>Later can consult with repeats[] in termPositionsDiffer(pp), making that check efficient.
		* In particular, this allows to score queries with no repetiotions with no overhead due to this computation.
		* <br>- Example 1 - query with no repetitions: "ho my"~2
		* <br>- Example 2 - query with repetitions: "ho my my"~2
		* <br>- Example 3 - query with repetitions: "my ho my"~2
		* <br>Init per doc w/repeats in query, includes propagating some repeating pp's to avoid false phrase detection.  
		* @return end (max position), or -1 if any term ran out (i.e. done) 
		* @throws IOException 
		*/
		int32_t initPhrasePositions();

		// disalow two pp's to have the same tp position, so that same word twice 
		// in query would go elswhere in the matched doc
		bool termPositionsDiffer(PhrasePositions* pp);

	public:
		virtual TCHAR* toString();
	};
CL_NS_END
#endif

