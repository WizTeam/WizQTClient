# Cross compiling from linux using mingw32 tools
# On ubuntu, you'll need to install the packages: mingw32, mingw32-binutils, mingw32-runtime
#
# Use of this file:
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32.cmake -C ../cmake/Define-mingw32.cmake ..

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc /home/alex/mingw-install )

INCLUDE_DIRECTORIES(/usr/lib/gcc/i586-mingw32msvc/4.2.1-sjlj/include/c++)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(_CL_HAVE_GCCVISIBILITYPATCH 0)
SET(_CL_HAVE_NAMESPACES_EXITCODE 0)
SET(_CL_HAVE_NO_SNPRINTF_BUG_EXITCODE 0)
SET(_CL_HAVE_NO_SNWPRINTF_BUG_EXITCODE 0)
SET(LUCENE_STATIC_CONSTANT_SYNTAX_EXITCODE 1)
SET(_CL_HAVE_TRY_BLOCKS_EXITCODE 0)

