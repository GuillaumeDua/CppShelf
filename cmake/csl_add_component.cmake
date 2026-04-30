include_guard(GLOBAL)

include(CMakeParseArguments)
include(csl/print_aligned)

### Components - Targets - ${PROJECT_NAME}::<name>
function(csl_add_component)

    set(options)
    set(oneValueArgs NAME)
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

    set(csl_add_component_NAME ${csl_add_component_NAME} PARENT_SCOPE)

    message(STATUS "[${PROJECT_NAME}::${csl_add_component_NAME}]")

    add_library(${PROJECT_NAME}_${csl_add_component_NAME} INTERFACE)
    add_library(${PROJECT_NAME}::${csl_add_component_NAME} ALIAS ${PROJECT_NAME}_${csl_add_component_NAME})

    target_include_directories(csl_${csl_add_component_NAME} INTERFACE
        ${PROJECT_SOURCE_DIR}/libs/${csl_add_component_NAME}/includes/${csl_add_component_NAME}
    )

    set(_csl_options_cmake "${PROJECT_SOURCE_DIR}/libs/${csl_add_component_NAME}/cmake/options.cmake")
    if (EXISTS "${_csl_options_cmake}")
        message(VERBOSE "[${PROJECT_NAME}] ${PROJECT_NAME}::${csl_add_component_NAME} : loading options from ${_csl_options_cmake}")
        list(APPEND CMAKE_MESSAGE_INDENT "  ")
        include("${_csl_options_cmake}")
        list(POP_BACK CMAKE_MESSAGE_INDENT)
    endif()

    # test
    if (CSL_TEST_${csl_add_component_NAME})
        set(_csl_tests_dir "${PROJECT_SOURCE_DIR}/libs/${csl_add_component_NAME}/tests")
        if (EXISTS "${_csl_tests_dir}/CMakeLists.txt")
            enable_testing()
            message(VERBOSE "[${PROJECT_NAME}] ${PROJECT_NAME}::${csl_add_component_NAME} (tests)")
            list(APPEND CMAKE_MESSAGE_INDENT "   ")
            add_subdirectory(${_csl_tests_dir})
            list(POP_BACK CMAKE_MESSAGE_INDENT)
        endif()
    endif()

    # example
    if (CSL_EXAMPLE_${csl_add_component_NAME})
        set(_csl_examples_dir "${PROJECT_SOURCE_DIR}/libs/${csl_add_component_NAME}/examples")
        if (EXISTS "${_csl_examples_dir}/CMakeLists.txt")
            message(VERBOSE "[${PROJECT_NAME}] ${PROJECT_NAME}::${csl_add_component_NAME} (example)")
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
