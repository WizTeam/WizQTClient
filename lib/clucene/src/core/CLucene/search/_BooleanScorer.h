/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_BooleanScorer_
#define _lucene_search_BooleanScorer_


#include "Scorer.h"

CL_NS_DEF(search)
	
	class BooleanScorer: public Scorer {
	private:
			
		class Bucket {
		public:
			int32_t	doc;				  // tells if bucket is valid
			float_t	score;				  // incremental score
			int32_t	bits;					  // used for bool constraints
			int32_t	coord;					  // count of terms in score
			Bucket*	next;				  // next valid bucket

			Bucket();
			virtual ~Bucket();
		};

		class SubScorer {
		public:
			bool done;
			Scorer* scorer;
			bool required;
			bool prohibited;
			HitCollector* collector;
			SubScorer* next;
			SubScorer(Scorer* scr, const bool r, const bool p, HitCollector* c, SubScorer* nxt);
			virtual ~SubScorer();
		};

		class BucketTable {
		private:
			BooleanScorer* scorer;
		public:
			Bucket* buckets;
			Bucket* first;			  // head of valid list

			BucketTable(BooleanScorer* scr);
			int32_t size() const;
			HitCollector* newCollector(const int32_t mask);
			void clear();
			virtual ~BucketTable();
		};

		class Collector: public HitCollector {
		private:
			BucketTable* bucketTable;
			int32_t mask;
		public:
			Collector(const int32_t mask, BucketTable* bucketTable);
			
			void collect(const int32_t doc, const float_t score);
		};

		SubScorer* scorers;
		BucketTable* bucketTable;

		int32_t maxCoord;
		int32_t nextMask;

      	int32_t end;
		Bucket* current;
		
		int32_t minNrShouldMatch;
		
	public:
		LUCENE_STATIC_CONSTANT(int32_t,BucketTable_SIZE=1024);
		int32_t requiredMask;
		int32_t prohibitedMask;
		float_t* coordFactors;

    	BooleanScorer( Similarity* similarity, int32_t minNrShouldMatch = 1 );
		virtual ~BooleanScorer();
		void add(Scorer* scorer, const bool required, const bool prohibited);
		int32_t doc() const { return current->doc; }
		bool next();
		float_t score();
		void score( HitCollector* hc );
		bool skipTo(int32_t target);
		Explanation* explain(int32_t doc);
		virtual TCHAR* toString();
		void computeCoordFactors();
		
	protected:
		bool score( HitCollector* hc, const int32_t max );
		
	};

CL_NS_END
#endif
