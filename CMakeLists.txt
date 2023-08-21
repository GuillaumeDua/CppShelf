cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#TODO(Guss): dev vs. consumer profiles
# - consumer can load 1-N csl lib

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "[${CMAKE_PROJECT_NAME}] : In-source build not recommended")
endif()

## Components
set(CSL_COMPONENT_LIST
    mp
    wf
    ag
    ensure
    functional
    typeinfo
)

# TODO: fetch_content example in README, for 1, N components

### Components - build
option(CSL_BUILD_ALL "Build all csl components" ON) # force all components build
unset(CSL_ENABLED_COMPONENTS_LIST)
message(VERBOSE "[${CMAKE_PROJECT_NAME}] CSL_BUILD_ALL set to ${CSL_BUILD_ALL}")
message(STATUS "[${CMAKE_PROJECT_NAME}] Loading components:")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
foreach(component_name IN LISTS CSL_COMPONENT_LIST)
    option(CSL_BUILD_${component_name} "Build all csl components" ${CSL_BUILD_ALL})
    message(VERBOSE "[CSL_BUILD_${component_name}] CSL_BUILD_ALL set to ${CSL_BUILD_${component_name}}")
    message(CHECK_START "csl::${component_name}")
    if (${CSL_BUILD_${component_name}})
        message(CHECK_PASS "enabled")
        list(APPEND CSL_ENABLED_COMPONENTS_LIST ${component_name})
    else()
        message(CHECK_FAIL "disabled")
    endif()
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

### Components - tests
option(CSL_BUILD_ALL_TESTS "Build all tests for csl" OFF) # force all tests
message(VERBOSE "[${CMAKE_PROJECT_NAME}] CSL_BUILD_ALL_TESTS set to ${CSL_BUILD_ALL_TESTS} ...")
set(CSL_ENABLED_TESTS_LIST "")

### Components libraries - csl_lib::<name>
function(csl_create_component_library component_name)
    message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} ...")

    # check if component has a custom cmake for custom pre-build/configuration
    set(maybe_component_custom_cmake_path "${PROJECT_SOURCE_DIR}/cmake/${component_name}.cmake")
    if (EXISTS "${maybe_component_custom_cmake_path}")
        message(STATUS "[${CMAKE_PROJECT_NAME}] : csl::${component_name} : custom cmake detected ...")
        include("${maybe_component_custom_cmake_path}")
    endif()

    # add the component
    add_library(csl_${component_name}_lib INTERFACE)
    target_include_directories(csl_${component_name}_lib INTERFACE
        ${PROJECT_SOURCE_DIR}/includes/${component_name}
    )
    add_library(csl::${component_name} ALIAS csl_${component_name}_lib)
endfunction()

add_library(csl_lib INTERFACE)
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    csl_create_component_library(${component_name})
    
    add_dependencies(csl_lib csl::${component_name})
    target_link_libraries(csl_lib INTERFACE csl::${component_name})
endforeach()

## Tests
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    option(CSL_BUILD_TEST_${component_name} "[${CMAKE_PROJECT_NAME}] csl::${component_name} (tests)" ${CSL_BUILD_ALL_TESTS})
    if (CSL_BUILD_TEST_${component_name})
        message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} (tests) ...")
        LIST(APPEND CSL_ENABLED_TESTS_LIST ${component_name})
    endif()
endforeach()

list(LENGTH CSL_ENABLED_TESTS_LIST CSL_TESTS_LIST_length)
if (NOT ${CSL_TESTS_LIST_length} EQUAL 0)
    enable_testing()

    add_compile_options(-Wall -Wno-unknown-pragmas)
    #TODO(Guss): sanitizers

    # if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    #     add_compile_options(-fdiagnostics-color=always)
    # elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    #     add_compile_options(-fcolor-diagnostics)
    # endif()

    # Catch2
endif()

foreach(component_name IN LISTS CSL_ENABLED_TESTS_LIST)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests/${component_name})
endforeach()

## Examples
option(CSL_BUILD_ALL_EXAMPLES "Build all examples for csl" OFF) # force all examples
list(APPEND CMAKE_MESSAGE_INDENT "  ")
foreach(component_name IN LISTS CSL_COMPONENT_LIST)
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/examples/${component_name}/CMakeLists.txt)
        message(AUTHOR_WARNING "[${CMAKE_PROJECT_NAME}] : csl::${component_name} has no examples available, skipping ...")
        continue()
    endif()
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples/${component_name})
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)