/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_search_ConjunctionScorer_
#define _lucene_search_ConjunctionScorer_

#include "Scorer.h"
#include "CLucene/util/Array.h"
CL_NS_DEF(search)

/** Scorer for conjunctions, sets of queries, all of which are required. */
class ConjunctionScorer: public Scorer {
private:
  CL_NS(util)::ArrayBase<Scorer*>* scorers;
  typedef CL_NS(util)::CLVector<Scorer*,CL_NS(util)::Deletor::Object<Scorer> > ScorersType;
  bool firstTime;
  bool more;
  float_t coord;
  int32_t lastDoc;

  Scorer* last();
  bool doNext();

  bool init(int32_t target);
public:
  ConjunctionScorer(Similarity* similarity, ScorersType* scorers);
  ConjunctionScorer(Similarity* similarity, const CL_NS(util)::ArrayBase<Scorer*>* scorers);
  virtual ~ConjunctionScorer();
  virtual TCHAR* toString();
  int32_t doc() const;
  bool next();
  bool skipTo(int32_t target);
  virtual float_t score();
  virtual Explanation* explain(int32_t doc);
};

CL_NS_END
#endif
