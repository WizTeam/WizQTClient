cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

add_subdirectory(lib/cryptopp)
add_subdirectory(lib/clucene)
add_subdirectory(lib/quazip)
add_subdirectory(lib/aggregation)
add_subdirectory(lib/extensionsystem)

add_subdirectory(src)
