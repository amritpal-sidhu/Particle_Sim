cmake_minimum_required (VERSION 3.28)
cmake_policy(VERSION 3.28)
project(Physics_Sim LANGUAGES C VERSION 1.0)


if (WIN32)
    find_package(dlfcn-win32 REQUIRED)
    if (dlfcn-win32_FOUND)
        set(CMAKE_DL_LIBS dlfcn-win32::dl)
    endif()
endif()


# settings for GLFW
set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_USE_HYBRID_HPG ON CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# settings for GLAD
set(GLAD_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/glad")
set(GLAD_TARGET_API gl:core=4.6)

# settings for C-Utilities
set(SKIP_C_UTILITIES_TESTS ON CACHE BOOL "" FORCE)
set(SKIP_BINARY_TREES_LIB ON CACHE BOOL "" FORCE)
set(SKIP_LINKED_LISTS_LIB ON CACHE BOOL "" FORCE)


add_compile_definitions(POSIX UNICODE _UNICODE $<$<CONFIG:Debug>:DEBUG>)


add_subdirectory(Third-Party/Unity)
target_compile_definitions(unity PUBLIC UNITY_INCLUDE_CONFIG_H)


add_subdirectory("${GLAD_SOURCES_DIR}/cmake")
create_glad_library(API "${GLAD_TARGET_API}")

add_subdirectory(Third-Party/glfw)
import_header_interface(TARGET linmath
        SRCDIR "${CMAKE_SOURCE_DIR}/Third-Party/glfw/deps/linmath.h")

add_subdirectory(C-Utilities)
add_subdirectory(mechanics)
add_subdirectory(graphic_helpers)
add_subdirectory(particle_sim)
