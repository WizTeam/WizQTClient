#check if we can do try and catch.
#bit useless, since we don't have any alternatives to try and catch currently

MACRO ( CHECK_HAVE_FUNCTION_TRY_BLOCKS result )
    #check for try/catch blocks
    CHECK_CXX_SOURCE_RUNS("
    	void foo() { try{ return; } catch( ... ){} }
    	int main(){ foo(); return 0; }" ${result})
    IF ( NOT ${result} )
    	SET ( ${result} 1 FORCE)
    ENDIF ( NOT ${result} )
ENDMACRO ( CHECK_HAVE_FUNCTION_TRY_BLOCKS )
