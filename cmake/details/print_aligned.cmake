
function(check_option option_name)
    message(CHECK_START "${option_name}")
    if (${option_name})
        message(CHECK_PASS "enabled")
    else()
        message(CHECK_FAIL "disabled")
    endif()
endfunction()

# print_aligned(log_level variable [width] [filler_char])
function(print_aligned log_level variable)

    if (NOT DEFINED variable)
        message(FATAL_ERROR "[print_aligned] argument [variable] is not defined")
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
        set(arg_width 40)
    endif()

    if (NOT DEFINED arg_filler_char OR arg_filler_char STREQUAL "")
        set(arg_filler_char ".")
    endif()

    if (NOT DEFINED ${variable})
        set(variable "<undefined>")
    endif()

    if (NOT "${arg_width}" MATCHES "^[0-9]+$")
        message(FATAL_ERROR "[print_aligned] argument [arg_width] must be a non-negative integer, got [${arg_width}]")
    endif()

    if ("${CMAKE_MESSAGE_INDENT}" STREQUAL "")
        set(label "[${PROJECT_NAME}] ")
    else()
        set(label "")
    endif()

    string(LENGTH "${label}" label_length)
    string(LENGTH "${variable}" name_length)
    math(EXPR padding_length "${arg_width} - ${name_length} - ${label_length}")

    if (padding_length GREATER 0)
        string(REPEAT "${arg_filler_char}" ${padding_length} padding)
    else()
        set(padding " ")
    endif()



    message(${log_level} "${label}${variable} ${padding} ${${variable}}")
endfunction()

