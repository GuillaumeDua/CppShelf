include_guard(GLOBAL)

include(CMakeParseArguments)

macro(csl_make_subdirectory_list)

    set(options)
    set(oneValueArgs
        DIRECTORY
        RESULT
    )
    set(multiValueArgs )
    cmake_parse_arguments(csl_make_subdirectory_list
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED csl_make_subdirectory_list_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_make_subdirectory_list] error: UNPARSED_ARGUMENTS = [${csl_make_subdirectory_list_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED csl_make_subdirectory_list_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_make_subdirectory_list] error: KEYWORDS_MISSING_VALUES = [${csl_make_subdirectory_list_KEYWORDS_MISSING_VALUES}]")
    endif()

    file(
        GLOB children_directories
        RELATIVE ${csl_make_subdirectory_list_DIRECTORY}
        ${csl_make_subdirectory_list_DIRECTORY}/*)

    set(result "")
    foreach(directory_name ${children_directories})
        if (IS_DIRECTORY ${csl_make_subdirectory_list_DIRECTORY}/${directory_name})
            list(APPEND result ${directory_name})
        endif()
    endforeach()
    set(${csl_make_subdirectory_list_RESULT} ${result})
endmacro()
