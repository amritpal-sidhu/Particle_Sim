project(graphic_helpers)

get_sources_abspath(SRCS graphic_helpers.c)

add_library(${PROJECT_NAME} STATIC ${LOCAL_SOURCES})
target_link_libraries(${PROJECT_NAME}
    PUBLIC vector log mechanics glfw ${GLAD_LIB}
    PRIVATE ${CMAKE_DL_LIBS}
)
target_include_directories(${PROJECT_NAME}
    PUBLIC inc
    "${CMAKE_SOURCE_DIR}/inc"
    "${CMAKE_SOURCE_DIR}/particle_sim/inc"
    "${CMAKE_BINARY_DIR}/include"
    "$<TARGET_PROPERTY:glfw,INCLUDE_DIRECTORIES>"
)

file(COPY shaders DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug")
