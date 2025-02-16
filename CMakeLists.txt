cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# WIP: REFACTO: cleanunp + simplication => update CI
include(CMakeDependentOption)
include(CMakeParseArguments)

# CSL_ALL
# CSL_INSTALL
# CSL_TESTS
# CSL_EXAMPLES
# CSL_DOCS
# CSL_BENCHMARKS


# TODO(Guss): dev vs. consumer profiles
#   - consumer can load 1-N csl lib
# TODO(Guss): options to enable opt-in based macro defines

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "[${PROJECT_NAME}] : In-source build not recommended")
endif()

## Components - detection, list
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/make_subdirectory_list.cmake)
MAKE_SUBDIRECTORY_LIST(
    DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/includes
    RESULT      CSL_COMPONENTS_LIST
)
message(STATUS "[${PROJECT_NAME}] available components: [${CSL_COMPONENTS_LIST}]")

### Components - build
option(CSL_BUILD_ALL "CSL: Build all components" ON) # force all components build
message(VERBOSE "[${PROJECT_NAME}] CSL_BUILD_ALL set to ${CSL_BUILD_ALL}")

message(CHECK_START "[${PROJECT_NAME}] enabled components")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
foreach(component_name IN LISTS CSL_COMPONENTS_LIST)

    option(CSL_BUILD_${component_name} "CSL: Build ${component_name}" ${CSL_BUILD_ALL})
    message(VERBOSE "[${PROJECT_NAME}]: [CSL_BUILD_${component_name}] set to ${CSL_BUILD_${component_name}}")

    message(CHECK_START "CSL_BUILD_${component_name}")
    if (${CSL_BUILD_${component_name}})
        message(CHECK_PASS "enabled")
        list(APPEND CSL_ENABLED_COMPONENTS_LIST ${component_name})
    else()
        message(CHECK_FAIL "disabled")
    endif()

endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "done")

#### Warn for empty CSL_ENABLED_COMPONENTS_LIST
list(LENGTH CSL_ENABLED_COMPONENTS_LIST CSL_ENABLED_COMPONENTS_LIST_length)
if (${CSL_ENABLED_COMPONENTS_LIST_length} EQUAL 0)
    message(WARNING "[${PROJECT_NAME}] No components enabled, is it intended ? Aborting.")
    return()
endif()

### Components - tests
enable_testing()
option(CSL_TEST_ALL "Build all tests for csl" OFF) # force all tests
message(VERBOSE "[${PROJECT_NAME}] CSL_TEST_ALL set to ${CSL_TEST_ALL} ...")

### Components - examples 
option(CSL_EXAMPLE_ALL "Build all examples for csl" OFF) # force all examples
message(VERBOSE "[${PROJECT_NAME}] CSL_EXAMPLE_ALL set to ${CSL_EXAMPLE_ALL} ...")

### Components - Tests, Examples (Opt-outs)
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    option(CSL_TEST_${component_name}    "[${PROJECT_NAME}] csl::${component_name} (tests)"   ${CSL_TEST_ALL})
    # TECH-DEBT: tests: add_compile_options(-Wall -Wextra -Wno-unknown-pragmas)
    option(CSL_EXAMPLE_${component_name} "[${PROJECT_NAME}] csl::${component_name} (example)" ${CSL_EXAMPLE_ALL})
endforeach()

# WIP: move to another file cmake/add_component
### Components - Targets - csl_lib::<name>
function(csl_add)

    set(options)
    set(oneValueArgs COMPONENT)
    set(multiValueArgs)
    cmake_parse_arguments(csl_add
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED csl_add_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_add] error: UNPARSED_ARGUMENTS = [${csl_add_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED csl_add_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_add] error: KEYWORDS_MISSING_VALUES = [${csl_add_KEYWORDS_MISSING_VALUES}]")
    endif()

    add_library(${PROJECT_NAME}_${csl_add_COMPONENT} INTERFACE)
    add_library(${PROJECT_NAME}::${csl_add_COMPONENT} ALIAS ${PROJECT_NAME}_${csl_add_COMPONENT})

    # check if component has a custom cmake file to inject
    set(maybe_${csl_add_COMPONENT}_custom_cmake_path "${PROJECT_SOURCE_DIR}/cmake/custom/${csl_add_COMPONENT}.cmake")
    if (EXISTS "${maybe_${csl_add_COMPONENT}_custom_cmake_path}")
        message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} : custom cmake rules detected in [${maybe_${csl_add_COMPONENT}_custom_cmake_path}]")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${maybe_${csl_add_COMPONENT}_custom_cmake_path}")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    target_include_directories(csl_${csl_add_COMPONENT} INTERFACE
        ${PROJECT_SOURCE_DIR}/includes/${csl_add_COMPONENT}
    )

    # test
    option(CSL_TEST_${csl_add_COMPONENT} "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (tests)" ${CSL_TEST_ALL})
    if (CSL_TEST_${csl_add_COMPONENT})
        message(STATUS "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (tests)")
        add_subdirectory(${PROJECT_SOURCE_DIR}/tests/${csl_add_COMPONENT})
    endif()

    # example
    option(CSL_EXAMPLE_${csl_add_COMPONENT} "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (example)" ${CSL_EXAMPLE_ALL})
    if (CSL_EXAMPLE_${csl_add_COMPONENT})
        if (NOT EXISTS ${PROJECT_SOURCE_DIR}/examples/${csl_add_COMPONENT}/CMakeLists.txt)
            # message(AUTHOR_WARNING "[${PROJECT_NAME}] : csl::${csl_add_COMPONENT} has no examples available, skipping ...")
        else()
            message(STATUS "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (example)")
            add_subdirectory(${PROJECT_SOURCE_DIR}/examples/${csl_add_COMPONENT})
        endif()
    endif()

endfunction()

message(STATUS "[${PROJECT_NAME}] meta-target ...")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
add_library(${PROJECT_NAME} INTERFACE)
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    csl_add(
        COMPONENT ${component_name}
    )
    
    add_dependencies(${PROJECT_NAME} csl::${component_name})
    target_link_libraries(${PROJECT_NAME} INTERFACE csl::${component_name})
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)
