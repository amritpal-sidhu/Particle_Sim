include_guard(GLOBAL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "" FORCE)
set(CMAKE_COLOR_MAKEFILE ON CACHE BOOL "" FORCE)
set(CMAKE_COLOR_DIAGNOSTICS ON CACHE BOOL "" FORCE)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON CACHE BOOL "" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" CACHE PATH "" FORCE)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH "" FORCE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/archive" CACHE PATH "" FORCE)



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
                               COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE:test_runner>
                               VERBATIM USES_TERMINAL)
        else()
            message(STATUS "No unit test for ${FILENAME} exists.")
        endif()

    endforeach()

endmacro()


# simple wrapper to prepend absolute path to a list of source files
function(get_sources_abspath)
    cmake_parse_arguments(ARGS "" "" "SRCS" ${ARGN})
    if (ARGS_SRCS)
        list(TRANSFORM ARGS_SRCS PREPEND "${CMAKE_CURRENT_LIST_DIR}/src/" OUTPUT_VARIABLE LOCAL_SOURCES)
    endif()
    return(PROPAGATE LOCAL_SOURCES)
endfunction()


# adds header as an interface library and links it into the build tree
function(import_header_interface)
    cmake_parse_arguments(in "" "TARGET;SRCDIR" "" ${ARGN})

    set(DEST "${CMAKE_BINARY_DIR}/include")

    file(MAKE_DIRECTORY "${DEST}")
    file(COPY "${in_SRCDIR}" DESTINATION "${DEST}")

    # add_library(${in_TARGET} INTERFACE IMPORTED)
    # target_sources(${in_TARGET} INTERFACE
    #     FILE_SET HEADERS BASE_DIRS "${DEST}" FILES "${DEST}/${in_TARGET}.h"
    # )
    
    return()
endfunction()


# get API components from GLAD library API
function(get_glad_api_components)
    cmake_parse_arguments(in "" "API" "" ${ARGN})

    list(TRANSFORM in_API REPLACE "[:=\.]" ";" OUTPUT_VARIABLE API_SPLIT)
    list(LENGTH API_SPLIT LEN)
    if (NOT LEN EQUAL 4)
        message(FATAL_ERROR "get_glad_api_compnents() wrong API lenght")
    endif()

    list(GET API_SPLIT 0 SPEC)
    list(GET API_SPLIT 1 PROFILE)
    list(GET API_SPLIT 2 MAJOR)
    list(GET API_SPLIT 3 MINOR)

    if (NOT SPEC MATCHES "(egl|vulkan|gl|gles1|gles2|glsc2|wgl|glx)?")
        message(FATAL_ERROR "GLAD spec \"${SPEC}\" is invalid")
    elseif (MAJOR LESS 1 OR MAJOR GREATER 4)
        message(FATAL_ERROR "GLAD major \"${MAJOR}\" is out of range [1:4]")
    elseif (MINOR LESS 0 OR MINOR GREATER 9)
        message(FATAL_ERROR "GLAD minor \"${MINOR}\" is out of range [0:9]")
    elseif (NOT PROFILE MATCHES "(core|compatibility)?")
        message(FATAL_ERROR "GLAD profile \"${PROFILE}\" is not core or compatibility")
    endif()

    return(PROPAGATE SPEC PROFILE MAJOR MINOR)
endfunction()


# wrapper around glad_add_library()
function(create_glad_library)
    cmake_parse_arguments(in "" "API" "" ${ARGN})

    
    foreach (API IN LISTS in_API)

        get_glad_api_components(API ${API})
    
        set(GLAD_LIB glad_${SPEC}_${PROFILE}_${MAJOR}${MINOR})
        set(GLAD_OUT_DIR "${CMAKE_BINARY_DIR}/Third-Party/glad/${GLAD_LIB}")

        set(GLAD_LIB_OPTS HEADERONLY INTERFACE LOCATION "${GLAD_OUT_DIR}" API ${API})
        if (CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT "MX" IN_LIST GLAD_LIB_OPTS)
            list(PREPEND GLAD_LIB_OPTS DEBUG)
        endif()

        glad_add_library(${GLAD_LIB} "${GLAD_LIB_OPTS}")

        if (PROFILE STREQUAL "core")
            set(GLAD_API_PROFILE_DEF GLFW_OPENGL_CORE_PROFILE CACHE INTERNAL "")
        else()
            set(GLAD_API_PROFILE_DEF GLFW_OPENGL_COMPAT_PROFILE CACHE INTERNAL "")
        endif()
        set(GLAD_API_MAJOR ${MAJOR} CACHE INTERNAL "")
        set(GLAD_API_MINOR ${MINOR} CACHE INTERNAL "")
        
    endforeach()

    return(PROPAGATE GLAD_LIB)
endfunction()
