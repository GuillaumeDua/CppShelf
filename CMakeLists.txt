cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# top-level project
project(csl LANGUAGES CXX)

set(PROJECT_VERSION                 0.3.1) # not relying on latest tag to avoid unecessary stale targets
set(CMAKE_EXPORT_COMPILE_COMMANDS   ON)

if (${PROJECT_SOURCE_DIR} STREQUAL ${PROJECT_BINARY_DIR})
    message(FATAL_ERROR "[${PROJECT_NAME}] : In-source build not recommended")
endif()

# Dependencies: caching
if (NOT DEFINED CPM_SOURCE_CACHE AND NOT DEFINED ENV{CPM_SOURCE_CACHE} AND DEFINED ENV{HOME})
    set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM" CACHE PATH "[${PROJECT_NAME}] CPM_SOURCE_CACHE - shared across build directories/presets")
endif()

# Dependencies: local vs. fetch
if (NOT DEFINED CPM_USE_LOCAL_PACKAGES AND NOT DEFINED ENV{CPM_USE_LOCAL_PACKAGES})
    set(CPM_USE_LOCAL_PACKAGES ON CACHE BOOL "[${PROJECT_NAME}] CPM_USE_LOCAL_PACKAGES - let CPMAddPackage() try find_package() before fetching")
endif()

find_program(CSL_CCACHE_PROGRAM ccache)
if (CSL_CCACHE_PROGRAM AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CSL_CCACHE_PROGRAM} CACHE STRING "[${PROJECT_NAME}] CXX compiler launcher")
    set(CMAKE_C_COMPILER_LAUNCHER   ${CSL_CCACHE_PROGRAM} CACHE STRING "[${PROJECT_NAME}] C compiler launcher")
endif()

include(CMakeDependentOption)
include(CMakeParseArguments)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

message("[${PROJECT_NAME}] Exporting csl cmake modules as [${CMAKE_CURRENT_SOURCE_DIR}/cmake]")
set(CSL_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake CACHE PATH "[${PROJECT_NAME}] path to exported cmake modules")
list(APPEND CMAKE_MODULE_PATH ${CSL_CMAKE_MODULE_PATH})

# Internal modules
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/internal/csl/add_component.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/internal/csl/compile_warnings.cmake)
# Exported modules
include(csl/print_aligned)
include(csl/cache/entry_conversion)
include(csl/cache/multi_cache_support)
include(csl/cache/print_build_info)
include(csl/list_subdirectories)

csl_symlink_compile_commands()

## Build informations
message(STATUS "[${PROJECT_NAME}] Build informations")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
csl_print_build_info()
list(POP_BACK CMAKE_MESSAGE_INDENT)

## Components - detection, list
csl_list_subdirectories(
    DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}/libs
    RESULT      CSL_COMPONENTS_LIST
)
message(VERBOSE "[${PROJECT_NAME}] Available components: [${CSL_COMPONENTS_LIST}]")
message(STATUS  "[${PROJECT_NAME}] Global configuration")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

set(CSL_ENABLED_COMPONENTS_LIST ${CSL_COMPONENTS_LIST})

list(LENGTH CSL_ENABLED_COMPONENTS_LIST CSL_ENABLED_COMPONENTS_LIST_length)
if (${CSL_ENABLED_COMPONENTS_LIST_length} EQUAL 0)
    message(WARNING "[${PROJECT_NAME}] No components found under libs/, is it intended ? Aborting.")
    return()
endif()

#### Install switch: all-or-nothing (a single toggle, no per-component granularity).
# Defaults to PROJECT_IS_TOP_LEVEL:
# - ON for a standalone csl build, so `cmake --install` works out of the box,
# - OFF when csl is consumed via add_subdirectory/FetchContent, so a parent's `cmake --install`
#   never accidentally ships csl's headers + export sets.
option(CSL_INSTALL "[${PROJECT_NAME}] install/package all components" ${PROJECT_IS_TOP_LEVEL})
csl_print_aligned(STATUS CSL_INSTALL)

#### Per-component work-gating options (opt-outs): each gates real compilation/generation.
set(CSL_OPTIONS_LIST
    TEST # TECH-DEBT: clang sanitizers, etc.
    EXAMPLE
    DOC
    BENCHMARK
)
foreach(option_name IN LISTS CSL_OPTIONS_LIST)
    option(CSL_${option_name} "[${PROJECT_NAME}] default ${option_name} for all csl components (override per component via CSL_${option_name}_<name>)" OFF)
    csl_print_aligned(STATUS CSL_${option_name})

    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        option(CSL_${option_name}_${component_name} "[${PROJECT_NAME}] csl::${component_name} (${option_name})" ${CSL_${option_name}})
        csl_print_aligned(STATUS CSL_${option_name}_${component_name}
            depends CSL_${option_name}
        )
    endforeach()
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

# Internal test-support target (csl::internal::test): built from internals/libs/test/,
# NOT a public component (never discovered as a libs/ component, exported, nor installed).
# Each component's test suite can link with csl::internal::test and Catch2.
set(_csl_any_tests_enabled OFF)
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    if (CSL_TEST_${component_name})
        set(_csl_any_tests_enabled ON)
        break()
    endif()
endforeach()

if (_csl_any_tests_enabled)
    enable_testing()
    message(VERBOSE "[${PROJECT_NAME}] adding internal test-support target csl::internal::test")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/internals/libs/test)
endif()

message(VERBOSE "[${PROJECT_NAME}] Build [${PROJECT_NAME}] meta-library, aggregating enabled components: [${CSL_ENABLED_COMPONENTS_LIST}]")
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

message(STATUS  "[${PROJECT_NAME}] Adding components ...")
list(APPEND CMAKE_MESSAGE_INDENT "   ")
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    # NOTE: a component may have already been added by another component's tests
    # (self-provisioned as an optional, loosely-coupled integration, e.g. csl::ag's tests <-> csl::typeinfo).
    # Guard against adding it twice regardless of processing order.
    if (NOT TARGET ${PROJECT_NAME}::${component_name})
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/${component_name})
    endif()

    add_dependencies(${PROJECT_NAME} csl::${component_name})
    target_link_libraries(${PROJECT_NAME} INTERFACE csl::${component_name})
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

# Install and package: find_package(csl) support (all-or-nothing, gated on CSL_INSTALL).
# - Per-component install/export rules live in csl_add_component().
# - Top-level: the aggregate csl::csl target, package config + version file, and CPack archives.
if (CSL_INSTALL)

    set(CSL_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    # Aggregate csl::csl meta-target (links every component; all components are installed).
    install(TARGETS ${PROJECT_NAME} EXPORT csl-targets)
    install(EXPORT  csl-targets
        FILE        csl-targets.cmake
        NAMESPACE   ${PROJECT_NAME}::
        DESTINATION ${CSL_CMAKE_INSTALL_DIR}
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/csl/cslConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cslConfig.cmake"
        INSTALL_DESTINATION ${CSL_CMAKE_INSTALL_DIR}
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/cslConfigVersion.cmake"
        VERSION         ${PROJECT_VERSION}
        COMPATIBILITY   SameMajorVersion
        ARCH_INDEPENDENT
    )
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/cslConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/cslConfigVersion.cmake"
        DESTINATION ${CSL_CMAKE_INSTALL_DIR}
    )

    # Release archives (consumed by .github/workflows/csl-release.yml via CPack)
    set(CPACK_PACKAGE_NAME                  "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR                "Guillaume Dua")
    set(CPACK_PACKAGE_VERSION               "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "CppShelf (csl): C++ single-header, header-only libraries")
    set(CPACK_PACKAGE_HOMEPAGE_URL          "https://github.com/GuillaumeDua/CppShelf")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    endif()
    set(CPACK_GENERATOR                     "TGZ;ZIP")
    set(CPACK_SOURCE_GENERATOR              "TGZ;ZIP")
    include(CPack)
endif()

# Documentation
set(CSL_DOC_ANY ${CSL_DOC})
if (NOT CSL_DOC_ANY)
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        if (CSL_DOC_${component_name})
            set(CSL_DOC_ANY ON)
            break()
        endif()
    endforeach()
endif()

if (CSL_DOC_ANY)
    include(csl/graphviz)
    csl_graphviz(
        TARGET       ${PROJECT_NAME}
        OPTIONS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMakeGraphVizOptions.cmake"
    )
endif()
