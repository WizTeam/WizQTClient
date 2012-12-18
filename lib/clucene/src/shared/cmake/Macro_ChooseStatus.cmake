#internal macro for choose_* macros.

MACRO(_CHOOSE_STATUS status name type)
    STRING(TOUPPER ${name} NAME)
    STRING(TOUPPER ${type} TYPE)

    IF ( ${status} STREQUAL "PROGRESS" )
        IF ( "" STREQUAL "${_CHOOSE_STATUS_${TYPE}_${NAME}}" )
            MESSAGE ( STATUS "Choosing ${type} for ${name}" )
            SET ( _CHOOSE_STATUS_${TYPE}_${NAME} "XXX" )
        ENDIF ( "" STREQUAL "${_CHOOSE_STATUS_${TYPE}_${NAME}}" )
    ENDIF ( ${status} STREQUAL "PROGRESS" )
    
    IF ( ${status} STREQUAL "END" )
        IF ( "XXX" STREQUAL "${_CHOOSE_STATUS_${TYPE}_${NAME}}" )
            MESSAGE ( STATUS "Choosing ${type} for ${name} - ${ARGV3}" )
            SET ( _CHOOSE_STATUS_${TYPE}_${NAME} ON CACHE INTERNAL "Chose ${type} for ${name} - ${ARGV3}" )
        ENDIF ( "XXX" STREQUAL "${_CHOOSE_STATUS_${TYPE}_${NAME}}" )
    ENDIF ( ${status} STREQUAL "END" )
ENDMACRO(_CHOOSE_STATUS)
