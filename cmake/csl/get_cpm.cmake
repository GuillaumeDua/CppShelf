include_guard(GLOBAL)

set(CSL_GET_CPM_DEFAULT_VERSION "0.42.1" CACHE STRING "Default CPM.cmake version used by csl_get_cpm()")

# csl_get_cpm([VERSION <version>])
#
#   Downloads CPM.cmake at the pinned default version (or an explicit override) and includes it.
#   Respects CPM_SOURCE_CACHE / ENV{CPM_SOURCE_CACHE} for the download location, falls back to ${CMAKE_BINARY_DIR}/cmake/.
macro(csl_get_cpm)

    cmake_parse_arguments(_csl_get_cpm_arg "" "VERSION" "" ${ARGN})

    if (DEFINED _csl_get_cpm_arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_get_cpm] unexpected arguments: ${_csl_get_cpm_arg_UNPARSED_ARGUMENTS}")
    endif ()

    if (NOT DEFINED _csl_get_cpm_arg_VERSION)
        set(_csl_get_cpm_arg_VERSION "${CSL_GET_CPM_DEFAULT_VERSION}")
    endif ()

    if (CPM_SOURCE_CACHE)
        set(_csl_get_cpm_location "${CPM_SOURCE_CACHE}/cpm/CPM_${_csl_get_cpm_arg_VERSION}.cmake")
    elseif (DEFINED ENV{CPM_SOURCE_CACHE})
        set(_csl_get_cpm_location "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${_csl_get_cpm_arg_VERSION}.cmake")
    else()
        set(_csl_get_cpm_location "${CMAKE_BINARY_DIR}/cmake/CPM_${_csl_get_cpm_arg_VERSION}.cmake")
    endif()

    # expand `~` -> $HOME
    if (_csl_get_cpm_location MATCHES "^~")
        string(REGEX REPLACE "^~" "$ENV{HOME}" _csl_get_cpm_location "${_csl_get_cpm_location}")
    endif ()

    if (NOT EXISTS "${_csl_get_cpm_location}")
        
        message(STATUS "[csl_get_cpm] Downloading CPM.cmake v${_csl_get_cpm_arg_VERSION} to ${_csl_get_cpm_location}")
        get_filename_component(_csl_get_cpm_dir "${_csl_get_cpm_location}" DIRECTORY)
        file(MAKE_DIRECTORY "${_csl_get_cpm_dir}")
        
        set(_csl_download_error 1)
        set(_csl_download_max_attempt 5)
        
        foreach(_csl_attempt RANGE 1 ${_csl_download_max_attempt})
            file(DOWNLOAD
                "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${_csl_get_cpm_arg_VERSION}/CPM.cmake"
                "${_csl_get_cpm_location}"
                STATUS _csl_download_status
            )
            list(GET _csl_download_status 0 _csl_download_error)
            if (NOT _csl_download_error)
                break()
            endif ()
            list(GET _csl_download_status 1 _csl_download_message)
            message(STATUS "[csl_get_cpm] Download attempt ${_csl_attempt}/${_csl_download_max_attempt} failed: ${_csl_download_message}")
            file(REMOVE "${_csl_get_cpm_location}")
        endforeach()
        
        if (_csl_download_error)
            message(FATAL_ERROR "[csl_get_cpm] Failed to download CPM.cmake: ${_csl_download_message}")
        endif ()

        unset(_csl_get_cpm_dir)
        unset(_csl_download_status)
        unset(_csl_download_error)
        unset(_csl_download_max_attempt)
        unset(_csl_download_message)
        unset(_csl_attempt)
    endif()

    if (NOT EXISTS "${_csl_get_cpm_location}")
        message(FATAL_ERROR "[csl_get_cpm] CPM.cmake not found at: ${_csl_get_cpm_location}")
    endif ()
    include("${_csl_get_cpm_location}")

    unset(_csl_get_cpm_arg_VERSION)
    unset(_csl_get_cpm_arg_UNPARSED_ARGUMENTS)
    unset(_csl_get_cpm_arg_KEYWORDS_MISSING_VALUES)
    unset(_csl_get_cpm_location)
endmacro()
