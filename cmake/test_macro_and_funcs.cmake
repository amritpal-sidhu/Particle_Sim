include_guard(GLOBAL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# Check for ruby program needed by Unity script dependency
find_program(RUBY_PROGRAM ruby /usr/bin REQUIRED)

set(UNITY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/Unity CACHE PATH "")
set(GENERATE_TEST_RUNNER_SCRIPT ${UNITY_DIR}/auto/generate_test_runner.rb CACHE FILEPATH "")
set(SRC_EXT c CACHE INTERNAL "Source file extension")
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
                               COMMAND rm -f $<TARGET_FILE:test_runner>
                               VERBATIM USES_TERMINAL)
        else()
            message(STATUS "No unit test for ${FILENAME} exists.")
        endif()

    endforeach()

endmacro()


function(prepend_local_sources)
    block(SCOPE_FOR VARIABLES POLICIES)
        cmake_policy(SET CMP0140 NEW)
        set(LOCAL_SOURCES "${ARGN}")
        list(TRANSFORM LOCAL_SOURCES PREPEND "${CMAKE_CURRENT_LIST_DIR}/src/")
        return(PROPAGATE LOCAL_SOURCES)
    endblock()
endfunction()


function(set_glad_vars)
    block(SCOPE_FOR VARIABLES POLICIES)
        cmake_policy(SET CMP0140 NEW)

        set(GLAD_TARGET_API "${ARGN}")
        list(LENGTH GLAD_TARGET_API LEN)
        if (NOT ${LEN} EQUAL 3)
            message(FATAL_ERROR "The list passed to set_glad_vars() is not of size 3")
        endif()

        list(GET GLAD_TARGET_API 0 TARGET_GL_MAJOR)
        list(GET GLAD_TARGET_API 1 TARGET_GL_MINOR)
        list(GET GLAD_TARGET_API 2 TARGET_GL_PROFILE)
        if (${TARGET_GL_MAJOR} LESS 1 OR ${TARGET_GL_MAJOR} GREATER 4)
            message(FATAL_ERROR "GLAD major target is out of range [1:4]")
        elseif (${TARGET_GL_MINOR} LESS 0 OR ${TARGET_GL_MINOR} GREATER 9)
            message(FATAL_ERROR "GLAD minor target is out of range [0:9]")
        elseif (NOT ${TARGET_GL_PROFILE} STREQUAL "core" AND NOT ${TARGET_GL_PROFILE} STREQUAL "compatibility")
            message(FATAL_ERROR "GLAD profile is not core or compatibility")
        endif()
        
        set(GLAD_LIB glad_gl_${TARGET_GL_PROFILE}_${TARGET_GL_MAJOR}${TARGET_GL_MINOR})
        set(GLAD_GL_API gl:${TARGET_GL_PROFILE}=${TARGET_GL_MAJOR}.${TARGET_GL_MINOR})
        set(GLAD_OUT_DIR "${CMAKE_BINARY_DIR}/Third-Party/glad/${GLAD_LIB}")
        set(GLAD_LIB_OPTS ALIAS HEADERONLY INTERFACE LOCATION "${GLAD_OUT_DIR}" API ${GLAD_GL_API})

        return(PROPAGATE GLAD_LIB GLAD_LIB_OPTS TARGET_GL_MAJOR TARGET_GL_MINOR TARGET_GL_PROFILE)
    endblock()
endfunction()
