# --- Options ---
cmake_policy(SET CMP0127 NEW)
include(CMakeDependentOption)

# CSL_AG__VERBOSE_BUILD
option(CSL_AG__VERBOSE_BUILD "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: verbose build (might use additional useful build messages)" OFF)
print_aligned(STATUS CSL_AG__VERBOSE_BUILD)
if (CSL_AG__VERBOSE_BUILD)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__VERBOSE_BUILD)
endif()

# CSL_AG__ENABLE_BITFIELDS_SUPPORT
option(CSL_AG__ENABLE_BITFIELDS_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable bitfields support (slower compilation)" OFF)
print_aligned(STATUS CSL_AG__ENABLE_BITFIELDS_SUPPORT)
if (CSL_AG__ENABLE_BITFIELDS_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_BITFIELDS_SUPPORT)
endif()

# CSL_AG__ENABLE_IOSTREAM_SUPPORT
option(CSL_AG__ENABLE_IOSTREAM_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable std::format support" OFF)
print_aligned(STATUS CSL_AG__ENABLE_IOSTREAM_SUPPORT)
if (CSL_AG__ENABLE_IOSTREAM_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_IOSTREAM_SUPPORT)
endif()

# CSL_AG__ENABLE_FORMAT_SUPPORT
option(CSL_AG__ENABLE_FORMAT_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable std::format support" OFF)
print_aligned(STATUS CSL_AG__ENABLE_FORMAT_SUPPORT)
if (CSL_AG__ENABLE_FORMAT_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_FORMAT_SUPPORT)
endif()

# CSL_AG__ENABLE_FMTLIB_SUPPORT
option(CSL_AG__ENABLE_FMTLIB_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable fmt support" OFF)
print_aligned(STATUS CSL_AG__ENABLE_FMTLIB_SUPPORT)
if (${CSL_AG__ENABLE_FMTLIB_SUPPORT})

    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE CSL_AG__ENABLE_FMTLIB_SUPPORT)

    if (NOT TARGET fmt::fmt-header-only AND NOT TARGET fmt::fmt)

        message(STATUS "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}] - fetching [fmt] library ...")
    
        if (NOT TARGET fmt::fmt-header-only)
            list(APPEND CMAKE_MESSAGE_INDENT "  ")
            set(FETCHCONTENT_QUIET ON)
            include(FetchContent)
            FetchContent_Declare(fmt
                GIT_REPOSITORY  https://github.com/fmtlib/fmt.git
                GIT_TAG         main
            )
            FetchContent_MakeAvailable(fmt)
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        endif()
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
print_aligned(STATUS CSL_AG__ENABLE_CSL_TYPEINFO_SUPPORT)

# --- code generation ---

## CSL_AG__MAX_FIELDS_SUPPORTED_COUNT
set(CSL_AG__DEFAULT_MAX_FIELDS_SUPPORTED_COUNT 32)
set(CSL_AG__MAX_FIELDS_SUPPORTED_COUNT "${CSL_AG__DEFAULT_MAX_FIELDS_SUPPORTED_COUNT}" CACHE STRING "csl::${csl_add_component_NAME} : max fields count for aggregate to reflect")
print_aligned(STATUS CSL_AG__MAX_FIELDS_SUPPORTED_COUNT)
if (NOT CSL_AG__MAX_FIELDS_SUPPORTED_COUNT MATCHES "^[0-9]+$")
    message(FATAL "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}] : CSL_AG__MAX_FIELDS_SUPPORTED_COUNT is not a valid number")
endif()

if (NOT ${CSL_AG__MAX_FIELDS_SUPPORTED_COUNT} STREQUAL ${CSL_AG__DEFAULT_MAX_FIELDS_SUPPORTED_COUNT})
    message(STATUS "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: custom CSL_AG__MAX_FIELDS_SUPPORTED_COUNT requested, generating code ...")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/generate_cpp_code.cmake)
    ag_generate_cpp_code()
endif()
