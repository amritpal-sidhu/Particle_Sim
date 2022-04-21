set(UNITY_DIR ${CMAKE_SOURCE_DIR}/Third-Party/Unity)
set(TEST_RUNNER_DIR ${CMAKE_BINARY_DIR}/utilities/binary_trees/CMakeFiles)

file(GLOB SOURCES src/*.c)
file(GLOB GENERATE_TEST_RUNNER ${UNITY_DIR}/auto/generate_test_runner.rb)


if(NOT EXISTS ${TEST_RUNNER_DIR})
    file(MAKE_DIRECTORY ${TEST_RUNNER_DIR})
endif(NOT EXISTS ${TEST_RUNNER_DIR})


include_directories(inc ${UNITY_DIR}/src)


foreach(SRC_FILE ${SOURCES})

    get_filename_component(FILENAME ${SRC_FILE} NAME_WE)
    
    if (NOT ${FILENAME} STREQUAL "particle_sim")
        add_library(${FILENAME} OBJECT ${SRC_FILE})
        if (${FILENAME} STREQUAL vector_3d)
            target_link_libraries(${FILENAME} m)
        endif()
    
        if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/unit_tests/test_${FILENAME}.c)
            add_library(test_${FILENAME} OBJECT ${CMAKE_CURRENT_LIST_DIR}/unit_tests/test_${FILENAME}.c)
            target_link_libraries(test_${FILENAME} PRIVATE unity ${FILENAME})

            execute_process(COMMAND ruby ${GENERATE_TEST_RUNNER} 
                                     ${CMAKE_CURRENT_LIST_DIR}/unit_tests/test_${FILENAME}.c
                                     ${TEST_RUNNER_DIR}/test_${FILENAME}_runner.c)

            add_executable(test_${FILENAME}_runner ${TEST_RUNNER_DIR}/test_${FILENAME}_runner.c)
            target_link_libraries(test_${FILENAME}_runner PRIVATE test_${FILENAME})
            
            add_custom_command(TARGET test_${FILENAME}_runner POST_BUILD
                               WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/unit_tests
                               COMMAND $<TARGET_FILE:test_${FILENAME}_runner>)
        endif()
    endif()

endforeach(SRC_FILE ${SOURCES})

add_executable(particle_sim WIN32 ${CMAKE_CURRENT_LIST_DIR}/src/particle_sim.c)
target_link_libraries(particle_sim PUBLIC glfw glad mechanic_equations vector_3d)
target_include_directories(particle_sim PUBLIC Third-Party/glfw/include ${GLAD_INCLUDE_DIRS})
set_target_properties(particle_sim PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
