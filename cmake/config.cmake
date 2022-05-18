include_guard(GLOBAL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE PATH "" FORCE)

add_compile_definitions(POSIX UNICODE _UNICODE)

# Options for GLFW
set(GLFW_BUILD_SHARED_LIBRARY ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Options for GLAD
set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=4.6" CACHE STRING "" FORCE)
set(GLAD_REPRODUCIBLE ON CACHE BOOL "" FORCE)

# Check for ruby program needed by Unity script dependency
find_program(RUBY_PROGRAM ruby /usr/bin REQUIRED)

set(UNITY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/Unity CACHE PATH "")
set(GENERATE_TEST_RUNNER_SCRIPT ${UNITY_DIR}/auto/generate_test_runner.rb CACHE FILEPATH "")
set(SRC_EXT cpp CACHE INTERNAL "Source file extension")
set(TEST_SRC_FORMAT test_<FILENAME>.${SRC_EXT} CACHE FILEPATH "Template variable format for test source file")
set(TEST_RUNNER_FORMAT test_<FILENAME>_runner.${SRC_EXT} CACHE FILEPATH "Template variable format for test runner file")

include_directories(Third-Party)

macro(run_tests_macro)

    set(TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/unit_tests)
    set(TEST_RUNNER_DIR ${PROJECT_BINARY_DIR}/test_runners)
    if(NOT EXISTS ${TEST_RUNNER_DIR})
        file(MAKE_DIRECTORY ${TEST_RUNNER_DIR})
    endif()

    foreach(SRC_FILE ${LOCAL_SOURCES})

        get_filename_component(FILENAME ${SRC_FILE} NAME_WE)
        string(REPLACE <FILENAME> ${FILENAME} TEST_SRC ${TEST_DIR}/$CACHE{TEST_SRC_FORMAT})
        string(REPLACE <FILENAME> ${FILENAME} TEST_RUNNER ${TEST_RUNNER_DIR}/$CACHE{TEST_RUNNER_FORMAT})

        if (EXISTS ${TEST_SRC})

            add_custom_command(OUTPUT ${TEST_RUNNER}
                               COMMAND ruby ${GENERATE_TEST_RUNNER_SCRIPT}
                                            ${TEST_SRC}
                                            ${TEST_RUNNER}
                               DEPENDS ${TEST_SRC} VERBATIM)

            add_executable(test_runner ${TEST_SRC} ${TEST_RUNNER})
            target_link_libraries(test_runner PRIVATE unity ${PROJECT_NAME})
            target_include_directories(test_runner PRIVATE ${UNITY_DIR}/src)
            
            add_custom_command(TARGET test_runner POST_BUILD
                               WORKING_DIRECTORY  ${TEST_DIR}
                               COMMAND $<TARGET_FILE:test_runner>
                               VERBATIM USES_TERMINAL)
        else()
            message(STATUS "No unit test for ${FILENAME} exists.")
        endif()

    endforeach()

endmacro()
