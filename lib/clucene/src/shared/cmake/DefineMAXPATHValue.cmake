#checks if snprintf have bugs

MACRO ( DEFINE_MAXPATH_VALUE MaxPathValue )
# also check for MAXPATHLEN
#or this:
#path_max = pathconf (path, _PC_PATH_MAX);
  #if (path_max <= 0)
    #path_max = 4096;

	#use CHOOSE_SYMBOL mechanism to determine which variable to use...
	#CHOOSE_SYMBOL (_CL_MAX_PATH "PATH_MAX;MAX_PATH;_MAX_PATH;_POSIX_PATH_MAX" DefineMaxPathValue)
	#IF ( DefineMaxPathValue )
		#now try and find its value...
	#	Include ( MacroGetVariableValue )
		
	#	SET ( _CL_MAX_PATH_VALUE )
	#	GET_VARIABLE_VALUE (${DefineMaxPathValue} d _CL_MAX_PATH_VALUE)
	#	IF ( _CL_MAX_PATH_VALUE )
	#		SET( ${MaxPathValue} "#define ${MaxPathValue} ${_CL_MAX_PATH_VALUE}" )	
	#	ENDIF( _CL_MAX_PATH_VALUE )
	#ELSE ( DefineMaxPathValue )
	#	MESSAGE ( FATAL_ERROR "_CL_MAX_PATH could not be determined")
	#ENDIF ( DefineMaxPathValue )
	
    #SET (CMAKE_REQUIRED_DEFINITIONS)
    
    #HACK!!!
    #todo: fix this
    SET( ${MaxPathValue} "#define CL_MAX_PATH 4096")
ENDMACRO ( DEFINE_MAXPATH_VALUE )
