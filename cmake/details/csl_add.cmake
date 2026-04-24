option(CSL_STRICT_WARNINGS "[${PROJECT_NAME}] strict warnings" OFF)

include(CMakeParseArguments)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/details/print_aligned.cmake)

### Components - Targets - csl_lib::<name>
function(csl_add)

    set(options)
    set(oneValueArgs COMPONENT)
    set(multiValueArgs)
    cmake_parse_arguments(csl_add
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED csl_add_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_add] error: UNPARSED_ARGUMENTS = [${csl_add_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED csl_add_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_add] error: KEYWORDS_MISSING_VALUES = [${csl_add_KEYWORDS_MISSING_VALUES}]")
    endif()

    message(STATUS "[${PROJECT_NAME}::${csl_add_COMPONENT}]")

    add_library(${PROJECT_NAME}_${csl_add_COMPONENT} INTERFACE)
    add_library(${PROJECT_NAME}::${csl_add_COMPONENT} ALIAS ${PROJECT_NAME}_${csl_add_COMPONENT})

    target_compile_options(csl_${csl_add_COMPONENT} INTERFACE

        $<$<BOOL:${CSL_STRICT_WARNINGS}>:-Werror >
        # -Wdouble-promotion
        # -Wpedantic -pedantic-errors

        -Wall -Wextra
        -Wshadow
    #   -Wno-pointer-sign
        -Wunused-variable
        -Woverloaded-virtual
        -Wno-unknown-pragmas
        -Wno-unused-parameter
        -Wnull-dereference
        -Wconversion
    )

    # check if component has a custom cmake file to inject
    set(maybe_${csl_add_COMPONENT}_custom_cmake_path "${PROJECT_SOURCE_DIR}/cmake/custom/${csl_add_COMPONENT}.cmake")
    if (EXISTS "${maybe_${csl_add_COMPONENT}_custom_cmake_path}")
        message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} : custom cmake rules detected in [ ${maybe_${csl_add_COMPONENT}_custom_cmake_path} ]")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${maybe_${csl_add_COMPONENT}_custom_cmake_path}")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    target_include_directories(csl_${csl_add_COMPONENT} INTERFACE
        ${PROJECT_SOURCE_DIR}/includes/${csl_add_COMPONENT}
    )

    # test
    option(CSL_TEST_${csl_add_COMPONENT} "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (tests)" ${CSL_TEST_ALL})
    if (CSL_TEST_${csl_add_COMPONENT})
        enable_testing()
        message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (tests)")

        list(APPEND CMAKE_MESSAGE_INDENT "   ")
        add_subdirectory(${PROJECT_SOURCE_DIR}/tests/${csl_add_COMPONENT})
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    # example
    option(CSL_EXAMPLE_${csl_add_COMPONENT} "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (example)" ${CSL_EXAMPLE_ALL})
    if (CSL_EXAMPLE_${csl_add_COMPONENT})
        if (NOT EXISTS ${PROJECT_SOURCE_DIR}/examples/${csl_add_COMPONENT}/CMakeLists.txt)
            # message(AUTHOR_WARNING "[${PROJECT_NAME}] : csl::${csl_add_COMPONENT} has no examples available, skipping ...")
        else()
            message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_COMPONENT} (example)")

            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${PROJECT_SOURCE_DIR}/examples/${csl_add_COMPONENT})
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        endif()
    endif()

    # install
    # TODO

    # doc
    # TODO

    # benchmark
    # TODO

endfunction()
