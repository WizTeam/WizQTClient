# Copyright (c) 2010, 2011 Sergiu Dotenco
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required (VERSION 2.8)
project (cryptopp)

option (CRYPTOPP_FIPS_COMPLIANT "CryptoPP FIPS 140 compliant" OFF)
option (CRYPTOPP_BUILD_TESTS "Build CryptoPP tests" OFF)
option (CRYPTOPP_BUILD_SHARED_LIBS "Build shared CryptoPP libraries" OFF)
option (CRYPTOPP_BUILD_STATIC_LIBS "Build static CryptoPP libraries" ON)

if (NOT CRYPTOPP_BUILD_STATIC_LIBS AND CRYPTOPP_BUILD_TESTS)
  set (CRYPTOPP_BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
endif (NOT CRYPTOPP_BUILD_STATIC_LIBS AND CRYPTOPP_BUILD_TESTS)

if (NOT CRYPTOPP_BUILD_SHARED_LIBS AND CRYPTOPP_BUILD_TESTS)
  set (CRYPTOPP_BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
endif (NOT CRYPTOPP_BUILD_SHARED_LIBS AND CRYPTOPP_BUILD_TESTS)

file (GLOB PROJECT_CPP *.cpp)
file (GLOB PROJECT_H *.h)
file (GLOB PROJECT_PCH pch.h)

configure_file (adhoc.cpp.proto ${CMAKE_CURRENT_BINARY_DIR}/adhoc.cpp COPYONLY)

list (APPEND PROJECT_CPP ${CMAKE_CURRENT_BINARY_DIR}/adhoc.cpp)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (PROJECT_TEST bench bench2 test validat1 validat2 validat3 adhoc datatest
  regtest fipsalgt fipstest)

if (WIN32)
  list (APPEND PROJECT_TEST dlltest)
endif (WIN32)

set (PROJECT_TEST_CPP)

foreach (_PROJECT_TEST_FILE ${PROJECT_TEST})
  foreach (_PROJECT_FILE ${PROJECT_CPP})
    if (${_PROJECT_FILE} MATCHES ".*${_PROJECT_TEST_FILE}\\.cpp$")
      LIST (REMOVE_ITEM PROJECT_CPP ${_PROJECT_FILE})
    endif (${_PROJECT_FILE} MATCHES ".*${_PROJECT_TEST_FILE}\\.cpp$")
  endforeach (_PROJECT_FILE)
  list (APPEND PROJECT_TEST_CPP ${_PROJECT_TEST_FILE}.cpp)
endforeach(_PROJECT_TEST_FILE)

set (PROJECT_CPP_LIB ${PROJECT_CPP})
set (PROJECT_CPP_SHARED ${PROJECT_CPP})

list (APPEND PROJECT_CPP_SHARED ${CMAKE_CURRENT_SOURCE_DIR}/fipstest.cpp)
list (REMOVE_ITEM PROJECT_CPP_SHARED ${CMAKE_CURRENT_SOURCE_DIR}/ttmac.cpp)

foreach (_PROJECT_FILE ${PROJECT_CPP_LIB})
  if (${_PROJECT_FILE} MATCHES ".*dll.+\\.cpp$")
    list (REMOVE_ITEM PROJECT_CPP_LIB ${_PROJECT_FILE})
  endif (${_PROJECT_FILE} MATCHES ".*dll.+\\.cpp$")
endforeach (_PROJECT_FILE)

# Intel compiler
if (CMAKE_CXX_COMPILER MATCHES ic[c|l])
  add_definitions (-wd811)
  # Variable of static storage duration was declared but never referenced
  add_definitions (-wd2415)
  # Function was declared but never referenced
  add_definitions (-wd177)
endif (CMAKE_CXX_COMPILER MATCHES ic[c|l])

#remove_definitions (-DUNICODE -D_UNICODE)

if (MINGW AND NOT CMAKE_RC_COMPILER_INIT)
  set (CMAKE_RC_COMPILER_INIT windres)
  enable_language (RC)
  set (CMAKE_RC_COMPILE_OBJECT "-O coff -o <OBJECT> <SOURCE>")
endif (MINGW AND NOT CMAKE_RC_COMPILER_INIT)

if (WIN32)
  list (APPEND PROJECT_CPP_SHARED cryptopp.rc)
endif (WIN32)

if (CRYPTOPP_BUILD_SHARED_LIBS)
  add_library (cryptopp SHARED ${PROJECT_CPP_SHARED} ${PROJECT_H}
    ${PROJECT_PCH})
endif (CRYPTOPP_BUILD_SHARED_LIBS)

if (CRYPTOPP_BUILD_STATIC_LIBS)
  add_library (cryptlib STATIC ${PROJECT_CPP_LIB} ${PROJECT_H} ${PROJECT_PCH})
endif (CRYPTOPP_BUILD_STATIC_LIBS)

if (CRYPTOPP_BUILD_TESTS)
  add_executable (crypttest ${PROJECT_TEST_CPP})

  if (CMAKE_COMPILER_IS_GNUCXX)
    set_target_properties (crypttest PROPERTIES LINK_FLAGS -pthread)
  endif (CMAKE_COMPILER_IS_GNUCXX)

  if (WIN32)
    add_executable (dlltest dlltest.cpp)
  endif (WIN32)
endif (CRYPTOPP_BUILD_TESTS)

set (_CRYPTOPP_COMPILE_DEFINITIONS USE_PRECOMPILED_HEADERS)

if (CRYPTOPP_FIPS_COMPLIANT)
  list (APPEND _CRYPTOPP_COMPILE_DEFINITIONS
    CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2=1)
else (CRYPTOPP_FIPS_COMPLIANT)
  list (APPEND _CRYPTOPP_COMPILE_DEFINITIONS
    CRYPTOPP_ENABLE_COMPLIANCE_WITH_FIPS_140_2=0)
endif (CRYPTOPP_FIPS_COMPLIANT)

if (TARGET cryptlib)
  list (APPEND _CRYPTOPP_INSTALL_TARGETS cryptlib)
endif (TARGET cryptlib)

if (TARGET cryptopp)
  list (APPEND _CRYPTOPP_INSTALL_TARGETS cryptopp)
endif (TARGET cryptopp)

if (APPLE)
    add_definitions (-DCRYPTOPP_DISABLE_ASM)
endif (APPLE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES COMPILE_FLAGS
    "-Wno-tautological-compare -Wno-unused-value -Wno-switch -Wno-string-plus-int")
endif()

# Prevent MSVC from issuing the LNK4221 warning
set_source_files_properties (strciphr.cpp simple.cpp polynomi.cpp algebra.cpp
  eccrypto.cpp eprecomp.cpp PROPERTIES HEADER_FILE_ONLY TRUE)

if (MSVC)
  set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES
    COMPILE_DEFINITIONS "${_CRYPTOPP_COMPILE_DEFINITIONS}")
  # Setup precompiled headers and use intrinsics; the latter fixes MSVC8 linker
  # errors
  #set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES COMPILE_FLAGS
  #  "-Yupch.h -Oi")
  #set_source_files_properties (pch.cpp PROPERTIES COMPILE_FLAGS -Yc)
  # These files don't use precompiled headers
  set_source_files_properties (dll.cpp cryptlib_bds.cpp iterhash.cpp
    PROPERTIES COMPILE_FLAGS -Y-)
endif (MSVC)

if (_CRYPTOPP_INSTALL_TARGETS AND WIN32)
  set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES DEFINE_SYMBOL
    CRYPTOPP_EXPORTS)
endif (_CRYPTOPP_INSTALL_TARGETS AND WIN32)

if (TARGET crypttest)
  target_link_libraries (crypttest cryptlib cryptopp)
endif (TARGET crypttest)

if (TARGET dlltest)
  target_link_libraries (dlltest cryptopp)
endif (TARGET dlltest)

if (WIN32)
  get_target_property (_TEMP cryptopp COMPILE_DEFINITIONS)

  if (NOT _TEMP)
    unset (_TEMP)
  endif (NOT _TEMP)

  list (APPEND _TEMP _USRDLL)

  if (TARGET cryptopp)
    set_target_properties (cryptopp PROPERTIES COMPILE_DEFINITIONS "${_TEMP}")
    set_target_properties (cryptopp PROPERTIES DEBUG_POSTFIX d)
  endif (TARGET cryptopp)

  if (TARGET cryptlib)
    set_target_properties (cryptlib PROPERTIES DEBUG_POSTFIX d)
  endif (TARGET cryptlib)

  if (CRYPTOPP_BUILD_TESTS)
    set_target_properties (crypttest PROPERTIES COMPILE_DEFINITIONS _CONSOLE)

    set (_TEMP CRYPTOPP_DLL_ONLY _CONSOLE)
    set_target_properties (dlltest PROPERTIES COMPILE_DEFINITIONS
      "${_TEMP};CRYPTOPP_NO_LIB")
  endif (CRYPTOPP_BUILD_TESTS)
endif (WIN32)

IF (WIN32)
  if (TARGET cryptopp)
    target_link_libraries (cryptopp Ws2_32)
  endif (TARGET cryptopp)

  if (TARGET cryptlib)
    target_link_libraries (cryptlib Ws2_32)
  endif (TARGET cryptlib)
endif (WIN32)

if (_CRYPTOPP_INSTALL_TARGETS)
  #set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES PUBLIC_HEADER
  #  "${PROJECT_H}")

  #install (TARGETS ${_CRYPTOPP_INSTALL_TARGETS}
  #  EXPORT cryptopp-targets
  #  RUNTIME DESTINATION bin
  #  LIBRARY DESTINATION lib
  #  ARCHIVE DESTINATION lib
  #  PUBLIC_HEADER DESTINATION include/cryptopp)
endif (_CRYPTOPP_INSTALL_TARGETS)

if (TARGET crypttest)
  enable_testing ()
  # Run validation tests
  add_test (NAME cryptopp-validation COMMAND crypttest v WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR})
endif (TARGET crypttest)

# Intel compiler, Windows
if (CMAKE_CXX_COMPILER MATCHES icl)
  set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES COMPILE_FLAGS
    /Qopenmp)
# Intel compiler, non-Windows
elseif (CMAKE_CXX_COMPILER MATCHES icc)
  set_target_properties (${_CRYPTOPP_INSTALL_TARGETS} PROPERTIES COMPILE_FLAGS
    -fopenmp)
endif (CMAKE_CXX_COMPILER MATCHES icl)

if (TARGET crypttest)
  get_target_property (crypttest RUNTIME_OUTPUT_DIRECTORY _DATA_DIR)

  if (NOT _DATA_DIR)
    set (_DATA_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  endif (NOT _DATA_DIR)

  set (_DATA_DIR ${_DATA_DIR}/${CMAKE_CFG_INTDIR})

  add_custom_command (TARGET crypttest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/TestData ${_DATA_DIR}/TestData
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/TestVectors ${_DATA_DIR}/TestVectors
    COMMENT "Copying cryptopp test data")
endif (TARGET crypttest)

set (_CRYPTOPP_VERSION_HEADER ${CMAKE_CURRENT_LIST_DIR}/config.h)

file (STRINGS ${_CRYPTOPP_VERSION_HEADER} _CRYPTOPP_VERSION_TMP REGEX
  "^#define CRYPTOPP_VERSION[ \t]+[0-9]+$")

string (REGEX REPLACE
  "^#define CRYPTOPP_VERSION[ \t]+([0-9]+)" "\\1" _CRYPTOPP_VERSION_TMP
  ${_CRYPTOPP_VERSION_TMP})

string (REGEX REPLACE "([0-9]+)[0-9][0-9]" "\\1" CRYPTOPP_VERSION_MAJOR
  ${_CRYPTOPP_VERSION_TMP})
string (REGEX REPLACE "[0-9]([0-9])[0-9]" "\\1" CRYPTOPP_VERSION_MINOR
  ${_CRYPTOPP_VERSION_TMP})
string (REGEX REPLACE "[0-9][0-9]([0-9])" "\\1" CRYPTOPP_VERSION_PATCH
  ${_CRYPTOPP_VERSION_TMP})

configure_file (cryptopp-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-config.cmake @ONLY)
configure_file (cryptopp-config-build.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-config-build.cmake @ONLY)
configure_file (cryptopp-config-version.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-config-version.cmake @ONLY)

export (TARGETS ${_CRYPTOPP_INSTALL_TARGETS} FILE
  ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-targets.cmake)
export (PACKAGE CryptoPP)

#install (EXPORT cryptopp-targets DESTINATION . COMPONENT Development)
#install (FILES
#    ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-config.cmake
#    ${CMAKE_CURRENT_BINARY_DIR}/cryptopp-config-version.cmake
#    DESTINATION . COMPONENT Development)
