# --- Options ---
cmake_policy(SET CMP0127 NEW)
include(CMakeDependentOption)

# CSL_AG__VERBOSE_BUILD
option(CSL_AG__VERBOSE_BUILD "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: verbose build (might use additional useful build messages)" OFF)
csl_print_aligned(STATUS CSL_AG__VERBOSE_BUILD)
if (CSL_AG__VERBOSE_BUILD)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__VERBOSE_BUILD)
endif()

# CSL_AG__ENABLE_BITFIELDS_SUPPORT
option(CSL_AG__ENABLE_BITFIELDS_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable bitfields support (slower compilation)" OFF)
csl_print_aligned(STATUS CSL_AG__ENABLE_BITFIELDS_SUPPORT)
if (CSL_AG__ENABLE_BITFIELDS_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_BITFIELDS_SUPPORT)
endif()

# CSL_AG__ENABLE_IOSTREAM_SUPPORT
option(CSL_AG__ENABLE_IOSTREAM_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable std::format support" OFF)
csl_print_aligned(STATUS CSL_AG__ENABLE_IOSTREAM_SUPPORT)
if (CSL_AG__ENABLE_IOSTREAM_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_IOSTREAM_SUPPORT)
endif()

# CSL_AG__ENABLE_FORMAT_SUPPORT
option(CSL_AG__ENABLE_FORMAT_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable std::format support" OFF)
csl_print_aligned(STATUS CSL_AG__ENABLE_FORMAT_SUPPORT)
if (CSL_AG__ENABLE_FORMAT_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_FORMAT_SUPPORT)
endif()

# CSL_AG__ENABLE_FMTLIB_SUPPORT
option(CSL_AG__ENABLE_FMTLIB_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable fmt support" OFF)
csl_print_aligned(STATUS CSL_AG__ENABLE_FMTLIB_SUPPORT)
if (${CSL_AG__ENABLE_FMTLIB_SUPPORT})

    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_FMTLIB_SUPPORT)

    if (NOT TARGET fmt::fmt-header-only AND NOT TARGET fmt::fmt)
        include(csl/get_cpm)
        CPMAddPackage(
            NAME              fmt
            GITHUB_REPOSITORY fmtlib/fmt
            GIT_TAG           12.1.0
        )
    endif()

    if (TARGET fmt::fmt-header-only)
        add_dependencies(csl_${csl_add_component_NAME} fmt::fmt-header-only)
        target_link_libraries(csl_${csl_add_component_NAME} INTERFACE fmt::fmt-header-only)
    elseif(target fmt::fmt)
        add_dependencies(csl_${csl_add_component_NAME} fmt::fmt)
        target_link_libraries(csl_${csl_add_component_NAME} INTERFACE fmt::fmt)
    else()
        message(ERROR "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: unexpected ill-formed fmt library")
    endif()
endif()

# CSL_AG__ENABLE_CSL_TYPEINFO_SUPPORT
cmake_dependent_option(CSL_AG__ENABLE_CSL_TYPEINFO_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable csl::typeinfo support"
    ON
    "EXISTS ${PROJECT_SOURCE_DIR}/libs/typeinfo/includes/typeinfo/csl/typeinfo.hpp"
    OFF
)
csl_print_aligned(STATUS CSL_AG__ENABLE_CSL_TYPEINFO_SUPPORT)

# --- code generation ---

# CSL_AG__USE_EMBEDDED_IMPLEMENTATION
option(CSL_AG__USE_EMBEDDED_IMPLEMENTATION "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: force use of embedded implementation instead of CMake-generated files (faster, header-only mode)" OFF)
csl_print_aligned(STATUS CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
if (CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
endif()

## CSL_AG__MAX_FIELDS_SUPPORTED_COUNT
if (CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
    set(CSL_AG__MAX_FIELDS_SUPPORTED_COUNT 32  CACHE STRING "csl::${csl_add_component_NAME} : max fields count for aggregate to reflect (fixed to 32 when CSL_AG__USE_EMBEDDED_IMPLEMENTATION is ON)" FORCE)
else()
    set(CSL_AG__MAX_FIELDS_SUPPORTED_COUNT 256 CACHE STRING "csl::${csl_add_component_NAME} : max fields count for aggregate to reflect")
endif()
csl_print_aligned(STATUS CSL_AG__MAX_FIELDS_SUPPORTED_COUNT)
if (NOT CSL_AG__MAX_FIELDS_SUPPORTED_COUNT MATCHES "^[0-9]+$")
    message(FATAL "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}] : CSL_AG__MAX_FIELDS_SUPPORTED_COUNT is not a valid number")
endif()

if (NOT CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/generate_cpp_code.cmake)
    ag_generate_cpp_code(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    target_include_directories(${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}"
)
endif()
