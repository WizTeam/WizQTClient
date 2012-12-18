#define global options, this makes it easy to use ccmake, or the cmake gui
MACRO (DEFINE_OPTIONS extraOptions extraLibs)
  IF(ENABLE_DEBUG)
    SET (${extraOptions} "${${extraOptions}} -D_DEBUG")
  ENDIF(ENABLE_DEBUG)

  IF(ENABLE_MMAP)
    SET (${extraOptions} "${${extraOptions}} -DLUCENE_FS_MMAP")
  ENDIF(ENABLE_MMAP)

  IF(ENABLE_DMALLOC)
    SET (${extraOptions} "${${extraOptions}} -DDMALLOC")
    IF ( DISABLE_MULTITHREADING )
      SET (${extraLibs} ${${extraLibs}} "dmalloccxx")
    ELSE( DISABLE_MULTITHREADING )
      SET (${extraLibs} ${${extraLibs}} "dmallocthcxx")
    ENDIF ( DISABLE_MULTITHREADING )
  ENDIF(ENABLE_DMALLOC)

  IF(DISABLE_MULTITHREADING)
    SET (${extraOptions} "${${extraOptions}} -D_CL_DISABLE_MULTITHREADING")
  ELSE(DISABLE_MULTITHREADING)
    SET(${extraOptions} "${${extraOptions}} -D_REENTRANT")
  ENDIF(DISABLE_MULTITHREADING)

  IF(ENABLE_ASCII_MODE)
    SET (${extraOptions} "${${extraOptions}} -D_ASCII")
  ELSE(ENABLE_ASCII_MODE)
    SET (${extraOptions} "${${extraOptions}} -D_UCS2")
    SET (${extraOptions} "${${extraOptions}} -D_UNICODE")
  ENDIF(ENABLE_ASCII_MODE)

	IF ( MSVC80 OR MSVC90)
	    #todo: remove this once crt functions are fixed...
		SET (${extraOptions} "${${extraOptions}} -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE")
	ENDIF ( MSVC80 OR MSVC90 )
	
	IF(CYGWIN)
        ADD_DEFINITIONS(-D__LARGE64_FILES)
    ENDIF(CYGWIN)
    
    # calm mdown msvc
    IF(MSVC)
        IF ( NOT MSVC60 )
            #ADD_DEFINITIONS(-wd4251) # 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
            #ADD_DEFINITIONS(-wd4275) # non  DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
            #ADD_DEFINITIONS(-wd4309) # 'conversion' : truncation of constant value
            #ADD_DEFINITIONS(-wd4503) # decorated name length exceeded
            #ADD_DEFINITIONS(-wd4786) # identifier was truncated to '255' characters in the debug information
        ENDIF ( NOT MSVC60 )
    ENDIF(MSVC)

ENDMACRO (DEFINE_OPTIONS)
