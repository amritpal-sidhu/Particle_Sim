project(particle_sim)

prepend_local_sources(particle_sim.c callbacks.c init_functions.c)

add_executable(${PROJECT_NAME} WIN32)
target_sources(${PROJECT_NAME} PRIVATE ${LOCAL_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PUBLIC vector log mechanics graphic_helpers glfw ${GLAD_LIB}
        PRIVATE ${CMAKE_DL_LIBS})
target_include_directories(${PROJECT_NAME}
        PRIVATE inc
        "${CMAKE_SOURCE_DIR}/inc"
        "$<TARGET_PROPERTY:glfw,INCLUDE_DIRECTORIES>"
        "$<TARGET_PROPERTY:unity,INCLUDE_DIRECTORIES>"
        "$<TARGET_PROPERTY:${GLAD_LIB},INCLUDE_DIRECTORIES>")
target_sources(${PROJECT_NAME} PRIVATE FILE_SET interface_HEADER_SETS TYPE HEADERS BASE_DIRS "${GLFW_DIR}/deps/" FILES "${GLFW_DIR}/deps/linmath.h")
target_compile_definitions(${PROJECT_NAME} PRIVATE
        TARGET_GL_MAJOR=${TARGET_GL_MAJOR}
        TARGET_GL_MINOR=${TARGET_GL_MINOR}
        TARGET_GL_PROFILE=$<IF:$<STREQUAL:${TARGET_GL_PROFILE},core>,GLFW_OPENGL_CORE_PROFILE,GLFW_OPENGL_COMPAT_PROFILE>)
