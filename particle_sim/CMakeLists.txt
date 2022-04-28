set(MAIN particle_sim)
set(UNITY_DIR ${CMAKE_SOURCE_DIR}/Third-Party/Unity)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/Third-Party/glfw)
set(GENERATE_TEST_RUNNER ${UNITY_DIR}/auto/generate_test_runner.rb)


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
target_compile_definitions(${MAIN} PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(${MAIN} PRIVATE glfw glad)
target_sources(${MAIN} PRIVATE
            src/mechanic_equations.c
            src/particle_sim.c
            src/vector_3d.c
            ${GLFW_DIR}/deps/linmath.h)

file(COPY shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# set_target_properties(${MAIN} PROPERTIES GNUtoMS ON)
