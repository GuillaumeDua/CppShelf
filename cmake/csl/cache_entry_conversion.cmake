include_guard(GLOBAL)

# Converts a CACHE STRING entry to the index of its current value within the STRINGS property.
# The cache entry must first have its allowed values registered via:
#   set_property(CACHE <var> PROPERTY STRINGS ...)
#
# Usage:
#   csl_cache_entry_to_property_index(ID <cache-var> OUT_VAR <output-var>)
function(csl_cache_entry_to_property_index)

    set(options)
    set(one_value_args ID OUT_VAR)
    set(multi_value_args)
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${one_value_args}" "${multi_value_args}")

    if(NOT DEFINED arg_ID)
        message(FATAL_ERROR "[csl_cache_entry_to_property_index] ID argument is required")
    endif()
    if(NOT DEFINED arg_OUT_VAR)
        message(FATAL_ERROR "[csl_cache_entry_to_property_index] OUT_VAR argument is required")
    endif()

    get_property(_allowed_values CACHE "${arg_ID}" PROPERTY STRINGS)
    if(NOT _allowed_values)
        message(FATAL_ERROR "[csl_cache_entry_to_property_index] cache entry [${arg_ID}] has no STRINGS property — use set_property(CACHE ${arg_ID} PROPERTY STRINGS ...) to define allowed values")
    endif()

    list(FIND _allowed_values "${${arg_ID}}" _index)
    if(_index EQUAL -1)
        message(FATAL_ERROR "[csl_cache_entry_to_property_index] invalid value [${${arg_ID}}] for [${arg_ID}]. Allowed values: [${_allowed_values}]")
    endif()

    set(${arg_OUT_VAR} ${_index} PARENT_SCOPE)
endfunction()

function(csl_cache_entry_to_int)

    set(options)
    set(one_value_args
        ID
        OUT_VAR
        FORCE_VALUE # for test purposes
    )
    set(multi_value_args)
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${one_value_args}" "${multi_value_args}")

    if(NOT DEFINED arg_ID)
        message(FATAL_ERROR "cache_entry_to_int: ID argument is required")
    endif()
    if(NOT DEFINED arg_OUT_VAR)
        message(FATAL_ERROR "cache_entry_to_int: OUT_VAR argument is required")
    endif()

    if(DEFINED arg_FORCE_VALUE)
        set(${arg_ID} "${arg_FORCE_VALUE}")
    endif()

    # default is always 0
    if("${${arg_ID}}" STREQUAL "")
        set(${arg_OUT_VAR} 0 PARENT_SCOPE)
        return()
    endif()

    # numeric
    if("${${arg_ID}}" MATCHES "^[0-9]+$")
        set(${arg_OUT_VAR} ${${arg_ID}} PARENT_SCOPE)
        return()
    endif()
    
    # boolean
    if  ("${${arg_ID}}" STREQUAL "ON"
      OR "${${arg_ID}}" STREQUAL "TRUE")
        set(${arg_OUT_VAR} 1 PARENT_SCOPE)
        return()
    elseif("${${arg_ID}}" STREQUAL "OFF"
        OR "${${arg_ID}}" STREQUAL "FALSE")
        set(${arg_OUT_VAR} 0 PARENT_SCOPE)
        return()
    endif()

    # result
    set(${arg_OUT_VAR} ${index} PARENT_SCOPE)
endfunction()
