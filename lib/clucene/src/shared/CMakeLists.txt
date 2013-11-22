PROJECT(clucene-shared)

#define command line options
INCLUDE (DefineOptions)
DEFINE_OPTIONS(EXTRA_OPTIONS EXTRA_LIBS)
ADD_DEFINITIONS(${EXTRA_OPTIONS} -DMAKE_CLUCENE_SHARED_LIB)

# include specific modules
set(CMAKE_MODULE_PATH "${clucene-shared_SOURCE_DIR}/cmake")

INCLUDE (CheckIncludeFiles)
INCLUDE (CheckIncludeFileCXX)
INCLUDE (CheckCXXSourceCompiles)
INCLUDE (CheckCXXSourceRuns)
INCLUDE (CheckFunctionExists)

#local macros
INCLUDE (MacroMustDefine)
INCLUDE (MacroChooseType)
INCLUDE (MacroChooseMisc)
INCLUDE (MacroChooseFunction)
INCLUDE (MacroChooseSymbol)
INCLUDE (MacroCheckGccVisibility)

INCLUDE (DefineFloat)
INCLUDE (DefineDword)
INCLUDE (DefineLongLongSyntax)
INCLUDE (DefineStaticSyntax)
INCLUDE (DefineMAXPATHValue)

INCLUDE (CheckFloatByte)
INCLUDE (CheckErrorHandling)
INCLUDE (CheckHashmaps)
INCLUDE (CheckNamespace)
INCLUDE (CheckSnprintf)
INCLUDE (CheckStdCallFunctionExists)

find_package(Threads REQUIRED)
INCLUDE (CheckPthread)
INCLUDE (CheckAtomicFunctions)

#find_package(ZLIB)
#IF ( ZLIB_FOUND )
#  SET ( EXTRA_LIBS ${EXTRA_LIBS} ${ZLIB_LIBRARY} )
#ELSE ( ZLIB_FOUND )
#  MESSAGE( "ZLIB not found, using local: ${clucene-ext_SOURCE_DIR}/zlib" )
#  SET(ZLIB_INCLUDE_DIR ${clucene-ext_SOURCE_DIR}/zlib )
#  SET(ZLIB_LIBRARY ${clucene-ext_BINARY_DIR})
#ENDIF ( ZLIB_FOUND )
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

########################################################################
# test for headers
########################################################################

CHECK_INCLUDE_FILES ("sys/time.h;time.h" _CL_TIME_WITH_SYS_TIME)

CHECK_REQUIRED_HEADERS ( stdlib.h stdarg.h stdio.h stddef.h ctype.h algorithm  
						 functional map vector list set math.h fcntl.h limits.h)
						 
CHECK_OPTIONAL_HEADERS ( string.h sys/time.h memory.h sys/types.h 
                        stdint.h unistd.h io.h direct.h sys/dir.h sys/ndir.h dirent.h wctype.h fcntl.h
                        stat.h sys/stat.h stdexcept errno.h fcntl.h windef.h windows.h wchar.h 
                        hash_map hash_set ext/hash_map ext/hash_map tr1/unordered_set tr1/unordered_map
                        sys/timeb.h tchar.h strings.h stdexcept sys/mman.h winerror.h )


########################################################################
# test for types
########################################################################
#find int_t types
CHOOSE_TYPE(int8_t      1 signed   "int8_t;char")
CHOOSE_TYPE(uint8_t     1 unsigned "uint8_t;char")
CHOOSE_TYPE(int16_t     2 signed   "int16_t;short")
CHOOSE_TYPE(uint16_t    2 unsigned "uint16_t;short")
CHOOSE_TYPE(int32_t     4 signed   "int32_t;int;long")
CHOOSE_TYPE(uint32_t    4 unsigned "uint32_t;int;long")
CHOOSE_TYPE(int64_t     8 signed   "int64_t;long long;__int64")
CHOOSE_TYPE(uint64_t    8 unsigned "uint64_t;long long;__int64")

CHOOSE_TYPE(size_t      -1 unsigned "size_t;int" )
CHOOSE_MISC(TYPE__TIMEB
            HAVE_TYPE__TIMEB
            "struct _timeb x\;" "/* #undef _timeb */" "_timeb"
            "struct timeb x\;" "#define _timeb timeb" "timeb"
             )

#run macro for checking float. (and _FLT_EVAL_METHOD)
DEFINE_FLOAT()
DEFINE_DWORD()

########################################################################
# Test for functions
########################################################################


CHECK_REQUIRED_FUNCTIONS(  printf strftime wcscpy wcsncpy wcscat wcschr wcsstr wcslen 
                           wcscmp wcsncmp wcscspn )

#todo: wcstoq is bsd equiv of wcstoll, we can use that...
CHECK_OPTIONAL_FUNCTIONS( wcsupr wcscasecmp wcsicmp wcstoll wprintf lltow 
    wcstod wcsdup strupr strlwr lltoa strtoll gettimeofday _vsnwprintf mmap "MapViewOfFile(0,0,0,0,0)"
)

#make decisions about which functions to use...
CHOOSE_FUNCTION(fileHandleStat "fstati64;_fstati64;fstat64;fstat;_fstat")
IF ( _CL_HAVE_FUNCTION_FSTATI64 OR _CL_HAVE_FUNCTION__FSTATI64 OR _CL_HAVE_FUNCTION_FSTAT64 )
    SET ( USE_STAT64 1 )
ENDIF ( _CL_HAVE_FUNCTION_FSTATI64 OR _CL_HAVE_FUNCTION__FSTATI64 OR _CL_HAVE_FUNCTION_FSTAT64 )

IF ( USE_STAT64 )
    CHOOSE_FUNCTION(fileStat "stati64;_stati64;stat64;stat;_stat")
    CHOOSE_FUNCTION(fileSize "filelengthi64;_filelengthi64;filelength;_filelength" "#define fileSize CL_NS(util)::Misc::filelength")
    CHOOSE_FUNCTION(fileSeek "lseeki64;_lseeki64;lseek64;lseek;_lseek")
    CHOOSE_MISC(TYPE_CL_STAT_T
                HAVE_TYPE_CL_STAT_T
                "struct stati64 x\;" "#define cl_stat_t stati64" "stati64"
                "struct _stati64 x\;" "#define cl_stat_t _stati64" "_stati64"
                "struct stat64 x\;" "#define cl_stat_t stat64" "stat64"
                "struct stat x\;" "#define cl_stat_t stat" "stat"
                "struct _stat x\;" "#define cl_stat_t _stat" "_stat"
                 )
ELSE( USE_STAT64 )
    #borland doesn't have a fstat64, so we have to fallback to non 64 bit everything...
    CHOOSE_FUNCTION(fileStat "stat;_stat")
    CHOOSE_FUNCTION(fileSize "filelength;_filelength" "#define fileSize CL_NS(util)::Misc::filelength")
    CHOOSE_FUNCTION(fileSeek "lseek;_lseek")
    CHOOSE_MISC(TYPE_CL_STAT_T
                HAVE_TYPE_CL_STAT_T
                "struct stat x\;" "#define cl_stat_t stat" "stat"
                "struct _stat x\;" "#define cl_stat_t _stat" "_stat"
                )
ENDIF( USE_STAT64 )

#ftell (and probably soon ftell64) are POSIX standard functions, but tell and
#tell64 are not, so we define fileTell in terms of fileSeek.
CHOOSE_FUNCTION(fileTell "telli64;_telli64;tell64;tell;_tell" "#define fileTell(fhandle) fileSeek(fhandle, 0, SEEK_CUR)")

CHOOSE_FUNCTION(_realpath "realpath" "#define _realpath(rel,abs) ::_fullpath(abs,rel,CL_MAX_PATH)")
CHOOSE_FUNCTION(_rename "rename")
CHOOSE_FUNCTION(_close "_close((int)0);close")
CHOOSE_FUNCTION(_read "_read((int)0, (void*)0, (unsigned int)0);read")
CHOOSE_FUNCTION(_cl_open "_open(0,0,0);open")
CHOOSE_FUNCTION(_write "_write((int)0, (const void*)0, (unsigned int)0);write")
CHOOSE_FUNCTION(_unlink "_unlink((const char*)0);unlink")
CHOOSE_FUNCTION(_ftime "_ftime(0);ftime")
CHOOSE_FUNCTION(_mkdir "_mkdir((const char*)0)" "#define _mkdir(x) mkdir(x,0777)")
CHOOSE_FUNCTION(SLEEPFUNCTION "usleep;Sleep(0);_sleep")

CHOOSE_FUNCTION(_snprintf "snprintf;_snprintf")
CHOOSE_FUNCTION(_snwprintf "snwprintf;_snwprintf")

########################################################################
# test for symbols
########################################################################
CHOOSE_SYMBOL (_O_RANDOM "_O_RANDOM;O_RANDOM")
CHOOSE_SYMBOL (_O_BINARY "_O_BINARY;O_BINARY")
CHOOSE_SYMBOL (_S_IREAD  "_S_IREAD;S_IREAD")
CHOOSE_SYMBOL (_S_IWRITE "_S_IWRITE;S_IWRITE")


#define defaults
IF ( NOT HAVE_SYMBOL__O_RANDOM )
	SET (SYMBOL__O_RANDOM "#define _O_RANDOM 0")
ENDIF ( NOT HAVE_SYMBOL__O_RANDOM )

IF ( NOT HAVE_SYMBOL__O_BINARY )
	SET (SYMBOL__O_BINARY "#define _O_BINARY 0")
ENDIF ( NOT HAVE_SYMBOL__O_BINARY)

IF ( NOT HAVE_SYMBOL__S_IREAD )
	SET (SYMBOL__S_IREAD "#define _S_IREAD 0333")
ENDIF ( NOT HAVE_SYMBOL__S_IREAD )

IF ( NOT HAVE_SYMBOL__S_IWRITE )
	SET (SYMBOL__S_IWRITE "#define _S_IWRITE 0333")
ENDIF ( NOT HAVE_SYMBOL__S_IWRITE )

#try and figure out the actual value of what _CL_MAX_PATH is
DEFINE_MAXPATH_VALUE (SYMBOL_CL_MAX_PATH)


########################################################################
# test for tchar replacments
########################################################################
IF ( ENABLE_ASCII_MODE )
    CHOOSE_TYPE(TCHAR   1 "" "TCHAR;char" SYMBOL_TCHAR )
ELSE ( ENABLE_ASCII_MODE )
    CHOOSE_TYPE(TCHAR   -1 "" "TCHAR;wchar_t;unsigned short" SYMBOL_TCHAR)
ENDIF ( ENABLE_ASCII_MODE )
IF ( NOT SYMBOL_TCHAR STREQUAL "TCHAR" )
    SET( SYMBOL_TCHAR "#define TCHAR ${SYMBOL_TCHAR}" )
ELSE ( NOT SYMBOL_TCHAR STREQUAL "TCHAR" )
    SET( SYMBOL_TCHAR "/* #undef TCHAR */" )
ENDIF ( NOT SYMBOL_TCHAR STREQUAL "TCHAR" )

CHOOSE_SYMBOL (_T "_T" SYMBOL__T)
IF ( NOT HAVE_SYMBOL__T )
IF ( ENABLE_ASCII_MODE )
    SET (SYMBOL__T "#define _T(x) x")
ELSE ( ENABLE_ASCII_MODE )
    SET (SYMBOL__T "#define _T(x) L ## x")
ENDIF ( ENABLE_ASCII_MODE )
ELSE ( NOT HAVE_SYMBOL__T )
    SET( SYMBOL__T "/* #undef _T */" )
ENDIF ( NOT HAVE_SYMBOL__T )


########################################################################
# test for compiler capabilities.
########################################################################

#check for pthreads
IF ( CMAKE_USE_WIN32_THREADS_INIT )
    SET ( _CL_HAVE_WIN32_THREADS 1 )
ENDIF ( CMAKE_USE_WIN32_THREADS_INIT )
IF ( CMAKE_USE_PTHREADS_INIT )
    SET ( _CL_HAVE_PTHREAD 1 )
ENDIF (CMAKE_USE_PTHREADS_INIT)
IF ( CMAKE_USE_SPROC_INIT OR CMAKE_HP_PTHREADS_INIT )
    MESSAGE(FATAL_ERROR "Threads library not implemented")
ENDIF( CMAKE_USE_SPROC_INIT OR CMAKE_HP_PTHREADS_INIT )

#define if we have pthreads with recursive capabilities
CHECK_PTHREAD_RECURSIVE(_CL_HAVE_PTHREAD _CL_HAVE_PTHREAD_MUTEX_RECURSIVE)

CHECK_HAVE_GCC_ATOMIC_FUNCTIONS(_CL_HAVE_GCC_ATOMIC_FUNCTIONS)

#see if we can hide all symbols by default...
MACRO_CHECK_GCC_VISIBILITY(_CL_HAVE_GCCVISIBILITYPATCH)

#Check that we can handle try/catch
CHECK_HAVE_FUNCTION_TRY_BLOCKS (_CL_HAVE_TRY_BLOCKS)

#check that we support new float byte<->float conversions
CHECK_FLOAT_BYTE_WORKS(_CL_HAVE_NO_FLOAT_BYTE, 1)

#check how to use hashmaps
CHECK_HASH_MAPS (CL_NS_HASHING_VALUE LUCENE_DISABLE_HASHING)

#check that we have namespace support
CHECK_NAMESPACE (_CL_HAVE_NAMESPACES)

#check if snprintf functions are buggy
CHECK_SNPRINTF()

#define how we are going to define 64bit numbers
DEFINE_LONGLONG_SYNTAX()

#define how to define a static const (or fallback to enum)
DEFINE_STATIC_SYNTAX()

#test for ansi for scope (needed for msvc6)
INCLUDE(TestForANSIForScope)
IF ( CMAKE_ANSI_FOR_SCOPE )
    SET ( CMAKE_ANSI_FOR_SCOPE 1 )
ELSE ( CMAKE_ANSI_FOR_SCOPE )
    SET ( CMAKE_ANSI_FOR_SCOPE 0 )
ENDIF ( CMAKE_ANSI_FOR_SCOPE )


#check that all these definitions are set, or fail...
MUSTDEFINE_VAR("HAVE_TYPE_INT8_T;HAVE_TYPE_UINT8_T;HAVE_TYPE_INT16_T;HAVE_TYPE_UINT16_T;HAVE_TYPE_UINT16_T;" )
MUSTDEFINE_VAR("HAVE_TYPE_INT32_T;HAVE_TYPE_UINT32_T;HAVE_TYPE_INT64_T;HAVE_TYPE_UINT64_T;HAVE_TYPE_SIZE_T;HAVE_TYPE_CL_STAT_T")
MUSTDEFINE_VAR("_CL_HAVE_TRY_BLOCKS")

#must have timeb OR GETTIMEOFDAY
IF ( NOT HAVE_TYPE__TIMEB AND NOT _CL_HAVE_FUNCTION_GETTIMEOFDAY )
        MESSAGE( FATAL_ERROR "timeb or gettimeofday must be available." )
ENDIF ( NOT HAVE_TYPE__TIMEB AND NOT _CL_HAVE_FUNCTION_GETTIMEOFDAY )

#now write out our configuration....
CONFIGURE_FILE(${clucene-shared_SOURCE_DIR}/CLucene/clucene-config.h.cmake ${clucene-shared_BINARY_DIR}/CLucene/clucene-config.h)
CONFIGURE_FILE(${clucene-shared_SOURCE_DIR}/CLucene/_clucene-config.h.cmake ${clucene-shared_BINARY_DIR}/CLucene/_clucene-config.h)

#add the files to our groups
SOURCE_GROUP("config" ./CLucene/config/*)
SOURCE_GROUP("debug" ./CLucene/debug/*)
SOURCE_GROUP("util" ./CLucene/util/*)
SOURCE_GROUP("zlib" ./zlib/*)

MACRO (GET_SHARED_FILES result)
    IF ( "" STREQUAL "${ARGV2}" )
        SET ( rel ${clucene-shared_SOURCE_DIR} )
    ELSE ( "" STREQUAL "${ARGV2}" )
        SET ( rel ${ARGV2} )
    ENDIF ( "" STREQUAL "${ARGV2}" )

    SET(${result}
    	${rel}/CLucene/SharedHeader.cpp
   	  ${rel}/CLucene/config/gunichartables.cpp
    	${rel}/CLucene/config/repl_tcslwr.cpp
    	${rel}/CLucene/config/repl_tcstoll.cpp
    	${rel}/CLucene/config/repl_tcscasecmp.cpp
    	${rel}/CLucene/config/repl_tprintf.cpp
    	${rel}/CLucene/config/repl_lltot.cpp
    	${rel}/CLucene/config/repl_tcstod.cpp
    	${rel}/CLucene/config/utf8.cpp
    	${rel}/CLucene/config/threads.cpp
    	${rel}/CLucene/debug/condition.cpp
    	${rel}/CLucene/util/StringBuffer.cpp
    	${rel}/CLucene/util/Misc.cpp
      ${rel}/CLucene/util/dirent.cpp
    )

    #IF ( NOT ZLIB_FOUND )
    #  SET(${result} ${${result}}
    #    ${clucene-ext_SOURCE_DIR}/zlib/adler32.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/compress.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/crc32.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/deflate.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/gzio.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/inffast.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/inflate.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/inftrees.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/trees.c
    #    ${clucene-ext_SOURCE_DIR}/zlib/zutil.c
    #  )
    #ENDIF ( NOT ZLIB_FOUND )
ENDMACRO (GET_SHARED_FILES)

GET_SHARED_FILES(clucene_shared_Files ".")

#find our headers
file(GLOB_RECURSE HEADERS ${clucene-shared_SOURCE_DIR}/*.h)

if (CLUCENE_BUILD_SHARED_LIBRARIES)
    add_library(clucene-shared SHARED
        ${clucene_shared_Files} ${HEADERS}
    )

    #set properties on the libraries
    SET_TARGET_PROPERTIES(clucene-shared PROPERTIES
        VERSION ${CLUCENE_VERSION}
        SOVERSION ${CLUCENE_SOVERSION}
        COMPILE_DEFINITIONS_DEBUG _DEBUG
    )
    TARGET_LINK_LIBRARIES(clucene-shared ${CMAKE_THREAD_LIBS_INIT})
    IF ( EXTRA_LIBS )
      TARGET_LINK_LIBRARIES(clucene-shared ${EXTRA_LIBS})
    ENDIF ( EXTRA_LIBS )

    #install(TARGETS clucene-shared
    #        DESTINATION ${LIB_DESTINATION}
    #        COMPONENT development )
endif()

IF ( BUILD_STATIC_LIBRARIES )
	add_library(clucene-shared-static STATIC
		${clucene_shared_Files} ${HEADERS}
	)
	
	SET_TARGET_PROPERTIES(clucene-shared-static PROPERTIES
	    VERSION ${CLUCENE_VERSION}
	    SOVERSION ${CLUCENE_SOVERSION}
	    COMPILE_DEFINITIONS_DEBUG _DEBUG
	)
  TARGET_LINK_LIBRARIES(clucene-shared-static ${CMAKE_THREAD_LIBS_INIT})
        #install(TARGETS clucene-shared-static
        #        DESTINATION ${LIB_DESTINATION}
        #        COMPONENT runtime )
ENDIF ( BUILD_STATIC_LIBRARIES )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
set_target_properties(clucene-shared-static PROPERTIES
                        COMPILE_FLAGS "-Wno-deprecated-declarations -Wno-#warnings")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
set_source_files_properties(${rel}/CLucene/config/gunichartables.cpp PROPERTIES
                        COMPILE_FLAGS "-Wno-cpp")
endif()
