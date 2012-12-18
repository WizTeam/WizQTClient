MACRO ( DEFINE_FLOAT )
    #find float symbol
    CHECK_CXX_SOURCE_COMPILES("
    	#include <math.h>
    	int main(){ float_t x=0.0; }" 
    	HAVE_SYMBOL_FLOAT_T)
    
    IF ( NOT HAVE_SYMBOL_FLOAT_T )
        #try using FLT_EVAL_METHOD
        CHECK_CXX_SOURCE_COMPILES("
    	    #define FLT_EVAL_METHOD 2
    	    #include <math.h>
    	    int main(){ float_t x=0.0; }" 
    	    HAVE_SYMBOL_FLOAT_T)
        
        IF ( HAVE_SYMBOL_FLOAT_T )
            SET ( _FLT_EVAL_METHOD 2 )
            SET ( TYPE_FLOAT_T "/* undef float_t*/" )
        ENDIF ( HAVE_SYMBOL_FLOAT_T )
    
    ELSE ( NOT HAVE_SYMBOL_FLOAT_T )
        SET ( TYPE_FLOAT_T "/* undef float_t*/" )
    ENDIF ( NOT HAVE_SYMBOL_FLOAT_T )
    
    IF ( NOT HAVE_SYMBOL_FLOAT_T )
        #todo: could we do a better guess here?
        SET ( TYPE_FLOAT_T "typedef double float_t;" )
    ENDIF ( NOT HAVE_SYMBOL_FLOAT_T )
ENDMACRO ( DEFINE_FLOAT )
