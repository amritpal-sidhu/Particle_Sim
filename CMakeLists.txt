cmake_minimum_required (VERSION 3.22)
project(Physics_Sim)

include_directories(Third-Party Third-Party/glfw/include)
add_definitions(-DUNITY_INCLUDE_CONFIG_H)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(Third-Party/glfw)
add_subdirectory(Third-Party/Unity)

add_subdirectory(particle_sim)
