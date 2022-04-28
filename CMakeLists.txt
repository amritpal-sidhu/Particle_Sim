cmake_minimum_required (VERSION 3.22)
cmake_policy(SET CMP0091 NEW)
project(Physics_Sim LANGUAGES C)


set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(cmake/config.cmake)
include_guard(GLOBAL)


add_subdirectory(Third-Party/glfw)
target_compile_definitions(glfw PRIVATE GLFW_USE_CONFIG_H)
target_include_directories(glfw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party)

add_subdirectory(Third-Party/glad)

add_subdirectory(Third-Party/Unity)
target_compile_definitions(unity PRIVATE UNITY_INCLUDE_CONFIG_H)
target_include_directories(unity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party)

add_subdirectory(particle_sim)


