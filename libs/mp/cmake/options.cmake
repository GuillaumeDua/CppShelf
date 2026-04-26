# opt-in: CSL_MP_TUPLE__IMPLICIT_CONVERSION (NONE, SAFE, UNSAFE)
set(CSL_MP_TUPLE__IMPLICIT_CONVERSION "NONE"
    CACHE STRING
    "[${CMAKE_PROJECT_NAME}::${component_name}]: implicit conversion mode (NONE, SAFE, UNSAFE)"
)
print_aligned(STATUS CSL_MP_TUPLE__IMPLICIT_CONVERSION)

set(CSL_MP_TUPLE__IMPLICIT_CONVERSION__allowed_values
    NONE
    SAFE
    UNSAFE # Drop-in replacement for <tuple>, as -isystem
)

cache_entry_to_int(
    ID      CSL_MP_TUPLE__IMPLICIT_CONVERSION
    OUT_VAR CSL_MP_TUPLE__IMPLICIT_CONVERSION_value
)

target_compile_definitions(
    csl_${component_name} INTERFACE
    CSL_MP_TUPLE__IMPLICIT_CONVERSION=${CSL_MP_TUPLE__IMPLICIT_CONVERSION_value}
)
