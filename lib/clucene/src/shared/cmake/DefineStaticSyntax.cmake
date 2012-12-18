#defines how we can define a static const (or fallback to using an enum)

MACRO ( DEFINE_STATIC_SYNTAX ) 

    #check which syntax of static const to use
    CHECK_CXX_SOURCE_RUNS("class x{public: static const int SCI=55; }; int main(){ x a; if ( a.SCI!=55 ) throw \"err\"; return 0; }" LUCENE_STATIC_CONSTANT_SYNTAX)
    IF ( LUCENE_STATIC_CONSTANT_SYNTAX )
        SET ( LUCENE_STATIC_CONSTANT_SYNTAX "static const type assignment")
    ELSE ( LUCENE_STATIC_CONSTANT_SYNTAX )
        SET ( LUCENE_STATIC_CONSTANT_SYNTAX "enum { assignment }")
    ENDIF ( LUCENE_STATIC_CONSTANT_SYNTAX )

ENDMACRO ( DEFINE_STATIC_SYNTAX ) 