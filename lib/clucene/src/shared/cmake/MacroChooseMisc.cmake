#macro that sets result to one of results (and have_result to true) when the first option TRY_COMPILE is successful.
#this is mainly to get around the fact that some compilers couldn't do sizeof(structs), therefore CHOOSE_TYPE wasn't going to work
#syntax: CHOOSE_TYPE ( TYPE_X HAVE_TYPE_X "struct stat x\;" "typedef stat cl_stat" "stat" ) (the last 3 options can be repeated indefinately in pairs of 3)

INCLUDE (Macro_ChooseStatus)

MACRO(CHOOSE_MISC result have_result )
    IF ( HAVE_SYS_STAT_H )
    	SET (HEADERS "${HEADERS}
    	    #include \"sys/stat.h\"")
    ENDIF ( HAVE_SYS_STAT_H )
    IF ( HAVE_SYS_TIMEB_H )
        SET (HEADERS "${HEADERS}
    	    #include \"sys/timeb.h\"")
    ENDIF ( HAVE_SYS_TIMEB_H )
    
    SET ( M_TEST )
    SET ( M_SUCCESS )
    SET ( M_name )
    SET ( M_RESULT )
    SET ( M_HAVE_RESULT )

    FOREACH(optiong ${ARGV})
        #MESSAGE( STATUS  "m ${optiong}" )
        SET ( m_continue )
        
        #this will only happen the first round...
        IF ( NOT m_continue AND NOT M_RESULT )
            SET ( M_RESULT "${optiong}")
            SET ( m_continue 1 )
        ENDIF ( NOT m_continue AND NOT M_RESULT )
        IF ( NOT m_continue AND NOT M_HAVE_RESULT )
            SET ( M_HAVE_RESULT "${optiong}")
            SET ( m_continue 1 )
        ENDIF ( NOT m_continue AND NOT M_HAVE_RESULT )
        
        #test and success in pairs...
        IF ( NOT m_continue AND NOT M_TEST )
            SET ( M_TEST "${optiong}")
            SET ( m_continue 1 )
        ENDIF ( NOT m_continue AND NOT M_TEST )
        IF ( NOT m_continue AND NOT M_SUCCESS )
            SET ( M_SUCCESS "${optiong}")
            SET ( m_continue 1 )
        ENDIF ( NOT m_continue AND NOT M_SUCCESS )
        IF ( NOT m_continue AND NOT M_name )
            SET ( M_name "${optiong}")
            STRING(TOUPPER ${M_name} M_NAME)
            #this one doesn't continue...
        ENDIF ( NOT m_continue AND NOT M_name )
        
        IF ( NOT m_continue )
            _CHOOSE_STATUS(PROGRESS ${M_HAVE_RESULT} "option" ${M_TEST})
           
            IF ( NOT ${M_HAVE_RESULT} )
                CHECK_CXX_SOURCE_COMPILES(
                    "${HEADERS}
            	    int main(){
            	        ${M_TEST} 
            	    }" 
            	    _CL_HAVE_OPTION_${M_NAME} )
            	
            	IF ( _CL_HAVE_OPTION_${M_NAME} )
                    SET (${M_RESULT} "${M_SUCCESS}")
                    SET (${M_HAVE_RESULT} 1 )
    				_CHOOSE_STATUS(END ${M_HAVE_RESULT} "option" "${M_name}")
        	    ENDIF ( _CL_HAVE_OPTION_${M_NAME} )
            ENDIF ( NOT ${M_HAVE_RESULT} )
            
            #reset for next round
            SET ( M_TEST )
            SET ( M_SUCCESS )
            SET ( M_name )
        ENDIF ( NOT m_continue )
    ENDFOREACH(optiong)
    
    IF ( NOT ${M_HAVE_RESULT} )
        _CHOOSE_STATUS(END ${M_HAVE_RESULT} option "not found")
    ENDIF ( NOT ${M_HAVE_RESULT} )
    
    SET(CMAKE_EXTRA_INCLUDE_FILES) 
ENDMACRO(CHOOSE_MISC)
