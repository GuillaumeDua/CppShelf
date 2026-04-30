# opt-in: CSL_MP_TUPLE__IMPLICIT_CONVERSION (NONE, SAFE, UNSAFE)
set(CSL_MP_TUPLE__IMPLICIT_CONVERSION "NONE"
    CACHE STRING
    "[${CMAKE_PROJECT_NAME}::${csl_add_component_NAME}]: implicit conversion mode (NONE, SAFE, UNSAFE)"
)
csl_print_aligned(STATUS CSL_MP_TUPLE__IMPLICIT_CONVERSION)

# NOTE: UNSAFE is the drop-in replacement for <tuple>, since standard hearders are injected using -isystem
set_property(CACHE CSL_MP_TUPLE__IMPLICIT_CONVERSION PROPERTY STRINGS
    NONE
    SAFE
    UNSAFE
)

# Warnings suppressed in tests per conversion mode
set(CSL_MP_TUPLE__IMPLICIT_CONVERSION_NONE_test_disabled_warnings   "")
set(CSL_MP_TUPLE__IMPLICIT_CONVERSION_SAFE_test_disabled_warnings   -Wno-conversion)
set(CSL_MP_TUPLE__IMPLICIT_CONVERSION_UNSAFE_test_disabled_warnings -Wno-conversion -Wno-sign-conversion)

csl_cache_entry_to_property_index(
    ID      CSL_MP_TUPLE__IMPLICIT_CONVERSION
    OUT_VAR CSL_MP_TUPLE__IMPLICIT_CONVERSION_value
)
csl_print_aligned(STATUS CSL_MP_TUPLE__IMPLICIT_CONVERSION_value)

target_compile_definitions(
    csl_${csl_add_component_NAME} INTERFACE
    CSL_MP_TUPLE__IMPLICIT_CONVERSION=${CSL_MP_TUPLE__IMPLICIT_CONVERSION_value}
)
