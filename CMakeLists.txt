cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION                 0.3.1) # not relying on latest tag to avoid stale target
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "[${PROJECT_NAME}] : In-source build not recommended")
endif()

include(CMakeDependentOption)
include(CMakeParseArguments)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/csl_add_component.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/print_aligned.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/cache_entry_to_int.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/make_subdirectory_list.cmake)

## Components - detection, list
MAKE_SUBDIRECTORY_LIST(
    DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/libs
    RESULT      CSL_COMPONENTS_LIST
)
message(VERBOSE "[${PROJECT_NAME}] Available components: [${CSL_COMPONENTS_LIST}]")
message(STATUS "[${PROJECT_NAME}] Global configuration")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

### Components - build
option(CSL_BUILD_ALL "CSL: Build all components" ON) # force all components build
print_aligned(STATUS CSL_BUILD_ALL)

list(APPEND CMAKE_MESSAGE_INDENT "  ")
foreach(component_name IN LISTS CSL_COMPONENTS_LIST)

    option(CSL_BUILD_${component_name} "CSL: Build ${component_name}" ${CSL_BUILD_ALL})
    print_aligned(STATUS CSL_BUILD_${component_name}
        depends CSL_BUILD_ALL
    )

    if (${CSL_BUILD_${component_name}})
        list(APPEND CSL_ENABLED_COMPONENTS_LIST ${component_name})
    endif()

endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

#### Warn for empty CSL_ENABLED_COMPONENTS_LIST
list(LENGTH CSL_ENABLED_COMPONENTS_LIST CSL_ENABLED_COMPONENTS_LIST_length)
if (${CSL_ENABLED_COMPONENTS_LIST_length} EQUAL 0)
    message(WARNING "[${PROJECT_NAME}] No components enabled, is it intended ? Aborting.")
    return()
endif()

#### Common  options (opt-outs)
set(CSL_OPTIONS_LIST
    TEST # TECH-DEBT: clang sanitizers, etc.
    EXAMPLE
    INSTALL
    DOC
    BENCHMARK
)
foreach(option_name IN LISTS CSL_OPTIONS_LIST)
    option(CSL_${option_name}_ALL "${PROJECT_NAME}: Enable all ${option_name} for all csl components (can be override per component)" ON)
    print_aligned(STATUS CSL_${option_name}_ALL)

    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        option(CSL_${option_name}_${component_name} "[${PROJECT_NAME}] csl::${component_name} (${option_name})" ${CSL_${option_name}_ALL})
        print_aligned(STATUS CSL_${option_name}_${component_name}
            depends CSL_${option_name}_ALL
        )
    endforeach()
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

message(VERBOSE "[${PROJECT_NAME}] Build [${PROJECT_NAME}] meta-library, aggregating enabled components: [${CSL_ENABLED_COMPONENTS_LIST}]")
message(STATUS "[${PROJECT_NAME}] Adding components ...")
list(APPEND CMAKE_MESSAGE_INDENT "   ")

list(FIND CSL_ENABLED_COMPONENTS_LIST test _csl_test_idx)
if (_csl_test_idx GREATER -1)
    list(REMOVE_AT CSL_ENABLED_COMPONENTS_LIST ${_csl_test_idx})
    list(INSERT CSL_ENABLED_COMPONENTS_LIST 0 test)
endif()

add_library(${PROJECT_NAME} INTERFACE)
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/${component_name})
    add_dependencies(${PROJECT_NAME} csl::${component_name})
    target_link_libraries(${PROJECT_NAME} INTERFACE csl::${component_name})
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)
