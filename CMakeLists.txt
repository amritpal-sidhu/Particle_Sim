cmake_minimum_required (VERSION 3.22)
project(Physics_Sim)

include_directories(Third-Party)
add_definitions(-DUNITY_INCLUDE_CONFIG_H)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=4.6" CACHE STRING "" FORCE)

add_subdirectory(Third-Party/glfw)
add_subdirectory(Third-Party/glad)
add_subdirectory(Third-Party/Unity)

add_subdirectory(particle_sim)
