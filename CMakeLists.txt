cmake_minimum_required (VERSION 3.22)
project(Physics_Sim LANGUAGES C)

include(cmake/config.cmake RESULT_VARIABLE CONFIG_FILE)
if (NOT CONFIG_FILE)
    message(FATAL_ERROR "configuration file not found")
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "" FORCE)


add_compile_definitions(POSIX UNICODE _UNICODE)

# Options for GLFW
set(GLFW_LIBRARY_TYPE SHARED CACHE STRING "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Options for C-Utilities
set(SKIP_C_UTILITIES_TESTS ON CACHE BOOL "" FORCE)
set(SKIP_BINARY_TREES_LIB ON CACHE BOOL "" FORCE)
set(SKIP_LINKED_LISTS_LIB ON CACHE BOOL "" FORCE)


add_subdirectory(Third-Party/glfw)
target_compile_definitions(glfw PRIVATE GLFW_USE_CONFIG_H)
target_include_directories(glfw PRIVATE Third-Party)

add_subdirectory(Third-Party/glad/cmake)
set(TARGET_GLAD_LIB glad_gl_core_46)
set(GLAD_LIB_DIR "${CMAKE_BINARY_DIR}/Third-Party/glad/${TARGET_GLAD_LIB}")
glad_add_library(${TARGET_GLAD_LIB} REPRODUCIBLE LOCATION "${GLAD_LIB_DIR}" API gl:core=4.6)

add_subdirectory(Third-Party/Unity)
target_compile_definitions(unity PUBLIC UNITY_INCLUDE_CONFIG_H)
target_include_directories(unity PRIVATE Third-Party)

add_subdirectory(C-Utilities)

add_subdirectory(mechanics)

add_subdirectory(graphic_helpers)
target_include_directories(graphic_helpers PRIVATE
                                        Third-Party/glfw/include
                                        Third-Party/glfw/deps)

add_subdirectory(particle_sim)
target_include_directories(particle_sim PRIVATE
                                        Third-Party/glfw/include
                                        Third-Party/glfw/deps)

