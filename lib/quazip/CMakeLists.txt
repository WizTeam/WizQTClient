project (QuaZip)

file(GLOB SRCS "*.c" "*.cpp")
file(GLOB PUBLIC_HEADERS "*.h")

include(../../cmake/QtChooser.cmake)
add_library(quazip STATIC ${SRCS} ${PUBLIC_HEADERS})
qt_use_modules(quazip)
qt_suppress_warnings(quazip)

# set all include directories for in and out of source builds
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
        ${ZLIB_INCLUDE_DIR}
)

# Link against ZLIB_LIBRARIES if needed (on Windows this variable is empty)
target_link_libraries(quazip ${ZLIB_LIBRARY})

