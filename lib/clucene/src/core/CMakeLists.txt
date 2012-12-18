PROJECT(clucene-core)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${clucene-shared_SOURCE_DIR}/cmake")

#define command line options
INCLUDE (DefineOptions)
DEFINE_OPTIONS(EXTRA_OPTIONS EXTRA_LIBS)
ADD_DEFINITIONS(${EXTRA_OPTIONS} -DMAKE_CLUCENE_CORE_LIB)

#see if we can hide all symbols by default...
MACRO_CHECK_GCC_VISIBILITY(_CL_HAVE_GCCVISIBILITYPATCH)

#add the files to our groups and core
SOURCE_GROUP("analysis" ./CLucene/analysis/*)
#seems to be a bug in cmake, can't put these in analysis\\standard folder :(
SOURCE_GROUP("analysis-standard" ./CLucene/analysis/standard/*)
SOURCE_GROUP("document" ./CLucene/document/*)
SOURCE_GROUP("debug" ./CLucene/debug/*)
SOURCE_GROUP("index" ./CLucene/index/*)
SOURCE_GROUP("queryParser" ./CLucene/queryParser/*)
SOURCE_GROUP("queryParser-legacy" ./CLucene/queryParser/legacy/*)
SOURCE_GROUP("search" ./CLucene/search/*)
SOURCE_GROUP("search-spans" ./CLucene/search/spans/*)
SOURCE_GROUP("store" ./CLucene/store/*)
SOURCE_GROUP("util" ./CLucene/util/*)

SET(clucene_core_Files
	./CLucene/StdHeader.cpp
	./CLucene/debug/error.cpp
	./CLucene/util/ThreadLocal.cpp
	./CLucene/util/Reader.cpp
	./CLucene/util/Equators.cpp
	./CLucene/util/FastCharStream.cpp
	./CLucene/util/MD5Digester.cpp
	./CLucene/util/StringIntern.cpp
	./CLucene/util/BitSet.cpp
	./CLucene/queryParser/FastCharStream.cpp
	./CLucene/queryParser/MultiFieldQueryParser.cpp
	./CLucene/queryParser/QueryParser.cpp
	./CLucene/queryParser/QueryParserTokenManager.cpp
	./CLucene/queryParser/QueryToken.cpp
	./CLucene/queryParser/legacy/Lexer.cpp
	./CLucene/queryParser/legacy/MultiFieldQueryParser.cpp
	./CLucene/queryParser/legacy/QueryParser.cpp
	./CLucene/queryParser/legacy/QueryParserBase.cpp
	./CLucene/queryParser/legacy/QueryToken.cpp
	./CLucene/queryParser/legacy/TokenList.cpp
	./CLucene/analysis/standard/StandardAnalyzer.cpp
	./CLucene/analysis/standard/StandardFilter.cpp
	./CLucene/analysis/standard/StandardTokenizer.cpp
	./CLucene/analysis/Analyzers.cpp
	./CLucene/analysis/AnalysisHeader.cpp
	./CLucene/store/MMapInput.cpp
	./CLucene/store/IndexInput.cpp
	./CLucene/store/Lock.cpp
	./CLucene/store/LockFactory.cpp
	./CLucene/store/IndexOutput.cpp
	./CLucene/store/Directory.cpp
	./CLucene/store/FSDirectory.cpp
	./CLucene/store/RAMDirectory.cpp
	./CLucene/document/Document.cpp
	./CLucene/document/DateField.cpp
	./CLucene/document/DateTools.cpp
	./CLucene/document/Field.cpp
	./CLucene/document/FieldSelector.cpp
	./CLucene/document/NumberTools.cpp
	./CLucene/index/IndexFileNames.cpp
	./CLucene/index/IndexFileNameFilter.cpp
	./CLucene/index/IndexDeletionPolicy.cpp
	./CLucene/index/SegmentMergeInfo.cpp
	./CLucene/index/SegmentInfos.cpp
	./CLucene/index/MergeScheduler.cpp
	./CLucene/index/SegmentTermDocs.cpp
	./CLucene/index/FieldsWriter.cpp
	./CLucene/index/TermInfosWriter.cpp
	./CLucene/index/Term.cpp
	./CLucene/index/Terms.cpp
	./CLucene/index/MergePolicy.cpp
	./CLucene/index/DocumentsWriter.cpp
	./CLucene/index/DocumentsWriterThreadState.cpp
	./CLucene/index/SegmentTermVector.cpp
	./CLucene/index/TermVectorReader.cpp
	./CLucene/index/FieldInfos.cpp
	./CLucene/index/CompoundFile.cpp
	./CLucene/index/SkipListReader.cpp
	./CLucene/index/SkipListWriter.cpp
	./CLucene/index/IndexFileDeleter.cpp
	./CLucene/index/SegmentReader.cpp
	./CLucene/index/DirectoryIndexReader.cpp
	./CLucene/index/TermVectorWriter.cpp
	./CLucene/index/IndexReader.cpp
	./CLucene/index/SegmentTermPositions.cpp
	./CLucene/index/SegmentMerger.cpp
	./CLucene/index/IndexWriter.cpp
	./CLucene/index/MultiReader.cpp
	./CLucene/index/MultiSegmentReader.cpp
	./CLucene/index/Payload.cpp
	./CLucene/index/SegmentTermEnum.cpp
	./CLucene/index/TermInfo.cpp
	./CLucene/index/IndexModifier.cpp
	./CLucene/index/SegmentMergeQueue.cpp
	./CLucene/index/FieldsReader.cpp
	./CLucene/index/TermInfosReader.cpp
	./CLucene/index/MultipleTermPositions.cpp
	./CLucene/search/Compare.cpp
	./CLucene/search/Scorer.cpp
	./CLucene/search/ScorerDocQueue.cpp
	./CLucene/search/PhraseScorer.cpp
	./CLucene/search/SloppyPhraseScorer.cpp
	./CLucene/search/DisjunctionSumScorer.cpp
	./CLucene/search/ConjunctionScorer.cpp
	./CLucene/search/PhraseQuery.cpp
	./CLucene/search/PrefixQuery.cpp
	./CLucene/search/ExactPhraseScorer.cpp
	./CLucene/search/TermScorer.cpp
	./CLucene/search/Similarity.cpp
	./CLucene/search/BooleanScorer.cpp
	./CLucene/search/BooleanScorer2.cpp
	./CLucene/search/HitQueue.cpp
	./CLucene/search/FieldCacheImpl.cpp
	./CLucene/search/ChainedFilter.cpp
	./CLucene/search/RangeFilter.cpp
	./CLucene/search/CachingWrapperFilter.cpp
	./CLucene/search/QueryFilter.cpp
	./CLucene/search/TermQuery.cpp
	./CLucene/search/FuzzyQuery.cpp
	./CLucene/search/SearchHeader.cpp
	./CLucene/search/RangeQuery.cpp
	./CLucene/search/IndexSearcher.cpp
	./CLucene/search/Sort.cpp
	./CLucene/search/PhrasePositions.cpp
	./CLucene/search/FieldDocSortedHitQueue.cpp
	./CLucene/search/WildcardTermEnum.cpp
	./CLucene/search/MultiSearcher.cpp
	./CLucene/search/Hits.cpp
	./CLucene/search/MultiTermQuery.cpp
	./CLucene/search/FilteredTermEnum.cpp
	./CLucene/search/FieldSortedHitQueue.cpp
	./CLucene/search/WildcardQuery.cpp
	./CLucene/search/Explanation.cpp
	./CLucene/search/BooleanQuery.cpp
	./CLucene/search/FieldCache.cpp
	./CLucene/search/DateFilter.cpp
	./CLucene/search/MatchAllDocsQuery.cpp
	./CLucene/search/MultiPhraseQuery.cpp
	./CLucene/search/ConstantScoreQuery.cpp
	./CLucene/search/CachingSpanFilter.cpp
	./CLucene/search/CachingSpanFilter.h
	./CLucene/search/SpanFilter.h
	./CLucene/search/SpanFilterResult.h
	./CLucene/search/SpanQueryFilter.cpp
	./CLucene/search/SpanQueryFilter.h	
	./CLucene/search/spans/_EmptySpans.h
	./CLucene/search/spans/_NearSpansOrdered.h
	./CLucene/search/spans/_NearSpansUnordered.h
	./CLucene/search/spans/_TermSpans.h
	./CLucene/search/spans/NearSpansOrdered.cpp
	./CLucene/search/spans/NearSpansUnordered.cpp
	./CLucene/search/spans/SpanFirstQuery.cpp
	./CLucene/search/spans/SpanFirstQuery.h
	./CLucene/search/spans/SpanNearQuery.cpp
	./CLucene/search/spans/SpanNearQuery.h
	./CLucene/search/spans/SpanNotQuery.cpp
	./CLucene/search/spans/SpanNotQuery.h
	./CLucene/search/spans/SpanOrQuery.cpp
	./CLucene/search/spans/SpanOrQuery.h
	./CLucene/search/spans/SpanQuery.h
	./CLucene/search/spans/Spans.h
	./CLucene/search/spans/SpanScorer.cpp
	./CLucene/search/spans/SpanScorer.h
	./CLucene/search/spans/SpanTermQuery.cpp
	./CLucene/search/spans/SpanTermQuery.h
	./CLucene/search/spans/SpanWeight.cpp
	./CLucene/search/spans/SpanWeight.h
	./CLucene/search/spans/TermSpans.cpp
)

#if USE_SHARED_OBJECT_FILES then we link directly to the object files (means rebuilding them for the core)
IF ( USE_SHARED_OBJECT_FILES )
    GET_SHARED_FILES(clucene_shared_Files)
ENDIF ( USE_SHARED_OBJECT_FILES )

#find our headers
file(GLOB_RECURSE HEADERS ${clucene-core_SOURCE_DIR}/*.h)


#create the libraries
INCLUDE_DIRECTORIES( ${clucene_SOURCE_DIR}/src/core )

add_library(clucene-core SHARED
	${clucene_core_Files} ${clucene_shared_Files} ${HEADERS}
)

#set properties on the libraries
SET_TARGET_PROPERTIES(clucene-core PROPERTIES
    VERSION ${CLUCENE_VERSION}
    SOVERSION ${CLUCENE_SOVERSION}
    COMPILE_DEFINITIONS_DEBUG _DEBUG
)

#link the clucene-core library against the releavent clucene-shared library (if we aren't using the object files)
IF ( NOT USE_SHARED_OBJECT_FILES )
    TARGET_LINK_LIBRARIES(clucene-core clucene-shared ${EXTRA_LIBS})
ENDIF ( NOT USE_SHARED_OBJECT_FILES )


IF ( BUILD_STATIC_LIBRARIES )
	add_library(clucene-core-static STATIC
		${clucene_core_Files} ${clucene_shared_Files} ${HEADERS}
	)
	
	SET_TARGET_PROPERTIES(clucene-core-static PROPERTIES
	    VERSION ${CLUCENE_VERSION}
	    SOVERSION ${CLUCENE_SOVERSION}
	    COMPILE_DEFINITIONS_DEBUG _DEBUG
	)
	
	#and install library
	install(TARGETS clucene-core-static 
	        DESTINATION ${LIB_DESTINATION} 
	        COMPONENT runtime )
ENDIF ( BUILD_STATIC_LIBRARIES )

#install public headers.
FOREACH(file ${HEADERS})
	get_filename_component(apath ${file} PATH)
	get_filename_component(aname ${file} NAME)
	file(RELATIVE_PATH relpath ${CMAKE_SOURCE_DIR}/src/core ${apath})
	IF ( NOT aname MATCHES "^_.*" )
		install(FILES ${file} 
		        DESTINATION include/${relpath}
		        COMPONENT development)
	ENDIF ( NOT aname MATCHES "^_.*" )
ENDFOREACH(file)

#install clucene-shared headers.
install(FILES ${clucene-shared_SOURCE_DIR}/CLucene/SharedHeader.h 
        DESTINATION include/CLucene 
        COMPONENT development)
install(FILES ${clucene-shared_SOURCE_DIR}/CLucene/LuceneThreads.h 
        DESTINATION include/CLucene 
        COMPONENT development )

# code for installing an script to help cmake applications determine
# the CLucene version number
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CLuceneConfig.cmake" "
    set(CLUCENE_VERSION_MAJOR ${CLUCENE_VERSION_MAJOR})
    set(CLUCENE_VERSION_MINOR ${CLUCENE_VERSION_MINOR})
    set(CLUCENE_VERSION_REVISION ${CLUCENE_VERSION_REVISION})
    set(CLUCENE_VERSION_PATCH ${CLUCENE_VERSION_PATCH})
    
    set(CLUCENE_VERSION ${CLUCENE_VERSION})
    set(CLUCENE_SOVERSION ${CLUCENE_SOVERSION})
")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/CLuceneConfig.cmake"
        DESTINATION ${LIB_DESTINATION}/CLuceneConfig.cmake)

# install pkg-config file
IF(NOT WIN32)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libclucene-core.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/libclucene-core.pc @ONLY)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libclucene-core.pc
          DESTINATION ${LIB_DESTINATION}/pkgconfig )
ENDIF(NOT WIN32)


#install non system-independent
IF ( LUCENE_SYS_INCLUDES )
    install(FILES ${clucene-shared_BINARY_DIR}/CLucene/clucene-config.h 
            DESTINATION ${LUCENE_SYS_INCLUDES}/CLucene  
            COMPONENT development)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CLuceneConfig.cmake 
            DESTINATION ${LUCENE_SYS_INCLUDES}/CLucene  
            COMPONENT development)
ELSE ( LUCENE_SYS_INCLUDES )
    install(FILES ${clucene-shared_BINARY_DIR}/CLucene/clucene-config.h 
            DESTINATION include/CLucene
            COMPONENT development)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CLuceneConfig.cmake 
            DESTINATION include/CLucene
            COMPONENT development)
ENDIF ( LUCENE_SYS_INCLUDES )


#and install library
install(TARGETS clucene-core 
        DESTINATION ${LIB_DESTINATION}  
        COMPONENT runtime )
