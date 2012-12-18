/*------------------------------------------------------------------------------
* Copyright (C) 2010 Borivoj Kostka and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_test_index_IndexWriter4Test_
#define _lucene_test_index_IndexWriter4Test_

#include "CLucene/index/IndexWriter.h"

CL_NS_DEF(index)

/*
 * Derived from IndexWriter, contains methods used only for testing purposes.
 *
 */
class IndexWriter4Test : public IndexWriter {

public:

  IndexWriter4Test(CL_NS(store)::Directory* d, CL_NS(analysis)::Analyzer* a, const bool create);
  IndexWriter4Test(CL_NS(store)::Directory* d, bool autocommit, CL_NS(analysis)::Analyzer* a, const bool create);

  int32_t getDocCount(int32_t i);
  int32_t getNumBufferedDocuments();
  int32_t getSegmentCount();
  int32_t getBufferedDeleteTermsSize();
  int32_t getNumBufferedDeleteTerms();
  SegmentInfo* newestSegment();

};

CL_NS_END
#endif
