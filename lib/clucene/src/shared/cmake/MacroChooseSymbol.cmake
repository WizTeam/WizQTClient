
INCLUDE (CheckSymbolExists)
INCLUDE (Macro_ChooseStatus)

MACRO(CHOOSE_SYMBOL name options)
    #note: don't add windows.h to this list, since we don't want to find things 
    #in there (bcc might make you think otherwise, but it's true!)...
    
    IF ( CYGWIN )
        #cygwin requires this to determine some things...
        SET ( CMAKE_REQUIRED_DEFINITIONS "-D_WIN32")
    ENDIF( CYGWIN )
    
    IF ( HAVE_LIMITS_H )
    	SET (CHOOSE_SYMBOL_INCLUDES "${CHOOSE_SYMBOL_INCLUDES};limits.h")
    ENDIF ( HAVE_LIMITS_H )
    IF ( HAVE_STAT_H )
    	SET (CHOOSE_SYMBOL_INCLUDES "${CHOOSE_SYMBOL_INCLUDES};stat.h")
    ENDIF ( HAVE_STAT_H )
    IF ( HAVE_SYS_STAT_H )
    	SET (CHOOSE_SYMBOL_INCLUDES "${CHOOSE_SYMBOL_INCLUDES};sys/stat.h")
    ENDIF ( HAVE_SYS_STAT_H )
    IF ( HAVE_TCHAR_H )
    	SET (CHOOSE_SYMBOL_INCLUDES "${CHOOSE_SYMBOL_INCLUDES};tchar.h")
    ENDIF ( HAVE_TCHAR_H )
    
    SET (CHOOSE_SYMBOL_INCLUDES "${CHOOSE_SYMBOL_INCLUDES};fcntl.h")

    STRING(TOUPPER ${name} NAME)
    FOREACH(option ${options})
        IF ( NOT SYMBOL_${NAME} )
            STRING(TOUPPER ${option} OPTION)
            
            _CHOOSE_STATUS(PROGRESS ${name} "symbol" )
    	    CHECK_SYMBOL_EXISTS (${option} "${CHOOSE_SYMBOL_INCLUDES}" _CL_HAVE_SYMBOL_${OPTION})

            IF ( _CL_HAVE_SYMBOL_${OPTION} )
            	IF ( option STREQUAL ${name} )
					#already have it, ignore this...
					SET (SYMBOL_${NAME} "/* undef ${name} ${option} */" )
				ELSE ( option STREQUAL ${name} )
					SET (SYMBOL_${NAME} "#define ${name} ${option}")
				ENDIF ( option STREQUAL ${name} )
				
				IF ( NOT "${ARGV2}" STREQUAL "" )
					SET ( ${ARGV2} "${option}" )
				ENDIF ( NOT "${ARGV2}" STREQUAL "" )
				_CHOOSE_STATUS(END ${name} "symbol" ${option})
    	    ENDIF ( _CL_HAVE_SYMBOL_${OPTION} )
    	ENDIF( NOT SYMBOL_${NAME} )
    ENDFOREACH(option ${options})
    
    IF ( NOT SYMBOL_${NAME} )
        SET (SYMBOL_${NAME} "/* undef ${name} */" )
        _CHOOSE_STATUS(END ${name} "symbol" "not found" )
    ELSE ( NOT SYMBOL_${NAME} )
        SET (HAVE_SYMBOL_${NAME} 1)
    ENDIF ( NOT SYMBOL_${NAME} )
    
    SET (CHOOSE_SYMBOL_INCLUDES)
    SET (CMAKE_REQUIRED_DEFINITIONS)
ENDMACRO(CHOOSE_SYMBOL)
