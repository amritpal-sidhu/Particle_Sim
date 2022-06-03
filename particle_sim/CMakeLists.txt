set(MAIN particle_sim)
set(UNITY_DIR ${CMAKE_SOURCE_DIR}/Third-Party/Unity)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/Third-Party/glfw)
set(GENERATE_TEST_RUNNER ${UNITY_DIR}/auto/generate_test_runner.rb)

set(LOCAL_SOURCES particle_sim.c
                  particle.c
                  mechanics.c
                  graphic_helpers.c)
list(TRANSFORM LOCAL_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/src/)

add_compile_options(
    -Wsign-conversion
    -Wcast-qual
    -Wstrict-prototypes
)

include_directories(inc
                    ${UNITY_DIR}/src
                    ${GLFW_DIR}/include
                    ${GLFW_DIR}/src
                    ${GLAD_INCLUDE_DIRS})


add_executable(${MAIN} WIN32)
target_sources(${MAIN} PRIVATE ${LOCAL_SOURCES} ${GLFW_DIR}/deps/linmath.h)
target_link_libraries(${MAIN} PRIVATE vector log glfw glad)

file(COPY shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
