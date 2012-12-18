#define if we have pthreads with recusrive capabilities

MACRO ( CHECK_PTHREAD_RECURSIVE ifpthread result) 

IF ( ${ifpthread} )
	SET ( CMAKE_REQUIRED_FLAGS "${CMAKE_THREAD_LIBS_INIT}")

    CHECK_CXX_SOURCE_RUNS("
        #include <sys/types.h>
        #include <pthread.h>
        #include <stdlib.h>
        
        int main() {
            pthread_mutexattr_t attr;
            pthread_mutex_t m;
        
            exit (pthread_mutexattr_init(&attr) 
                  || pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)
                  || pthread_mutex_init(&m, &attr));
        }
    " ${result} )
    #NOTE: pthread_mutexattr_setkind_np is the deprecated name for pthread_mutexattr_settype. old compilers might need it

	
	SET ( CMAKE_REQUIRED_FLAGS)
ENDIF ( ${ifpthread} )
ENDMACRO ( CHECK_PTHREAD_RECURSIVE ) 
