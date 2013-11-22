cmake_minimum_required(VERSION 2.8.4)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

if(NOT WIZNOTE_USE_QT5)
    set(WIZNOTE_USE_QT5 OFF CACHE BOOL
        "Build WizNote use Qt5, default is off.")
endif(NOT WIZNOTE_USE_QT5)


if(APPLE)
    set(WIZNOTE_ROOT_DIR ${CMAKE_BINARY_DIR}/wiznote.app)
    set(WIZNOTE_PLUGIN_DIR ${WIZNOTE_ROOT_DIR}/Contents/PlugIns)
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/Contents/Resources)
else(APPLE)
    set(WIZNOTE_ROOT_DIR ${CMAKE_BINARY_DIR})
    set(WIZNOTE_PLUGIN_DIR ${WIZNOTE_ROOT_DIR}/share/wiznote/plugins)
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/share)
endif(APPLE)

# must output to "lib" to make macdeployqt work correctly
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


add_subdirectory(lib)
add_subdirectory(src)
