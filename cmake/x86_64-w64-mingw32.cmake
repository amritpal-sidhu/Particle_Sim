# Define the environment for cross-compiling with 64-bit MinGW-w64 GCC
set(CMAKE_SYSTEM_NAME    Windows) # Target system name
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_LIBRARY_ARCHITECTURE "x86_64-w64-mingw32" CACHE STRING "" FORCE)

set(CMAKE_C_COMPILER        "$CACHE{CMAKE_LIBRARY_ARCHITECTURE}-gcc")
set(CMAKE_CXX_COMPILER      "$CACHE{CMAKE_LIBRARY_ARCHITECTURE}-g++")
set(CMAKE_RC_COMPILER       "$CACHE{CMAKE_LIBRARY_ARCHITECTURE}-windres")
set(CMAKE_RANLIB_COMPILER   "$CACHE{CMAKE_LIBRARY_ARCHITECTURE}-ranlib")

# Configure the behaviour of the find commands
set(CMAKE_FIND_ROOT_PATH    "/usr/$CACHE{CMAKE_LIBRARY_ARCHITECTURE}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

