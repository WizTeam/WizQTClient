PROJECT(cl_demo)

INCLUDE (DefineOptions)
DEFINE_OPTIONS(EXTRA_OPTIONS EXTRA_LIBS)
ADD_DEFINITIONS(${EXTRA_OPTIONS})

INCLUDE_DIRECTORIES( ${clucene-demo_SOURCE_DIR} )
INCLUDE_DIRECTORIES( ${clucene_SOURCE_DIR}/src/core )

file(GLOB_RECURSE demo_HEADERS ${CMAKE_SOURCE_DIR}/test/*.h)

ADD_EXECUTABLE(cl_demo EXCLUDE_FROM_ALL
#./DeleteFiles.cpp
./IndexFiles.cpp
./Main.cpp
./SearchFiles.cpp
./Statistics.cpp
#${clucene-shared_SOURCE_DIR}/CLucene/util/Misc.cpp
#${clucene-shared_SOURCE_DIR}/CLucene/util/dirent.cpp
${demo_HEADERS}
)

TARGET_LINK_LIBRARIES(cl_demo clucene-core clucene-shared ${EXTRA_LIBS})
