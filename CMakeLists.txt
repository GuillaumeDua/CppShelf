cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION 0.3.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "[${PROJECT_NAME}] : In-source build not recommended")
endif()

include(CMakeDependentOption)
include(CMakeParseArguments)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/print_option.cmake)
## Components - detection, list
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/make_subdirectory_list.cmake)
MAKE_SUBDIRECTORY_LIST(
    DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/includes
    RESULT      CSL_COMPONENTS_LIST
)
message(STATUS "[${PROJECT_NAME}] available components: [${CSL_COMPONENTS_LIST}]")

### Components - build
option(CSL_BUILD_ALL "CSL: Build all components" ON) # force all components build
message(VERBOSE "[${PROJECT_NAME}] CSL_BUILD_ALL set to [${CSL_BUILD_ALL}]")

list(APPEND CMAKE_MESSAGE_INDENT "  ")
foreach(component_name IN LISTS CSL_COMPONENTS_LIST)

    option(CSL_BUILD_${component_name} "CSL: Build ${component_name}" ${CSL_BUILD_ALL})
    print_aligned(STATUS CSL_BUILD_${component_name})

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
    message(STATUS "[${PROJECT_NAME}] CSL_${option_name}_ALL set to [${CSL_${option_name}_ALL}]")

    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        option(CSL_${option_name}_${component_name} "[${PROJECT_NAME}] csl::${component_name} (${option_name})" ${CSL_${option_name}_ALL})
        print_aligned(VERBOSE CSL_${option_name}_${component_name})
    endforeach()
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endforeach()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/csl_add.cmake)

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
