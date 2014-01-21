# NOTE for OSX users:
# export MACOSX_DEPLOYMENT_TARGET=10.7 on .bash_profile if you want to compile use 10.7 sdk
# sdk location: /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs
# you can download old sdk from here: https://github.com/JuliaEichler/Mac_OSX_SDKs
# default is 10.7 for binary release.

if (APPLE)
    cmake_minimum_required(VERSION 2.8.12)
else(APPLE)
    cmake_minimum_required(VERSION 2.8.4)
endif(APPLE)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# rpath resolve.
set(CMAKE_BUILD_WITH_INSTALL_RPATH YES)
if(APPLE)
    set(CMAKE_INSTALL_NAME_DIR "@rpath/PlugIns")
    SET(CMAKE_INSTALL_RPATH "@loader_path/..;@executable_path/..")
else(APPLE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/..;$ORIGIN/../lib/wiznote/plugins")
endif(APPLE)

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
    set(WIZNOTE_PLUGIN_DIR ${WIZNOTE_ROOT_DIR}/lib/wiznote/plugins)
    set(WIZNOTE_RESOURCE_DIR ${WIZNOTE_ROOT_DIR}/share)
endif(APPLE)

# must output to "lib" to make macdeployqt work correctly
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${WIZNOTE_PLUGIN_DIR})


add_subdirectory(lib)
add_subdirectory(src)
