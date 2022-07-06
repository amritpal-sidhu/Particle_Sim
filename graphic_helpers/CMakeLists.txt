project(graphic_helpers)

set(LOCAL_SOURCES graphic_helpers.c)
list(TRANSFORM LOCAL_SOURCES PREPEND ${CMAKE_CURRENT_LIST_DIR}/src/)

add_library(${PROJECT_NAME} STATIC ${LOCAL_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC inc)
target_link_libraries(${PROJECT_NAME} vector log mechanics glfw glad)

file(COPY shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
