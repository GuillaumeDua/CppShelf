include_guard(GLOBAL)

# csl_graphviz(
#   TARGET        <target-name>
#   OPTIONS_FILE  <path/to/CMakeGraphVizOptions.cmake>
#   [OUTPUT_DIR   <dir>]          # default: <source_dir>/docs
#   [FORMAT       svg|png|...]    # default: svg; skipped if dot is not found
# )
#
# Creates a custom target `csl_graphviz_<target>` that:
#   1. Copies OPTIONS_FILE into CMAKE_BINARY_DIR as CMakeGraphVizOptions.cmake
#   2. Runs cmake --graphviz to produce the overall <target>.dot
#   3. If the dot executable is found, renders it to <target>.<format>
function(csl_graphviz)

    set(options)
    set(one_value_args
        TARGET
        OPTIONS_FILE
        OUTPUT_DIR
        FORMAT
    )
    set(multi_value_args)
    cmake_parse_arguments(PARSE_ARGV 0 arg "${options}" "${one_value_args}" "${multi_value_args}")

    if (DEFINED arg_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "[csl_graphviz] UNPARSED_ARGUMENTS = [${arg_UNPARSED_ARGUMENTS}]")
    endif ()
    if (DEFINED arg_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "[csl_graphviz] KEYWORDS_MISSING_VALUES = [${arg_KEYWORDS_MISSING_VALUES}]")
    endif ()
    if (NOT DEFINED arg_TARGET)
        message(FATAL_ERROR "[csl_graphviz] TARGET argument is required")
    endif ()
    if (NOT TARGET ${arg_TARGET})
        message(FATAL_ERROR "[csl_graphviz] TARGET [${arg_TARGET}] is not a known CMake target")
    endif ()
    if (NOT DEFINED arg_OPTIONS_FILE)
        message(FATAL_ERROR "[csl_graphviz] OPTIONS_FILE argument is required")
    endif ()

    if (NOT DEFINED arg_OUTPUT_DIR)
        set(arg_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/doc")
    endif ()
    if (NOT DEFINED arg_FORMAT)
        set(arg_FORMAT "svg")
    endif ()

    # cmake --graphviz=<prefix> produces the overall graph at <prefix>
    set(_dot_prefix "${arg_OUTPUT_DIR}/${arg_TARGET}")
    set(_dot_all    "${_dot_prefix}.dot")
    set(_image_file "${_dot_prefix}.${arg_FORMAT}")

    find_program(_found_dot_exe NAMES dot)

    if (_found_dot_exe)
        set(_render_command
            COMMAND "${_found_dot_exe}" "-T${arg_FORMAT}" "${_dot_all}" -o "${_image_file}"
        )
        set(_comment "[csl_graphviz] ${arg_TARGET} -> ${_image_file}")
    else()
        message(WARNING "[csl_graphviz] dot executable not found — image rendering skipped")
        set(_render_command)
        set(_comment "[csl_graphviz] ${arg_TARGET} -> ${_dot_all}")
    endif ()

    set(_generate_commands
        COMMAND ${CMAKE_COMMAND} -E make_directory "${arg_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${arg_OPTIONS_FILE}"
            "${CMAKE_BINARY_DIR}/CMakeGraphVizOptions.cmake"
        COMMAND ${CMAKE_COMMAND} "--graphviz=${_dot_all}" "${CMAKE_BINARY_DIR}"
    )

    add_custom_target(csl_graphviz_${arg_TARGET}
        ${_generate_commands}
        ${_render_command}
        COMMENT "${_comment}"
        VERBATIM
    )
endfunction()
