include_guard(GLOBAL)

# Creates a compile_commands.json symlink in the parent of BINARY_DIR,
# so tools like clangd (pointed at the shared build root) always picks up whichever preset was last configured.
# No-op when called from a subdirectory context (PROJECT_IS_TOP_LEVEL is false).
#
# Usage:
#   csl_symlink_compile_commands([BINARY_DIR <dir>] [SOURCE_DIR <dir>])
#
#   BINARY_DIR   Directory containing compile_commands.json. Defaults to CMAKE_BINARY_DIR.
#   SOURCE_DIR   Used only for the flat-build warning. Defaults to CMAKE_SOURCE_DIR.
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

    cmake_path(GET arg_BINARY_DIR PARENT_PATH _dest_dir)

    if (_dest_dir STREQUAL arg_SOURCE_DIR)
        message(WARNING "[csl_symlink_compile_commands] build dir is a direct child of the source root (no preset subdir) - symlink will land in the source tree")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${arg_BINARY_DIR}/compile_commands.json
            ${_dest_dir}/compile_commands.json
    )
endfunction()
