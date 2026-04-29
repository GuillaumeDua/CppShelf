include_guard(GLOBAL)

set(CSL_PRINT_ALIGNED_DEFAULT_WIDTH                    50 CACHE STRING "csl/print_aligned: default column width")
set(CSL_PRINT_ALIGNED_ACCOUNT_FOR_CMAKE_MESSAGE_INDENT ON CACHE BOOL   "csl/print_aligned: account for CMAKE_MESSAGE_INDENT in padding calculation")

function(csl_check_option option_name)
    message(CHECK_START "${option_name}")
    if (${option_name})
        message(CHECK_PASS "enabled")
    else()
        message(CHECK_FAIL "disabled")
    endif()
endfunction()

# print_aligned(log_level variable [width] [filler_char])
function(csl_print_aligned log_level variable)

    if (NOT DEFINED variable)
        message(FATAL_ERROR "[csl/print_aligned] argument [variable] is not defined")
    endif()

    set(options "")
    set(oneValueArgs width filler_char depends)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 2 arg "${options}" "${oneValueArgs}" "${multiValueArgs}")

    # Only print if defers from depends
    if (DEFINED ${arg_depends})
        # Boolean normalisation (so TRUE == ON, etc.)
        if ((${${variable}} AND ${${arg_depends}})
        OR  ((NOT ${${variable}}) AND (NOT ${${arg_depends}}))
        )
            return()
        endif()
    endif()

    if (NOT DEFINED arg_width OR arg_width STREQUAL "")
        set(arg_width ${CSL_PRINT_ALIGNED_DEFAULT_WIDTH})
    endif()

    if (NOT DEFINED arg_filler_char OR arg_filler_char STREQUAL "")
        set(arg_filler_char ".")
    endif()

    if (NOT DEFINED ${variable})
        set(variable "<undefined>")
    endif()

    if (NOT "${arg_width}" MATCHES "^[0-9]+$")
        message(FATAL_ERROR "[csl/print_aligned] argument [arg_width] must be a non-negative integer, got [${arg_width}]")
    endif()

    if ("${CMAKE_MESSAGE_INDENT}" STREQUAL "")
        set(label "[${PROJECT_NAME}] ")
    else()
        set(label "")
    endif()

    string(LENGTH "${label}" label_length)
    string(LENGTH "${variable}" name_length)
    if (CSL_PRINT_ALIGNED_ACCOUNT_FOR_CMAKE_MESSAGE_INDENT)
        string(JOIN "" _csl_indent_str ${CMAKE_MESSAGE_INDENT})
        string(LENGTH "${_csl_indent_str}" _csl_indent_length)
    else()
        set(_csl_indent_length 0)
    endif()
    math(EXPR padding_length "${arg_width} - ${name_length} - ${label_length} - ${_csl_indent_length}")

    if (padding_length GREATER 0)
        string(REPEAT "${arg_filler_char}" ${padding_length} padding)
    else()
        message(AUTHOR_WARNING
            "[csl/print_aligned] padding overflow for [${variable}]: width=${arg_width}, label=${label_length}, name=${name_length}."
            "Consider using argument [width <size>] or increasing the cache entry [CSL_PRINT_ALIGNED_DEFAULT_WIDTH]"
        )
        set(padding "")
    endif()

    message(${log_level} "${label}${variable} ${padding} ${${variable}}")

endfunction()

function(csl_print_build_info)
    csl_print_aligned(STATUS CMAKE_BUILD_TYPE)
    csl_print_aligned(STATUS CMAKE_CURRENT_BINARY_DIR)
    csl_print_aligned(STATUS CMAKE_CURRENT_SOURCE_DIR)
    csl_print_aligned(STATUS CMAKE_INSTALL_PREFIX)
    csl_print_aligned(STATUS CMAKE_GENERATOR)
    csl_print_aligned(STATUS CMAKE_SYSTEM_NAME)
    csl_print_aligned(STATUS CMAKE_SYSTEM_PROCESSOR)
    csl_print_aligned(STATUS CMAKE_CXX_COMPILER)
    csl_print_aligned(STATUS CMAKE_CXX_COMPILER_ID)
    csl_print_aligned(STATUS CMAKE_CXX_COMPILER_VERSION)
    csl_print_aligned(STATUS CMAKE_CXX_STANDARD)
endfunction()
