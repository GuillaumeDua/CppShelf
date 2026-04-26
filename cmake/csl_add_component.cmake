option(CSL_STRICT_WARNINGS "[${PROJECT_NAME}] strict warnings" OFF)

include(CMakeParseArguments)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/print_aligned.cmake)

### Components - Targets - csl_lib::<name>
function(csl_add_component)

    set(options)
    set(oneValueArgs COMPONENT)
    set(multiValueArgs)
    cmake_parse_arguments(csl_add_component
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if (DEFINED csl_add_component_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_add_component] error: UNPARSED_ARGUMENTS = [${csl_add_component_UNPARSED_ARGUMENTS}]")
    endif()
    if (DEFINED csl_add_component_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_add_component] error: KEYWORDS_MISSING_VALUES = [${csl_add_component_KEYWORDS_MISSING_VALUES}]")
    endif()

    message(STATUS "[${PROJECT_NAME}::${csl_add_component_COMPONENT}]")

    add_library(${PROJECT_NAME}_${csl_add_component_COMPONENT} INTERFACE)
    add_library(${PROJECT_NAME}::${csl_add_component_COMPONENT} ALIAS ${PROJECT_NAME}_${csl_add_component_COMPONENT})

    target_compile_options(csl_${csl_add_component_COMPONENT} INTERFACE

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

    target_include_directories(csl_${csl_add_component_COMPONENT} INTERFACE
        ${PROJECT_SOURCE_DIR}/libs/${csl_add_component_COMPONENT}/includes/${csl_add_component_COMPONENT}
    )

    set(component_name ${csl_add_component_COMPONENT})
    set(_csl_options_cmake "${PROJECT_SOURCE_DIR}/libs/${csl_add_component_COMPONENT}/cmake/options.cmake")
    if (EXISTS "${_csl_options_cmake}")
        message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_component_COMPONENT} : loading options from ${_csl_options_cmake}")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${_csl_options_cmake}")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    # test
    if (CSL_TEST_${csl_add_component_COMPONENT})
        set(_csl_tests_dir "${PROJECT_SOURCE_DIR}/libs/${csl_add_component_COMPONENT}/tests")
        if (EXISTS "${_csl_tests_dir}/CMakeLists.txt")
            enable_testing()
            message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_component_COMPONENT} (tests)")
            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${_csl_tests_dir})
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        endif()
    endif()

    # example
    if (CSL_EXAMPLE_${csl_add_component_COMPONENT})
        set(_csl_examples_dir "${PROJECT_SOURCE_DIR}/libs/${csl_add_component_COMPONENT}/examples")
        if (EXISTS "${_csl_examples_dir}/CMakeLists.txt")
            message(VERBOSE "[${PROJECT_NAME}] csl::${csl_add_component_COMPONENT} (example)")
            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${_csl_examples_dir})
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
