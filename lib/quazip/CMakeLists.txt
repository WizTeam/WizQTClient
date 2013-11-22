project (QuaZip)

file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

#find_package(ZLIB)
#if(NOT ZLIB_FOUND)
#    message("ZLIB not found, quazip using local: ${clucene-ext_SOURCE_DIR}/zlib")
#    set(ZLIB_INCLUDE_DIR ${clucene-ext_SOURCE_DIR}/zlib)
#    set(ZLIB_LIBRARY ${clucene-ext_BINARY_DIR})
#endif(NOT ZLIB_FOUND)

#INCLUDE_DIRECTORIES( ${ZLIB_INCLUDE_DIR} )

#find_package(ZLIB)
#if(NOT ZLIB_FOUND)
#	SET(ZLIB_INCLUDE_DIRS "${QT_ROOT}/src/3rdparty/zlib" CACHE STRING "Path to ZLIB headers of Qt")
#	SET(ZLIB_LIBRARIES "")
#	IF(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
#		MESSAGE("Please specify a valid zlib include dir")
#	ENDIF(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
#ENDIF(NOT ZLIB_FOUND)

include(QtChooser)
add_library(quazip STATIC ${SRCS} ${PUBLIC_HEADERS})
qt_use_modules(quazip)

# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
        ${ZLIB_INCLUDE_DIR}
)

# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(quazip ${ZLIB_LIBRARY})
