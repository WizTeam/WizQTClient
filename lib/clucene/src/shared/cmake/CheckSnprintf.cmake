#checks if snprintf have bugs

MACRO ( CHECK_SNPRINTF )
    #check that our snprintf works correctly...
    IF ( _CL_HAVE_FUNCTION_SNPRINTF )
    CHECK_CXX_SOURCE_RUNS("
    	#include <stdio.h>
    	int main(){
    		char ovbuf[7];
    		int i;
    		for (i=0; i<7; i++) ovbuf[i]='x';
    		snprintf(ovbuf, 4,\"foo%s\", \"bar\");
    		if (ovbuf[5]!='x') return 1;
    		snprintf(ovbuf, 4,\"foo%d\", 666);
    		if (ovbuf[5]!='x') return 1;
    		return 0;
    	}" _CL_HAVE_NO_SNPRINTF_BUG) 
    	IF ( NOT _CL_HAVE_NO_SNPRINTF_BUG )
    		SET ( _CL_HAVE_SNPRINTF_BUG 1 )
    		MESSAGE ( FATAL_ERROR "snprintf has a bug, and we don't have a replacement!" )
    	ENDIF ( NOT _CL_HAVE_NO_SNPRINTF_BUG )
    ENDIF ( _CL_HAVE_FUNCTION_SNPRINTF )
    
    #check that our swnprintf works correctly...
    IF ( _CL_HAVE_FUNCTION_SNWPRINTF )
    CHECK_CXX_SOURCE_RUNS("
    	#include <stdio.h>
    	#include <wchar.h>
    	
    	int main(void)
    	{
        	wchar_t buf[5];
        	snwprintf(buf,5,L\"%s\",L\"foo\");
        	if ( wcslen(buf) != 3 )
        		return 1;
        	return 0;
    	}" _CL_HAVE_NO_SNWPRINTF_BUG) 
    	IF ( NOT _CL_HAVE_NO_SNWPRINTF_BUG )
    		SET ( _CL_HAVE_SNWPRINTF_BUG 1 )
    	ENDIF ( NOT _CL_HAVE_NO_SNWPRINTF_BUG )
    ENDIF ( _CL_HAVE_FUNCTION_SNWPRINTF )
ENDMACRO ( CHECK_SNPRINTF )
