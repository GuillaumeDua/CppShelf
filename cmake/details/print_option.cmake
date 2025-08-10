
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

    set(options "")
    set(oneValueArgs width filler_char)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 3 arg "${options}" "${oneValueArgs}" "${multiValueArgs}")

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

    string(LENGTH "${variable}" name_length)
    math(EXPR padding_length "${arg_width} - ${name_length}")

    if (padding_length GREATER 0)
        string(REPEAT "${arg_filler_char}" ${padding_length} padding)
    else()
        set(padding " ")
    endif()

    message(${log_level} "${variable} ${padding} ${${variable}}")
endfunction()

