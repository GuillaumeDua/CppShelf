# opt-in: CSL_MP_TUPLE__IMPLICIT_CONVERSION (NONE, SAFE, UNSAFE)
set(CSL_MP_TUPLE__IMPLICIT_CONVERSION "NONE"
    CACHE STRING
    "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: implicit conversion mode (NONE, SAFE, UNSAFE)"
)
csl_print_aligned(STATUS CSL_MP_TUPLE__IMPLICIT_CONVERSION)

set(CSL_MP_TUPLE__IMPLICIT_CONVERSION__allowed_values
    NONE
    SAFE
    UNSAFE # Drop-in replacement for <tuple>, as -isystem
)

csl_cache_entry_to_int(
    ID      CSL_MP_TUPLE__IMPLICIT_CONVERSION
    OUT_VAR CSL_MP_TUPLE__IMPLICIT_CONVERSION_value
)

target_compile_definitions(
    csl_${csl_add_component_NAME} INTERFACE
    CSL_MP_TUPLE__IMPLICIT_CONVERSION=${CSL_MP_TUPLE__IMPLICIT_CONVERSION_value}
)
