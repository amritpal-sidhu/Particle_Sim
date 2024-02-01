project(graphic_helpers)

prepend_local_sources(graphic_helpers.c)

add_library(${PROJECT_NAME} STATIC ${LOCAL_SOURCES})
target_link_libraries(${PROJECT_NAME}
    PUBLIC vector log mechanics glfw ${GLAD_LIB}
    PRIVATE ${CMAKE_DL_LIBS})
target_include_directories(${PROJECT_NAME}
    PUBLIC inc
    "${CMAKE_SOURCE_DIR}/inc"
    "$<TARGET_PROPERTY:glfw,INCLUDE_DIRECTORIES>"
    "$<TARGET_PROPERTY:${GLAD_LIB},INCLUDE_DIRECTORIES>")
target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS "${GLFW_DIR}/deps" FILES "${GLFW_DIR}/deps/linmath.h")

file(COPY shaders DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
