project (QuaZip)

file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

if(WIZNOTE_USE_QT5)
    find_package(Qt5 REQUIRED Core)
else(WIZNOTE_USE_QT5)
    find_package(Qt4 REQUIRED)
    INCLUDE(${QT_USE_FILE})
endif(WIZNOTE_USE_QT5)

set(CMAKE_AUTOMOC ON)

# Use system zlib on unix and Qt ZLIB on Windows
IF(UNIX)
	find_package(ZLIB REQUIRED)
ELSE(UNIX)
	SET(ZLIB_INCLUDE_DIRS "${QT_ROOT}/src/3rdparty/zlib" CACHE STRING "Path to ZLIB headers of Qt")
	SET(ZLIB_LIBRARIES "")
	IF(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
		MESSAGE("Please specify a valid zlib include dir")
	ENDIF(NOT EXISTS "${ZLIB_INCLUDE_DIRS}/zlib.h")
ENDIF(UNIX)

add_library(quazip STATIC ${SRCS} ${PUBLIC_HEADERS})
set_target_properties(quazip PROPERTIES VERSION 0.5.1 SOVERSION 2)

if(WIZNOTE_USE_QT5)
    qt5_use_modules(quazip Core)
else(WIZNOTE_USE_QT5)
    target_link_libraries(quazip ${QT_LIBRARIES})
endif(WIZNOTE_USE_QT5)

# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIRS}
)

# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(quazip ${ZLIB_LIBRARIES})
