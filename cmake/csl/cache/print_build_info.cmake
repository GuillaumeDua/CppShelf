include_guard(GLOBAL)

include(csl/print_aligned)

function(csl_print_build_info)

    get_property(_is_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (_is_GENERATOR_IS_MULTI_CONFIG)
        csl_print_aligned(STATUS CMAKE_CONFIGURATION_TYPES)
        csl_print_aligned(STATUS CMAKE_DEFAULT_BUILD_TYPE)
    else()
        csl_print_aligned(STATUS CMAKE_BUILD_TYPE)
    endif()

    csl_print_aligned(STATUS CMAKE_CURRENT_SOURCE_DIR)
    csl_print_aligned(STATUS CMAKE_CURRENT_BINARY_DIR)
    csl_print_aligned(STATUS CMAKE_INSTALL_PREFIX)
    csl_print_aligned(STATUS CMAKE_GENERATOR)
    csl_print_aligned(STATUS CMAKE_SYSTEM_NAME)
    csl_print_aligned(STATUS CMAKE_SYSTEM_PROCESSOR)
    csl_print_aligned(STATUS CMAKE_CXX_COMPILER)
    csl_print_aligned(STATUS CMAKE_CXX_COMPILER_ID)
    csl_print_aligned(STATUS CMAKE_CXX_COMPILER_VERSION)
    csl_print_aligned(STATUS CMAKE_CXX_STANDARD)

    if (CMAKE_EXPORT_COMPILE_COMMANDS)
        set(compile_commands_json "${CMAKE_BINARY_DIR}/compile_commands.json")
        csl_print_aligned(STATUS compile_commands_json)
    endif()

endfunction()
