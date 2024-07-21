# --- Options ---
# CSL_AG__VERBOSE_BUILD
option(CSL_AG__VERBOSE_BUILD "[${CMAKE_PROJECT_NAME}] csl::${component_name}: verbose build (might use additional useful build messages)" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_AG__VERBOSE_BUILD set to [${CSL_AG__VERBOSE_BUILD}]")
if (CSL_AG__VERBOSE_BUILD)
    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_AG__VERBOSE_BUILD)
endif()

# CSL_AG__ENABLE_BITFIELDS_SUPPORT
option(CSL_AG__ENABLE_BITFIELDS_SUPPORT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: enable bitfields support (slower compilation)" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_AG__ENABLE_BITFIELDS_SUPPORT set to [${CSL_AG__ENABLE_BITFIELDS_SUPPORT}]")
if (CSL_AG__ENABLE_BITFIELDS_SUPPORT)
    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_AG__ENABLE_BITFIELDS_SUPPORT)
endif()

# CSL_AG__ENABLE_IOSTREAM_SUPPORT
option(CSL_AG__ENABLE_IOSTREAM_SUPPORT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: enable std::format support" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_AG__ENABLE_IOSTREAM_SUPPORT set to [${CSL_AG__ENABLE_IOSTREAM_SUPPORT}]")
if (CSL_AG__ENABLE_IOSTREAM_SUPPORT)
    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_AG__ENABLE_IOSTREAM_SUPPORT)
endif()

# CSL_AG__ENABLE_FORMAT_SUPPORT
option(CSL_AG__ENABLE_FORMAT_SUPPORT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: enable std::format support" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_AG__ENABLE_FORMAT_SUPPORT set to [${CSL_AG__ENABLE_FORMAT_SUPPORT}]")
if (CSL_AG__ENABLE_FORMAT_SUPPORT)
    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_AG__ENABLE_FORMAT_SUPPORT)
endif()

# CSL_AG__ENABLE_FMTLIB_SUPPORT
option(CSL_AG__ENABLE_FMTLIB_SUPPORT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: enable fmt support" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_AG__ENABLE_FMTLIB_SUPPORT set to [${CSL_AG__ENABLE_FMTLIB_SUPPORT}]")
if (${CSL_AG__ENABLE_FMTLIB_SUPPORT})

    set(FETCHCONTENT_QUIET ON)
    message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} fetching [fmt] library ...")
   
    if (NOT TARGET fmt::fmt-header-only)
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include(FetchContent)
        FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(fmt)
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    target_compile_definitions(csl_${component_name}_lib INTERFACE CSL_AG__ENABLE_FMTLIB_SUPPORT)
    if (TARGET fmt::fmt-header-only)
        add_dependencies(csl_${component_name}_lib fmt::fmt-header-only)
        target_link_libraries(csl_${component_name}_lib INTERFACE fmt::fmt-header-only)
    elseif(target fmt::fmt)
        add_dependencies(csl_${component_name}_lib fmt::fmt)
        target_link_libraries(csl_${component_name}_lib INTERFACE fmt::fmt)
    else()
        message(ERROR "[${CMAKE_PROJECT_NAME}] csl::${component_name}: ill-formed fmt library")
    endif()
endif()

# --- code generation ---

## CSL_AG__MAX_FIELDS_SUPPORTED_COUNT
set(CSL_AG__DEFAULT_MAX_FIELDS_SUPPORTED_COUNT 32)
set(CSL_AG__MAX_FIELDS_SUPPORTED_COUNT "${CSL_AG__DEFAULT_MAX_FIELDS_SUPPORTED_COUNT}" CACHE STRING "csl::${component_name} : max fields count for aggregate to reflect")
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_AG__MAX_FIELDS_SUPPORTED_COUNT set to [${CSL_AG__MAX_FIELDS_SUPPORTED_COUNT}]")
if (NOT CSL_AG__MAX_FIELDS_SUPPORTED_COUNT MATCHES "^[0-9]+$")
    message(FATAL "[${CMAKE_PROJECT_NAME}] csl::${component_name} : CSL_AG__MAX_FIELDS_SUPPORTED_COUNT is not a valid number")
endif()

if (NOT ${CSL_AG__MAX_FIELDS_SUPPORTED_COUNT} STREQUAL ${CSL_AG__DEFAULT_MAX_FIELDS_SUPPORTED_COUNT})
    message(VERBOSE )
    include(${PROJECT_SOURCE_DIR}/cmake/ag_generate_cpp_code.cmake)
    ag_generate_cpp_code()
endif()
