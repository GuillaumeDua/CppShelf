include_guard(GLOBAL)

include(CMakeParseArguments)

macro(csl_list_subdirectories)

    set(options)
    set(oneValueArgs
        DIRECTORY
        RESULT
    )
    set(multiValueArgs )
    cmake_parse_arguments(csl_list_subdirectories
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED csl_list_subdirectories_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_list_subdirectories] error: UNPARSED_ARGUMENTS = [${csl_list_subdirectories_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED csl_list_subdirectories_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_list_subdirectories] error: KEYWORDS_MISSING_VALUES = [${csl_list_subdirectories_KEYWORDS_MISSING_VALUES}]")
    endif()

    file(
        GLOB children_directories
        RELATIVE ${csl_list_subdirectories_DIRECTORY}
        ${csl_list_subdirectories_DIRECTORY}/*)

    set(result "")
    foreach(directory_name ${children_directories})
        if (IS_DIRECTORY ${csl_list_subdirectories_DIRECTORY}/${directory_name})
            list(APPEND result ${directory_name})
        endif()
    endforeach()
    set(${csl_list_subdirectories_RESULT} ${result})
endmacro()
