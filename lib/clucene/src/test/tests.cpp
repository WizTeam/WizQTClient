/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "test.h"

unittest tests[] = {
    {"threads", testatomicupdates},
    {"indexreader", testindexreader},
    {"indexsearcher", testIndexSearcher},
    {"reuters", testreuters},
    {"analysis", testanalysis},
    {"analyzers", testanalyzers},
    {"document", testdocument},
    {"field", testField},
    {"numbertools", testNumberTools},
    {"debug", testdebug},
    {"ramdirectory", testRAMDirectory},
    {"indexwriter", testindexwriter},
    {"indexmodifier", testIndexModifier},
    {"addIndexesNoOptimize", testAddIndexesNoOptimize},
    {"highfreq", testhighfreq},
    {"priorityqueue", testpriorityqueue},
    {"datetools", testDateTools},
    {"queryparser", testQueryParser},
    {"mfqueryparser", testMultiFieldQueryParser},
    {"boolean", testBoolean},
    {"search", testsearch},
    {"rangefilter", testRangeFilter},
    {"queries", testqueries},
    {"csrqueries", testConstantScoreQueries},
    {"termvector",testtermvector},
    {"sort",testsort},
    {"duplicates", testduplicates},
    {"datefilter", testdatefilter},
    {"wildcard", testwildcard},
    {"store", teststore},
    {"utf8", testutf8},
    {"bitset", testBitSet},
    {"extractterms",testExtractTerms},
    {"spanqueries",testSpanQueries},
    {"stringbuffer", testStringBuffer},
    {"termvectorsreader",testTermVectorsReader},
#ifdef TEST_CONTRIB_LIBS
    {"germananalyzer", testGermanAnalyzer},
#endif
    {"LastTest", NULL}
};
