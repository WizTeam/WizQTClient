PROJECT(clucene-contribs-lib-test)

INCLUDE (DefineOptions)
DEFINE_OPTIONS(EXTRA_OPTIONS EXTRA_LIBS)
ADD_DEFINITIONS(${EXTRA_OPTIONS})

INCLUDE_DIRECTORIES( ${clucene-contribs-lib-test_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${clucene-contribs-lib_SOURCE_DIR} )

file(GLOB_RECURSE test_HEADERS ${CMAKE_SOURCE_DIR}/test/*.h)

SET(test_files
  ./contribTests.cpp
  ./TestHighlight.cpp
  ./TestSnowball.cpp
  ./TestStreams.cpp
  ./TestUtf8.cpp
  ./TestAnalysis.cpp
  ./CuTest.cpp
  ./testall.cpp
  ${test_HEADERS}
)
IF ( USE_SHARED_OBJECT_FILES )
    GET_SHARED_FILES(clucene_shared_Files)
ENDIF ( USE_SHARED_OBJECT_FILES )

#todo: do glob header and include header files for IDE.
ADD_EXECUTABLE(cl_contribs-lib-test EXCLUDE_FROM_ALL ${clucene_shared_Files} ${test_files} )

#link the executable against the releavent clucene-shared library (if we aren't using the object files)
IF ( NOT USE_SHARED_OBJECT_FILES )
    TARGET_LINK_LIBRARIES(cl_contribs-lib-test clucene-core clucene-shared clucene-contribs-lib ${EXTRA_LIBS})
ENDIF ( NOT USE_SHARED_OBJECT_FILES )
