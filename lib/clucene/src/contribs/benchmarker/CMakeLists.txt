PROJECT(clucene-benchmarker)

INCLUDE (DefineOptions)
DEFINE_OPTIONS(EXTRA_OPTIONS EXTRA_LIBS)
ADD_DEFINITIONS(${EXTRA_OPTIONS})

file(GLOB_RECURSE benchmarker_HEADERS ${clucene-benchmarker_SOURCE_DIR}/*.h)

SET(benchmarker_files
  ./Benchmarker.cpp
  ./Main.cpp
  ./stdafx.cpp
  ./Unit.cpp

  ./TestCLString.cpp
  ${benchmarker_HEADERS}
)

ADD_EXECUTABLE(cl_benchmarker EXCLUDE_FROM_ALL ${benchmarker_files} )
TARGET_LINK_LIBRARIES(cl_benchmarker clucene-core clucene-shared ${EXTRA_LIBS})
