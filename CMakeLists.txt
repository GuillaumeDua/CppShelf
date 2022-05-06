cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "[${CMAKE_PROJECT_NAME}] : In-source build not recommended")
endif()

## Components
set(CSL_COMPONENT_LIST
    mp
    wf
)

### Components - tests
option(CSL_BUILD_ALL_TESTS "Build all tests for csl" ON) # force all tests
message(STATUS "[${CMAKE_PROJECT_NAME}] : CSL_BUILD_ALL_TESTS set to ${CSL_BUILD_ALL_TESTS} ...")
set(CSL_TESTS_LIST "")

### Components - add/glue (csl_lib)
function(CSL_ADD_COMPONENT component_name)
    message(STATUS "[${CMAKE_PROJECT_NAME}] : csl::${component_name} ...")
    add_library(csl_${component_name}_lib INTERFACE)
    target_include_directories(csl_${component_name}_lib INTERFACE
        ${PROJECT_SOURCE_DIR}/includes/${component_name}
    )
    add_library(csl::${component_name} ALIAS csl_${component_name}_lib)
endfunction()

add_library(csl_lib INTERFACE)
foreach(component_name IN LISTS CSL_COMPONENT_LIST)
    CSL_ADD_COMPONENT(${component_name})
    
    # library glue
    add_dependencies(csl_lib csl::${component_name})
    target_link_libraries(csl_lib INTERFACE csl::${component_name})
    
    # tests
    if (${CSL_BUILD_ALL_TESTS})
        set(CSL_BUILD_TEST_${component_name} ON)
    else()
        option(CSL_BUILD_TEST_${component_name} "[${CMAKE_PROJECT_NAME}] : csl::${component_name} (tests)" OFF)
    endif()
    if (CSL_BUILD_TEST_${component_name})
        message(STATUS "[${CMAKE_PROJECT_NAME}] : csl::${component_name} (tests) ...")
        LIST(APPEND CSL_TESTS_LIST ${component_name})
    endif()
endforeach()

## Tests
list(LENGTH CSL_TESTS_LIST CSL_TESTS_LIST_length)
if (${CSL_TESTS_LIST_length} EQUAL 0)
    return()
else()
    enable_testing()

    add_compile_options(-Wall -Wno-unknown-pragmas)

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-fcolor-diagnostics)
    endif()

    # Catch2
endif()

foreach(component_name IN LISTS CSL_TESTS_LIST)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests/${component_name})
endforeach()
