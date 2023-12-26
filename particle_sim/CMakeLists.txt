set(MAIN particle_sim)
set(UNITY_DIR "${CMAKE_SOURCE_DIR}/Third-Party/Unity")
set(GLFW_DIR "${CMAKE_SOURCE_DIR}/Third-Party/glfw")

set(LOCAL_SOURCES particle_sim.c)
list(TRANSFORM LOCAL_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/src/")

add_compile_options(
    -Wsign-conversion
    -Wcast-qual
    -Wstrict-prototypes
)


add_executable(${MAIN} WIN32)
target_sources(${MAIN} PRIVATE ${LOCAL_SOURCES} "${GLFW_DIR}/deps/linmath.h")
target_link_libraries(${MAIN} PRIVATE vector log mechanics graphic_helpers)
target_include_directories(${MAIN} PUBLIC
                                    inc
                                    "${UNITY_DIR}/src"
                                    "${GLFW_DIR}/include"
                                    "${GLFW_DIR}/src"
                                    "${GLAD_INCLUDE_DIRS}")
