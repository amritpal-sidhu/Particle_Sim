project(particle_sim)

get_sources_abspath(SRCS particle_sim.c callbacks.c init_functions.c)

add_executable(${PROJECT_NAME} WIN32)
target_sources(${PROJECT_NAME} PRIVATE ${LOCAL_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PUBLIC vector log mechanics graphic_helpers glfw ${GLAD_LIB}
        PRIVATE ${CMAKE_DL_LIBS}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC inc
    "${CMAKE_SOURCE_DIR}/inc"
    "${CMAKE_BINARY_DIR}/include"
    "$<TARGET_PROPERTY:glfw,INCLUDE_DIRECTORIES>"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        TARGET_GL_MAJOR=$CACHE{GLAD_API_MAJOR}
        TARGET_GL_MINOR=$CACHE{GLAD_API_MINOR}
        TARGET_GL_PROFILE=$CACHE{GLAD_API_PROFILE_DEF}
)

unset(GLAD_API_MAJOR CACHE)
unset(GLAD_API_MINOR CACHE)
unset(GLAD_API_PROFILE_DEF CACHE)
