cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

include_guard(GLOBAL)

include(CMakeParseArguments)

# Usage:
#
#   csl_make_pairs(OUTPUT_PREFIX smthg PAIRS TEST ON EXAMPLE ON)
#   foreach(_key _value IN ZIP_LISTS smthg_keys smthg_values)
#       message(STATUS "${_key} = ${_value}")
#   endforeach()
function(csl_make_pairs)
    cmake_parse_arguments(PARSE_ARGV 0 _mp "" "OUTPUT_PREFIX" "PAIRS")

    if(NOT DEFINED _mp_OUTPUT_PREFIX)
        message(FATAL_ERROR "[csl_make_pairs] error: OUTPUT_PREFIX is required")
    endif()

    list(LENGTH _mp_PAIRS _len)
    math(EXPR _rem "${_len} % 2")
    if(NOT _rem EQUAL 0)
        message(FATAL_ERROR "[csl_make_pairs] error: PAIRS must contain an even number of elements (got ${_len})")
    endif()

    set(_keys "")
    set(_values "")
    if(_len GREATER 0)
        math(EXPR _last "${_len} - 1")
        foreach(_i RANGE 0 ${_last} 2)
            math(EXPR _j "${_i} + 1")
            list(GET _mp_PAIRS ${_i} _k)
            list(GET _mp_PAIRS ${_j} _v)
            list(APPEND _keys "${_k}")
            list(APPEND _values "${_v}")
        endforeach()
    endif()

    set(${_mp_OUTPUT_PREFIX}_keys   "${_keys}"   PARENT_SCOPE)
    set(${_mp_OUTPUT_PREFIX}_values "${_values}" PARENT_SCOPE)
endfunction()
