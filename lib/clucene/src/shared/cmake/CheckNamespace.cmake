#check if we support namespaces
MACRO ( CHECK_NAMESPACE haveNamespace )
    #Check if namespaces work in the compiler
    CHECK_CXX_SOURCE_RUNS("
    	namespace Outer { namespace Inner { int i = 0; }}
    	int main(){ using namespace Outer::Inner; return i; }" 
    	${haveNamespace} )
ENDMACRO ( CHECK_NAMESPACE haveNamespace )