cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION 0.1.0)

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "[${CMAKE_PROJECT_NAME}] : In-source build not recommended")
endif()

# TODO : Component list + foreach

## Components
## - csl::mp
add_library(csl_mp_lib INTERFACE)
target_include_directories(csl_mp_lib INTERFACE
    ${PROJECT_SOURCE_DIR}/includes #/csl/mp.hpp
)
add_library(csl::mp ALIAS csl_mp_lib)
## - csl::wf
add_library(csl_wf_lib INTERFACE)
target_include_directories(csl_wf_lib INTERFACE
    ${PROJECT_SOURCE_DIR}/includes #/csl/workflow.hpp
)
add_library(csl::wf ALIAS csl_wf_lib)

add_library(csl_lib INTERFACE)
add_dependencies(csl_lib
    csl::mp
    csl::wf
)
target_link_libraries(csl_lib INTERFACE
    csl::mp
    csl::wf
)
add_library(csl::csl ALIAS csl_lib)

# Tests
option(CSL_BUILD_ALL_TESTS "Build all tests for csl" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] : CSL_BUILD_ALL_TESTS set to ${CSL_BUILD_ALL_TESTS} ...")

if (CSL_BUILD_ALL_TESTS)
    set(CSL_BUILD_TEST_WF ON)
    set(CSL_BUILD_TEST_MP ON)
else()
    option(CSL_BUILD_TEST_WF "Build all tests for csl::wf" OFF)
    option(CSL_BUILD_TEST_MP "Build all tests for csl::mp" OFF)
endif()

set(CSL_TESTS_LIST "")
if (CSL_BUILD_TEST_WF)
    LIST(APPEND CSL_TESTS_LIST wf)
endif()
if (CSL_BUILD_TEST_MP)
    LIST(APPEND CSL_TESTS_LIST mp)
endif()

list(LENGTH CSL_TESTS_LIST CSL_TESTS_LIST_length)
if (${CSL_TESTS_LIST_length} EQUAL 0)
    return()
else()
    enable_testing()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()

    # Catch2
endif()

function(CSL_ADD_COMPONENT_TEST component_name)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests/${component_name})
endfunction()

foreach(component_name IN LISTS CSL_TESTS_LIST)
    CSL_ADD_COMPONENT_TEST(${component_name})
endforeach()

#add_subdirectory(${PROJECT_SOURCE_DIR}/tests/workflow)
