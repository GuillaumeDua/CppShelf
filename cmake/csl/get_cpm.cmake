include_guard(GLOBAL)

set(_csl_get_cpm_default_version "0.42.1")

# csl_get_cpm([VERSION <version>])
#
# Downloads CPM.cmake at the pinned default version (or an explicit override) and includes it.
# Respects CPM_SOURCE_CACHE / ENV{CPM_SOURCE_CACHE} for the download location, falls back to ${CMAKE_BINARY_DIR}/cmake/.
macro(csl_get_cpm)
    cmake_parse_arguments(_csl_get_cpm_arg "" "VERSION" "" ${ARGN})

    if (DEFINED _csl_get_cpm_arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_get_cpm] unexpected arguments: ${_csl_get_cpm_arg_UNPARSED_ARGUMENTS}")
    endif ()

    if (NOT DEFINED _csl_get_cpm_arg_VERSION)
        set(_csl_get_cpm_arg_VERSION "${_csl_get_cpm_default_version}")
    endif ()

    if (CPM_SOURCE_CACHE)
        set(_csl_get_cpm_location "${CPM_SOURCE_CACHE}/cpm/CPM_${_csl_get_cpm_arg_VERSION}.cmake")
    elseif (DEFINED ENV{CPM_SOURCE_CACHE})
        set(_csl_get_cpm_location "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${_csl_get_cpm_arg_VERSION}.cmake")
    else()
        set(_csl_get_cpm_location "${CMAKE_BINARY_DIR}/cmake/CPM_${_csl_get_cpm_arg_VERSION}.cmake")
    endif()

    if (NOT EXISTS "${_csl_get_cpm_location}")
        message(STATUS "[csl_get_cpm] Downloading CPM.cmake v${_csl_get_cpm_arg_VERSION} to ${_csl_get_cpm_location}")
        file(DOWNLOAD
            "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${_csl_get_cpm_arg_VERSION}/CPM.cmake"
            "${_csl_get_cpm_location}"
        )
    endif()

    include("${_csl_get_cpm_location}")

    unset(_csl_get_cpm_arg_VERSION)
    unset(_csl_get_cpm_arg_UNPARSED_ARGUMENTS)
    unset(_csl_get_cpm_arg_KEYWORDS_MISSING_VALUES)
    unset(_csl_get_cpm_location)
endmacro()
