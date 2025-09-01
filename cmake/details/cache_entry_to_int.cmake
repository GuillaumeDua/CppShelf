function(cache_entry_to_int)

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

    list(FIND ${arg_ID}__allowed_values "${${arg_ID}}" index)
    if (index EQUAL -1)
        message(FATAL_ERROR "[${CMAKE_PROJECT_NAME}] csl::${component_name}: Invalid value for ${arg_ID}: ${${arg_ID}}")
    endif()
    set(${arg_OUT_VAR} ${index} PARENT_SCOPE)
endfunction()
