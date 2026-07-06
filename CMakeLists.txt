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

### Components - build
option(CSL_BUILD_ALL "[${PROJECT_NAME}] Build all components" ON) # force all components build
csl_print_aligned(STATUS CSL_BUILD_ALL)

list(APPEND CMAKE_MESSAGE_INDENT "  ")
foreach(component_name IN LISTS CSL_COMPONENTS_LIST)

    option(CSL_BUILD_${component_name} "[${PROJECT_NAME}] Build ${component_name}" ${CSL_BUILD_ALL})
    csl_print_aligned(STATUS CSL_BUILD_${component_name}
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

#### Common options (opt-outs)
set(CSL_OPTIONS_LIST
    TEST # TECH-DEBT: clang sanitizers, etc.
    EXAMPLE
    INSTALL
    DOC
    BENCHMARK
)
foreach(option_name IN LISTS CSL_OPTIONS_LIST)

    # INSTALL auto-enable: defaults to PROJECT_IS_TOP_LEVEL
    # - ON for a standalone csl build, so `cmake --install` works out of the box,
    # - OFF when csl is consumed via add_subdirectory/FetchContent, so parent's `cmake --install` never accidentally ships csl's headers + export sets).
    set(_csl_option_all_default OFF)
    if (option_name STREQUAL "INSTALL")
        set(_csl_option_all_default ${PROJECT_IS_TOP_LEVEL})
    endif()
    option(CSL_${option_name}_ALL "${PROJECT_NAME}: Enable all ${option_name} for all csl components (can be override per component)" ${_csl_option_all_default})
    csl_print_aligned(STATUS CSL_${option_name}_ALL)

    list(APPEND CMAKE_MESSAGE_INDENT "  ")
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        option(CSL_${option_name}_${component_name} "[${PROJECT_NAME}] csl::${component_name} (${option_name})" ${CSL_${option_name}_ALL})
        csl_print_aligned(STATUS CSL_${option_name}_${component_name}
            depends CSL_${option_name}_ALL
        )
    endforeach()
    list(POP_BACK CMAKE_MESSAGE_INDENT)
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

# Tests support auto-enable
list(FIND CSL_ENABLED_COMPONENTS_LIST test _csl_auto_test_index)
if (_csl_auto_test_index EQUAL -1)
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        if (CSL_TEST_${component_name}
            AND NOT component_name STREQUAL "test"
        )
            message(VERBOSE "[${PROJECT_NAME}] csl::test auto-enabled (required by at least [csl::${component_name}] tests)")
            list(APPEND CSL_ENABLED_COMPONENTS_LIST test)
            break()
        endif()
    endforeach()
endif()

message(VERBOSE "[${PROJECT_NAME}] Build [${PROJECT_NAME}] meta-library, aggregating enabled components: [${CSL_ENABLED_COMPONENTS_LIST}]")
message(STATUS  "[${PROJECT_NAME}] Adding components ...")
list(APPEND CMAKE_MESSAGE_INDENT "   ")

list(FIND CSL_ENABLED_COMPONENTS_LIST test _csl_test_index)
if (_csl_test_index GREATER -1)
    list(REMOVE_AT CSL_ENABLED_COMPONENTS_LIST ${_csl_test_index})
    list(INSERT CSL_ENABLED_COMPONENTS_LIST 0 test)
    enable_testing()
endif()

# The csl::csl meta-target aggregates ALL public components.
# WARNING: it is created ONLY when all csl components/libraries are enabled.
set(CSL_ALL_COMPONENTS_ENABLED TRUE)
foreach(component_name IN LISTS CSL_COMPONENTS_LIST)
    if (NOT component_name STREQUAL "test")
        list(FIND CSL_ENABLED_COMPONENTS_LIST ${component_name} _csl_enabled_index)
        if (_csl_enabled_index EQUAL -1)
            set(CSL_ALL_COMPONENTS_ENABLED FALSE)
            break()
        endif()
    endif()
endforeach()

if (CSL_ALL_COMPONENTS_ENABLED)
    message(STATUS "[${PROJECT_NAME}] all components are enabled, creating [${PROJECT_NAME}::${PROJECT_NAME}] meta-target")
    add_library(${PROJECT_NAME} INTERFACE)
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
else()
    message(VERBOSE "[${PROJECT_NAME}] some components are disabled, preventing [${PROJECT_NAME}::${PROJECT_NAME}] meta-target creation")
endif()

foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    # FIXME ?
    # NOTE: a component may have already been added by another component's tests
    # - Self-provisioned as an optional, loosely-coupled integration, e.g. csl::ag's tests <-> csl::typeinfo).
    # - Guard against adding it twice regardless of processing order.
    if (NOT TARGET ${PROJECT_NAME}::${component_name})
        add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/${component_name})
    endif()

    # Wire the meta-target only when it exists (full build), and keep the internal csl::test helper out of it.
    if (TARGET ${PROJECT_NAME} AND NOT component_name STREQUAL "test")
        add_dependencies(${PROJECT_NAME} csl::${component_name})
        target_link_libraries(${PROJECT_NAME} INTERFACE csl::${component_name})
    endif()
endforeach()
list(POP_BACK CMAKE_MESSAGE_INDENT)

# Install and package: find_package(csl) support
# - Per-component install/export rules live in csl_add_component() (gated on CSL_INSTALL_<name>).
# - Top-level: package config + version file, the optional aggregate csl::csl target, and CPack archives for the release workflow.
set(CSL_INSTALLED_COMPONENTS "")
foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
    # csl::test is an internal test-support helper, never shipped in the package.
    if (CSL_INSTALL_${component_name} AND NOT component_name STREQUAL "test")
        list(APPEND CSL_INSTALLED_COMPONENTS ${component_name})
    endif()
endforeach()

# WIP: csl aggregate package vs. csl_<component>.cmake packages.
# Or it is OK to disable some components ? -> for consistency, check what Boost and other library do ?

if (CSL_INSTALLED_COMPONENTS)

    set(CSL_CMAKE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

    # Public components linked into the csl meta-target (all enabled except internal csl::test).
    set(CSL_PUBLIC_COMPONENTS "")
    foreach(component_name IN LISTS CSL_ENABLED_COMPONENTS_LIST)
        if (NOT component_name STREQUAL "test")
            list(APPEND CSL_PUBLIC_COMPONENTS ${component_name})
        endif()
    endforeach()

    # Aggregate csl::csl meta-target: exported only when it exists (full build).
    # - Every public component it links must itself be installed/exported, otherwise install(EXPORT) would reference a missing target.
    # - csl::test is internal and never installed, so it is excluded from both lists
    # The only way they can differ is a per-component CSL_INSTALL_<name>=OFF on an enabled component, which is unsupported here.
    if (TARGET ${PROJECT_NAME})
        if (NOT CSL_INSTALLED_COMPONENTS STREQUAL CSL_PUBLIC_COMPONENTS)
            message(FATAL_ERROR
                "[${PROJECT_NAME}] csl::csl requires every public component to be installed, "
                "but installed=[${CSL_INSTALLED_COMPONENTS}] differs from public=[${CSL_PUBLIC_COMPONENTS}]. "
                "Do not disable CSL_INSTALL_<name> for an enabled component on a full build."
            )
        endif()
        install(TARGETS ${PROJECT_NAME} EXPORT csl-targets)
        install(EXPORT  csl-targets
            FILE        csl-targets.cmake
            NAMESPACE   ${PROJECT_NAME}::
            DESTINATION ${CSL_CMAKE_INSTALL_DIR}
        )
    endif()

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
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "CppShelf (csl): single-header, header-only C++ libraries")
    set(CPACK_PACKAGE_HOMEPAGE_URL          "https://github.com/GuillaumeDua/CppShelf")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    endif()
    set(CPACK_GENERATOR                     "TGZ;ZIP")
    set(CPACK_SOURCE_GENERATOR              "TGZ;ZIP")
    include(CPack)
endif()

# Documentation
set(CSL_DOC_ANY ${CSL_DOC_ALL})
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
