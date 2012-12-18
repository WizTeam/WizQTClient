/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
/*
* this is a monolithic file that can be used to compile clucene tests using one source file.
* 
* note: when creating a project add either this file, or all the other .cpp files, not both!
*/

#include "CuTest.cpp"
#include "testall.cpp"
#include "tests.cpp"

#include "analysis/TestAnalysis.cpp"
#include "analysis/TestAnalyzers.cpp"
#include "debug/TestError.cpp"
#include "document/TestDateTools.cpp"
#include "document/TestDocument.cpp"
#include "document/TestField.cpp"
#include "document/TestNumberTools.cpp"
#include "index/IndexWriter4Test.cpp"
#include "index/TestAddIndexesNoOptimize.cpp"
#include "index/TestHighFreqTerms.cpp"
#include "index/TestIndexModifier.cpp"
#include "index/TestIndexReader.cpp"
#include "index/TestIndexWriter.cpp"
#include "index/TestReuters.cpp"
#include "index/TestTermVectorsReader.cpp"
#include "index/TestThreading.cpp"
#include "index/TestUtf8.cpp"
#include "queryParser/TestMultiFieldQueryParser.cpp"
#include "queryParser/TestQueryParser.cpp"
#include "search/BaseTestRangeFilter.cpp"
#include "search/CheckHits.cpp"
#include "search/QueryUtils.cpp"
#include "search/spans/TestBasics.cpp"
#include "search/spans/TestNearSpansOrdered.cpp"
#include "search/spans/TestSpanExplanations.cpp"
#include "search/spans/TestSpanExplanationsOfNonMatches.cpp"
#include "search/spans/TestSpanQueries.cpp"
#include "search/spans/TestSpansAdvanced2.cpp"
#include "search/spans/TestSpansAdvanced.cpp"
#include "search/spans/TestSpans.cpp"
#include "search/TestBoolean.cpp"
#include "search/TestConstantScoreRangeQuery.cpp"
#include "search/TestDateFilter.cpp"
#include "search/TestExplanations.cpp"
#include "search/TestExtractTerms.cpp"
#include "search/TestForDuplicates.cpp"
#include "search/TestIndexSearcher.cpp"
#include "search/TestQueries.cpp"
#include "search/TestRangeFilter.cpp"
#include "search/TestSearch.cpp"
#include "search/TestSort.cpp"
#include "search/TestTermVector.cpp"
#include "search/TestWildcard.cpp"
#include "store/MockRAMDirectory.cpp"
#include "store/TestRAMDirectory.cpp"
#include "store/TestStore.cpp"
#include "util/English.cpp"
#include "util/TestBitSet.cpp"
#include "util/TestPriorityQueue.cpp"
#include "util/TestStringBuffer.cpp"

