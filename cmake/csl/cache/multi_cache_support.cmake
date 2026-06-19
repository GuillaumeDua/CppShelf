include_guard(GLOBAL)

# Creates a compile_commands.json symlink at the top-level subdirectory of SOURCE_DIR that contains BINARY_DIR
# (e.g. "build", however many preset/kit/build-type levels BINARY_DIR is nested under it),
# so tools like `clangd` (pointed at that shared build root) always pick up whichever configuration was last run.
#
# No-op:
# - when called from a subdirectory context (PROJECT_IS_TOP_LEVEL is false),
# - or when BINARY_DIR is itself that top-level directory (flat build, nothing to symlink to).
#
# Usage:
#   csl_symlink_compile_commands([BINARY_DIR <dir>] [SOURCE_DIR <dir>])
#
#   BINARY_DIR   Directory containing compile_commands.json. Defaults to CMAKE_BINARY_DIR.
#   SOURCE_DIR   Root directory BINARY_DIR is expected to be nested under. Defaults to CMAKE_SOURCE_DIR.
function(csl_symlink_compile_commands)

    if (NOT PROJECT_IS_TOP_LEVEL)
        message(WARNING "[csl_symlink_compile_commands] PROJECT_IS_TOP_LEVEL=[${PROJECT_IS_TOP_LEVEL}], aborting (no-op)")
        return()
    endif()

    cmake_parse_arguments(PARSE_ARGV 0 arg "" "BINARY_DIR;SOURCE_DIR" "")

    if (NOT DEFINED arg_BINARY_DIR)
        set(arg_BINARY_DIR "${CMAKE_BINARY_DIR}")
    endif()

    if (NOT DEFINED arg_SOURCE_DIR)
        set(arg_SOURCE_DIR "${CMAKE_SOURCE_DIR}")
    endif()

    cmake_path(RELATIVE_PATH arg_BINARY_DIR BASE_DIRECTORY "${arg_SOURCE_DIR}" OUTPUT_VARIABLE _rel_binary_dir)
    string(REPLACE "/" ";" _rel_binary_dir_parts "${_rel_binary_dir}")
    list(GET _rel_binary_dir_parts 0 _root_dir_name)
    set(_dest_dir "${arg_SOURCE_DIR}/${_root_dir_name}")

    if (_dest_dir STREQUAL arg_BINARY_DIR)
        message(WARNING "[csl_symlink_compile_commands] build dir is a direct child of the source root (no preset/kit subdir) - compile_commands.json is already there, nothing to symlink")
        return()
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${arg_BINARY_DIR}/compile_commands.json
            ${_dest_dir}/compile_commands.json
    )
endfunction()
