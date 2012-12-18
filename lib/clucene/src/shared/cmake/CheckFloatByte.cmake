# - Check if our methods for converting from floats to bytes and back work.
# CHECK_FLOAT_BYTE_WORKS(RESULT reverse)
# reverse: set to false if the check succeeds
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link
#  CMAKE_EXTRA_INCLUDE_FILES = list of extra includes to check in

MACRO(CHECK_FLOAT_BYTE_WORKS RESULT reverse)
  IF("${RESULT}" MATCHES "^${RESULT}$")
    MESSAGE(STATUS "Checking support new float byte<->float conversions")
    
    CONFIGURE_FILE("${clucene-shared_SOURCE_DIR}/cmake/CheckFloatByte.cpp.in"
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFloatByte.cpp" IMMEDIATE @ONLY)

    TRY_COMPILE(${RESULT}
      ${CMAKE_BINARY_DIR}
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckFloatByte.cpp"
      OUTPUT_VARIABLE OUTPUT)
    IF(${RESULT})
      MESSAGE(STATUS "Checking support new float byte<->float conversions - yes")
    ELSE(${RESULT})
      MESSAGE(STATUS "Checking support new float byte<->float conversions - no")
    ENDIF(${RESULT})

	 #reverse decision if required.
	 IF (${reverse})
		IF (${RESULT})
		  SET ( ${RESULT} 0 )
		ELSE (${RESULT})
		  SET ( ${RESULT} 1 )
		ENDIF (${RESULT})
	 ENDIF (${reverse})
  ENDIF("${RESULT}" MATCHES "^${RESULT}$")
ENDMACRO(CHECK_FLOAT_BYTE_WORKS)
