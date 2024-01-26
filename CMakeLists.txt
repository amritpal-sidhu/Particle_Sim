cmake_minimum_required (VERSION 3.28)
project(Physics_Sim LANGUAGES C)


if (WIN32)
    find_package(dlfcn-win32 REQUIRED)
    if (dlfcn-win32_FOUND)
        set(CMAKE_DL_LIBS dlfcn-win32::dl)
    endif()
endif()


include(cmake/test_macro_and_funcs.cmake RESULT_VARIABLE CONFIG_FILE)
if (NOT CONFIG_FILE)
    message(FATAL_ERROR "test_macro_and_funcs.cmake configuration file not found")
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "" FORCE)


# settings for GLFW
set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/glfw")
set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# settings for GLAD
set(GLAD_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/glad")
set(GLAD_LIB glad_lib)
set(GLAD_OUT_DIR "${CMAKE_BINARY_DIR}/Third-Party/glad/${GLAD_LIB}")
set(GLAD_CONFIG_OPTS DEBUG LOADER REPRODUCIBLE SHARED
                     LOCATION "${GLAD_OUT_DIR}"
                     API gl:compatibility=4.6)

# settings for C-Utilities
set(SKIP_C_UTILITIES_TESTS ON CACHE BOOL "" FORCE)
set(SKIP_BINARY_TREES_LIB ON CACHE BOOL "" FORCE)
set(SKIP_LINKED_LISTS_LIB ON CACHE BOOL "" FORCE)


add_compile_definitions(POSIX UNICODE _UNICODE $<$<CONFIG:Debug>:DEBUG>)


add_subdirectory(Third-Party/Unity)
target_compile_definitions(unity PUBLIC UNITY_INCLUDE_CONFIG_H)


add_subdirectory("${GLAD_SOURCES_DIR}/cmake")
glad_add_library(${GLAD_LIB} ${GLAD_CONFIG_OPTS})

add_subdirectory(Third-Party/glfw)

add_subdirectory(C-Utilities)
add_subdirectory(mechanics)
add_subdirectory(graphic_helpers)
add_subdirectory(particle_sim)
