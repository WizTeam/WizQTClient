# Cross compiling from linux using g++-multilib to create 32 bit output
# On ubuntu, you'll need to install the packages: g++-multilib gcc-multilib
#
# Use of this file:
# cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-g++32.cmake ..

SET(CMAKE_CXX_FLAGS "-m32")
SET(CMAKE_C_FLAGS "-m32")
SET(CMAKE_EXE_LINKER_FLAGS "-m32")
SET(CMAKE_MODULE_LINKER_FLAGS "-m32")

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/lib32 )

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

