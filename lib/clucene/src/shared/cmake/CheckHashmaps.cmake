#check how to use hashmaps (which namespace)
#HashingValue is filled with namespace definition
#DisableHashing is set if we can't support hashing

INCLUDE (Macro_ChooseStatus)

#find hashing namespace (internal, use CHECK_HASH_MAPS) ...
MACRO(HASHMAP_TEST HashingValue namespace)
	IF ( NOT ${HashingValue} )
        IF ( _CL_HAVE_TR1_UNORDERED_MAP )
				SET(_CL_HASH_MAP unordered_map)
				SET(_CL_HASH_SET unordered_set)
            SET(CMAKE_REQUIRED_DEFINITIONS "-D_CL_HAVE_TR1_UNORDERED_MAP=1")
		  ELSE ( _CL_HAVE_TR1_UNORDERED_MAP )
			 IF ( _CL_HAVE_HASH_MAP )
				  SET(_CL_HASH_MAP hash_map)
				  SET(_CL_HASH_SET hash_set)
				  SET(CMAKE_REQUIRED_DEFINITIONS "-D_CL_HAVE_HASH_MAP=1")
			 ELSE ( _CL_HAVE_HASH_MAP )
				IF ( _CL_HAVE_EXT_HASH_MAP )
					 SET(_CL_HASH_MAP hash_map)
					 SET(_CL_HASH_SET hash_set)
					 SET(CMAKE_REQUIRED_DEFINITIONS "-D_CL_HAVE_EXT_HASH_MAP=1")
				ENDIF ( _CL_HAVE_EXT_HASH_MAP )
			 ENDIF ( _CL_HAVE_HASH_MAP )
        ENDIF ( _CL_HAVE_TR1_UNORDERED_MAP )
        
        STRING(TOUPPER ${namespace} NAMESPACE)
        STRING(REPLACE / _ NAMESPACE ${NAMESPACE})
        STRING(REPLACE : _ NAMESPACE ${NAMESPACE})
        	    
        CHECK_CXX_SOURCE_COMPILES("
#if defined(_CL_HAVE_HASH_MAP)
	#include <hash_map>
#elif defined(_CL_HAVE_EXT_HASH_MAP)
	#include <ext/hash_map>
#elif defined(_CL_HAVE_TR1_UNORDERED_MAP)
	#include <tr1/unordered_map>
#endif
int main() {  
    ${namespace}::${_CL_HASH_MAP}<int,char> a; 
    return 0; 
}
        " _CL_HAVE_${NAMESPACE}_HASHMAP)
        
        IF ( _CL_HAVE_${NAMESPACE}_HASHMAP )
            SET (${HashingValue} "${namespace}::func")
        ENDIF ( _CL_HAVE_${NAMESPACE}_HASHMAP )
    
    ENDIF ( NOT ${HashingValue} )
    
ENDMACRO(HASHMAP_TEST)


MACRO ( CHECK_HASH_MAPS HashingValue DisableHashing)
    IF ( _CL_HAVE_EXT_HASH_MAP OR _CL_HAVE_HASH_MAP )
        _CHOOSE_STATUS(PROGRESS "hashmaps" "namespace")
        HASHMAP_TEST (${HashingValue} "std::tr1")
        HASHMAP_TEST (${HashingValue} std)
        HASHMAP_TEST (${HashingValue} stdext)
        HASHMAP_TEST (${HashingValue} __gnu_cxx)
    
        IF ( NOT ${HashingValue} )
            _CHOOSE_STATUS(END "hashmaps" "namespace" "failed")
            SET(${DisableHashing} 1)
        ELSE ( NOT ${HashingValue} )
            _CHOOSE_STATUS(END "hashmaps" "namespace" ${${HashingValue}})
        ENDIF ( NOT ${HashingValue} )
    
    ENDIF ( _CL_HAVE_EXT_HASH_MAP OR _CL_HAVE_HASH_MAP )
    SET(CMAKE_REQUIRED_DEFINITIONS)
ENDMACRO ( CHECK_HASH_MAPS )
