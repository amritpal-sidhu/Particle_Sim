set(MAIN particle_sim)
set(UNITY_DIR ${CMAKE_SOURCE_DIR}/Third-Party/Unity)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/Third-Party/glfw)
set(GENERATE_TEST_RUNNER ${UNITY_DIR}/auto/generate_test_runner.rb)

file(GLOB LOCAL_SOURCES src/*.c)

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
target_compile_definitions(${MAIN} PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS)
target_compile_options(${MAIN} PRIVATE "-municode")
target_link_libraries(${MAIN} PRIVATE glfw glad m)
target_sources(${MAIN} PRIVATE ${LOCAL_SOURCES} ${GLFW_DIR}/deps/linmath.h)
set_property(TARGET ${MAIN} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

file(COPY shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
# set_target_properties(${MAIN} PROPERTIES GNUtoMS ON)
