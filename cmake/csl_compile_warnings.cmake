include_guard(GLOBAL)

option(CSL_STRICT_WARNINGS "[${PROJECT_NAME}] strict warnings" OFF)

add_library(csl_compile_warnings INTERFACE)
target_compile_options(csl_compile_warnings INTERFACE

    $<$<BOOL:${CSL_STRICT_WARNINGS}>:-Werror>
    -Wdouble-promotion
    # -Wpedantic -pedantic-errors

    -Wall -Wextra
    -Wshadow
    -Wunused-variable
    -Woverloaded-virtual
    -Wno-unknown-pragmas
    -Wno-unused-parameter
    -Wnull-dereference
    -Wconversion
    -Wold-style-cast
    -Wimplicit-fallthrough
    -Wzero-as-null-pointer-constant
    -Wundef
    -Wcast-align

    $<$<CXX_COMPILER_ID:GNU>:
        -Wlogical-op
        -Wduplicated-cond
        -Wduplicated-branches
    >
)
