cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# top-level project
project(csl LANGUAGES CXX)
set(PROJECT_VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# TODO(Guss): dev vs. consumer profiles
#   - consumer can load 1-N csl lib
# TODO(Guss): options to enable opt-in based macro defines

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
  message(FATAL_ERROR "[${PROJECT_NAME}] : In-source build not recommended")
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

### Components - build
option(CSL_BUILD_ALL "Build all csl components" ON) # force all components build
unset(CSL_ENABLED_COMPONENTS_LIST)
unset(CSL_DISABLED_COMPONENTS_LIST)
message(VERBOSE "[${PROJECT_NAME}] CSL_BUILD_ALL set to ${CSL_BUILD_ALL}")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
foreach(component_name IN LISTS CSL_COMPONENT_LIST)
    option(CSL_BUILD_${component_name} "Build all csl components" ${CSL_BUILD_ALL})
    message(VERBOSE "[${PROJECT_NAME}]: [CSL_BUILD_${component_name}] CSL_BUILD_ALL set to ${CSL_BUILD_${component_name}}")
    if (${CSL_BUILD_${component_name}})
        list(APPEND CSL_ENABLED_COMPONENTS_LIST ${component_name})
    endif()
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

### Components - enabled vs. disabled
set(CSL_DISABLED_COMPONENTS_LIST ${CSL_COMPONENT_LIST})
list(REMOVE_ITEM CSL_DISABLED_COMPONENTS_LIST ${CSL_ENABLED_COMPONENTS_LIST})

#### enabled
message(STATUS "[${PROJECT_NAME}] enabled components:")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
if (NOT CSL_ENABLED_COMPONENTS_LIST)
    message(STATUS "(none)")
endif()
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    message(STATUS "${PROJECT_NAME}::${component_name}")
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

#### disabled
message(STATUS "[${PROJECT_NAME}] disabled components:")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
if (NOT CSL_DISABLED_COMPONENTS_LIST)
    message(STATUS "(none)")
endif()
foreach(component_name IN LISTS CSL_DISABLED_COMPONENTS_LIST)
    message(STATUS "${PROJECT_NAME}::${component_name}")
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

### Components - tests
enable_testing()
option(CSL_ENABLE_ALL_TESTS "Build all tests for csl" OFF) # force all tests
message(VERBOSE "[${PROJECT_NAME}] CSL_ENABLE_ALL_TESTS set to ${CSL_ENABLE_ALL_TESTS} ...")

### Components - examples 
option(CSL_ENABLE_ALL_EXAMPLES "Build all examples for csl" OFF) # force all examples
message(VERBOSE "[${PROJECT_NAME}] CSL_ENABLE_ALL_EXAMPLES set to ${CSL_ENABLE_ALL_EXAMPLES} ...")

### Opt-ins: Tests, Examples
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    option(CSL_BUILD_TESTS_${component_name}   "[${PROJECT_NAME}] csl::${component_name} (tests)"   ${CSL_ENABLE_ALL_TESTS})
    option(CSL_BUILD_EXAMPLES_${component_name} "[${PROJECT_NAME}] csl::${component_name} (example)" ${CSL_ENABLE_ALL_EXAMPLES})
endforeach()

# list(LENGTH CSL_ENABLED_TESTS_LIST CSL_ENABLED_TESTS_LIST_length)
# if (NOT ${CSL_ENABLED_TESTS_LIST_length} EQUAL 0)
#     enable_testing()
#     add_compile_options(-Wall -Wextra -Wno-unknown-pragmas)
#     #TODO(Guss): sanitizers

#     # if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#     #     add_compile_options(-fdiagnostics-color=always)
#     # elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#     #     add_compile_options(-fcolor-diagnostics)
#     # endif()

#     # Catch2
# endif()

# target_compile_definitions

### Components libraries - csl_lib::<name>
function(csl_create_component component_name)
    message(STATUS "[${PROJECT_NAME}] csl::${component_name}")

    # add the component
    add_library(csl_${component_name}_lib INTERFACE)
    add_library(csl::${component_name} ALIAS csl_${component_name}_lib)
    # check if component has a custom cmake pre-build/configuration to inject
    set(maybe_component_custom_cmake_path "${PROJECT_SOURCE_DIR}/cmake/${component_name}.cmake")
    if (EXISTS "${maybe_component_custom_cmake_path}")
        message(STATUS "[${PROJECT_NAME}] csl::${component_name} : custom cmake rules detected ...")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${maybe_component_custom_cmake_path}")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()
    target_include_directories(csl_${component_name}_lib INTERFACE
        ${PROJECT_SOURCE_DIR}/includes/${component_name}
    )

    # test
    option(CSL_BUILD_TESTS_${component_name} "[${PROJECT_NAME}] csl::${component_name} (tests)" ${CSL_ENABLE_ALL_TESTS})
    if (CSL_BUILD_TESTS_${component_name})
        message(STATUS "[${PROJECT_NAME}] csl::${component_name} (tests)")
        add_subdirectory(${PROJECT_SOURCE_DIR}/tests/${component_name})
    endif()

    # example
    option(CSL_BUILD_EXAMPLES_${component_name} "[${PROJECT_NAME}] csl::${component_name} (example)" ${CSL_ENABLE_ALL_EXAMPLES})
    if (CSL_BUILD_EXAMPLES_${component_name})
        if (NOT EXISTS ${PROJECT_SOURCE_DIR}/examples/${component_name}/CMakeLists.txt)
            # message(AUTHOR_WARNING "[${PROJECT_NAME}] : csl::${component_name} has no examples available, skipping ...")
        else()
            message(STATUS "[${PROJECT_NAME}] csl::${component_name} (example)")
            add_subdirectory(${PROJECT_SOURCE_DIR}/examples/${component_name})
        endif()
    endif()

endfunction()

message(STATUS "")
message(STATUS "[${PROJECT_NAME}] csl_lib ...")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
add_library(csl_lib INTERFACE)
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    csl_create_component(${component_name})
    
    add_dependencies(csl_lib csl::${component_name})
    target_link_libraries(csl_lib INTERFACE csl::${component_name})
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)
