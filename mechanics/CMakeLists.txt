project(mechanics)

prepend_local_sources(mechanics.c particle.c)

add_library(${PROJECT_NAME} STATIC ${LOCAL_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC inc)
target_link_libraries(${PROJECT_NAME} PUBLIC m vector log)

run_tests_macro()
