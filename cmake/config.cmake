
# set(CMAKE_C_STANDARD "99" CACHE STRING "" FORCE)
# set(CMAKE_C_FLAGS "-Wall -Werror -Wstrict-prototypes -Wshadow" CACHE STRING "" FORCE)
# set(CMAKE_C_FLAGS_DEBUG "-gdwarf-2" CACHE STRING "" FORCE)
# set(CMAKE_C_LINK_EXECUTABLE "-static" CACHE STRING "" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "" FORCE)

# set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
# set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll" CACHE STRING "" FORCE)
# set(CMAKE_ENABLE_EXPORTS ON CACHE BOOL "" FORCE)
# set(CMAKE_LINK_DEF_FILE_FLAG ON CACHE BOOL "" FORCE)

# set(CMAKE_C_SIMULATE_ID "MSVC" CACHE STRING "" FORCE)
# set(CMAKE_C_COMPILER_FRONTEND_VARIANT "GNU" CACHE STRING "" FORCE)

set(GLFW_BUILD_SHARED_LIBRARY ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=4.6" CACHE STRING "" FORCE)
set(GLAD_REPRODUCIBLE ON CACHE BOOL "" FORCE)
