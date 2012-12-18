INCLUDE(CheckCXXSourceRuns)

MACRO ( CHECK_HAVE_GCC_ATOMIC_FUNCTIONS result )

# Do step by step checking,
CHECK_CXX_SOURCE_RUNS("
#include <cstdlib>
int main()
{
   unsigned value = 0;
   void* ptr = &value;
   __sync_add_and_fetch(&value, 1);
   __sync_synchronize();
   __sync_sub_and_fetch(&value, 1);
   if (!__sync_bool_compare_and_swap(&value, 0, 1))
      return EXIT_FAILURE;
   
   if (!__sync_bool_compare_and_swap(&ptr, ptr, ptr))
      return EXIT_FAILURE;

   return EXIT_SUCCESS;
}
" ${result} )

ENDMACRO ( CHECK_HAVE_GCC_ATOMIC_FUNCTIONS result )
