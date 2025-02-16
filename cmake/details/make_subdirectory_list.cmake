include (CMakeParseArguments)

MACRO(MAKE_SUBDIRECTORY_LIST)

    set(options)
    set(oneValueArgs
        DIRECTORY
        RESULT
    )
    set(multiValueArgs )
    cmake_parse_arguments(MAKE_SUBDIRECTORY_LIST
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED MAKE_SUBDIRECTORY_LIST_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[MAKE_SUBDIRECTORY_LIST] error: UNPARSED_ARGUMENTS = [${MAKE_SUBDIRECTORY_LIST_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED MAKE_SUBDIRECTORY_LIST_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[MAKE_SUBDIRECTORY_LIST] error: KEYWORDS_MISSING_VALUES = [${MAKE_SUBDIRECTORY_LIST_KEYWORDS_MISSING_VALUES}]")
    endif()

    FILE(
        GLOB children_directories
        RELATIVE ${MAKE_SUBDIRECTORY_LIST_DIRECTORY}
        ${MAKE_SUBDIRECTORY_LIST_DIRECTORY}/*)

    SET(result "")
    FOREACH(directory_name ${children_directories})
        IF(IS_DIRECTORY ${MAKE_SUBDIRECTORY_LIST_DIRECTORY}/${directory_name})
            LIST(APPEND result ${directory_name})
        ENDIF()
    ENDFOREACH()
    SET(${MAKE_SUBDIRECTORY_LIST_RESULT} ${result})
ENDMACRO()
