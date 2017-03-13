project(libs)

find_package(ZLIB)
if(NOT ZLIB_FOUND)
    add_subdirectory(zlib)
    message("ZLIB not found, using internal: ${zlib_SOURCE_DIR}")
    set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
    set(ZLIB_LIBRARY zlib)
endif(NOT ZLIB_FOUND)

add_subdirectory(cryptopp)
add_subdirectory(quazip)
#add_subdirectory(aggregation)
#add_subdirectory(extensionsystem)
