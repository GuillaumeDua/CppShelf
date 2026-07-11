# --- Options ---
cmake_policy(SET CMP0127 NEW)
include(CMakeDependentOption)

# Configuration macros are BUILD_INTERFACE-only:
#   they apply while building csl itself (tests, examples) but are stripped from the installed/exported target,
#   so the package stays neutral and dependency-free.
# Formatting backends are NOT configured here: opt-in = #include of a feature header
#   (csl/ag/formatting/{format,fmt,ostream,typeinfo}.hpp) - no CMake option, no macro.

# CSL_AG__VERBOSE_BUILD
option(CSL_AG__VERBOSE_BUILD "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: verbose build (might use additional useful build messages)" OFF)
csl_print_aligned(STATUS CSL_AG__VERBOSE_BUILD)
if (CSL_AG__VERBOSE_BUILD)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE $<BUILD_INTERFACE:CSL_AG__VERBOSE_BUILD>)
endif()

# CSL_AG__ENABLE_BITFIELDS_SUPPORT
option(CSL_AG__ENABLE_BITFIELDS_SUPPORT "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: enable bitfields support (slower compilation)" OFF)
csl_print_aligned(STATUS CSL_AG__ENABLE_BITFIELDS_SUPPORT)
if (CSL_AG__ENABLE_BITFIELDS_SUPPORT)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE $<BUILD_INTERFACE:CSL_AG__ENABLE_BITFIELDS_SUPPORT>)
endif()

# --- code generation ---

# CSL_AG__USE_EMBEDDED_IMPLEMENTATION
option(CSL_AG__USE_EMBEDDED_IMPLEMENTATION "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: force use of embedded implementation instead of CMake-generated files (faster, header-only mode)" OFF)
csl_print_aligned(STATUS CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
if (CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
    target_compile_definitions(csl_${csl_add_component_NAME} INTERFACE $<BUILD_INTERFACE:CSL_AG__USE_EMBEDDED_IMPLEMENTATION>)
endif()

# TODO(Guillaume) max supported sizeof

## CSL_AG__MAX_SUPPORTED_FIELDS_COUNT
if (CSL_AG__USE_EMBEDDED_IMPLEMENTATION)
    set(CSL_AG__MAX_SUPPORTED_FIELDS_COUNT 32  CACHE STRING "csl::${csl_add_component_NAME} : max fields count for aggregate to reflect (fixed to 32 when CSL_AG__USE_EMBEDDED_IMPLEMENTATION is ON)" FORCE)
else()
    set(CSL_AG__MAX_SUPPORTED_FIELDS_COUNT 64 CACHE STRING "csl::${csl_add_component_NAME} : max fields count for aggregate to reflect")
endif()
csl_print_aligned(STATUS CSL_AG__MAX_SUPPORTED_FIELDS_COUNT)
if (NOT CSL_AG__MAX_SUPPORTED_FIELDS_COUNT MATCHES "^[0-9]+$")
    message(FATAL_ERROR "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}] : CSL_AG__MAX_SUPPORTED_FIELDS_COUNT is not a valid number")
endif()

if (NOT CSL_AG__USE_EMBEDDED_IMPLEMENTATION)

    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/generate_cpp_code.cmake)
    ag_generate_cpp_code(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    target_include_directories(${csl_add_component_PROJECT_NAME}_${csl_add_component_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )

    # Ship the CMake-generated headers so the installed package keeps build parity (up to CSL_AG__MAX_SUPPORTED_FIELDS_COUNT fields).
    # NOTE: include path is already covered by the main target's $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> (files land under csl/ag/generated/).
    if (CSL_INSTALL)
        install(DIRECTORY   "${CMAKE_CURRENT_BINARY_DIR}/csl"
            DESTINATION     "${CMAKE_INSTALL_INCLUDEDIR}"
        )
    endif()
endif()
