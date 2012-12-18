#
# Copyright (c) 2006, Alexander Neundorf <neundorf@kde.org>
# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(MACRO_CHECK_GCC_VISIBILITY GccVisibility)
  if (CMAKE_COMPILER_IS_GNUCXX)
   include(CheckCXXCompilerFlag)
   include(MacroEnsureVersion)
   # visibility support
   check_cxx_compiler_flag(-fvisibility=hidden ${GccVisibility})

   # get the gcc version
   exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE _gcc_version_info)

   string (REGEX MATCH "[345]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
   if (NOT _gcc_version)
   
      # clang reports: clang version 1.1 (trunk 95754)
      string (REGEX MATCH "clang version ([123]\\.[0-9])" _gcc_version "${_gcc_version_info}")
      if ( _gcc_version )
        string(REGEX REPLACE "clang version (.*)" "\\1.0" _gcc_version "${_gcc_version}" )
      endif ( _gcc_version )
   
      # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
      if (NOT _gcc_version)
        string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
      endif (NOT _gcc_version)
   endif (NOT _gcc_version)
   
   

   macro_ensure_version("4.1.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_1)
   macro_ensure_version("4.2.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_2)

   set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
   if (GCC_IS_NEWER_THAN_4_1)
      exec_program(${CMAKE_C_COMPILER} ARGS -v OUTPUT_VARIABLE _gcc_alloc_info)
      string(REGEX MATCH "(--enable-libstdcxx-allocator=mt)" _GCC_COMPILED_WITH_BAD_ALLOCATOR "${_gcc_alloc_info}")
   endif (GCC_IS_NEWER_THAN_4_1)

   if (${GccVisibility} AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
      set (KDE4_C_FLAGS "${KDE4_C_FLAGS}" "-fvisibility=hidden")

      if (GCC_IS_NEWER_THAN_4_2)
          set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
      endif (GCC_IS_NEWER_THAN_4_2)
   else (${GccVisibility} AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
      set (${GccVisibility} 0)
   endif (${GccVisibility} AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

  else (CMAKE_COMPILER_IS_GNUCXX)
    set(${GccVisibility} FALSE)
  endif (CMAKE_COMPILER_IS_GNUCXX)
endmacro(MACRO_CHECK_GCC_VISIBILITY)
