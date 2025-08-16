# opt-in: CSL_MP_TUPLE__IMPLICIT_CONVERSION
option(CSL_MP_TUPLE__IMPLICIT_CONVERSION FALSE
    "[${CMAKE_PROJECT_NAME}] csl::${component_name}: allow implicit conversions"
)
print_aligned(STATUS CSL_MP_TUPLE__IMPLICIT_CONVERSION)

if (CSL_MP_TUPLE__IMPLICIT_CONVERSION)
    set(CSL_MP_TUPLE__IMPLICIT_CONVERSION_value 1)
else()
    set(CSL_MP_TUPLE__IMPLICIT_CONVERSION_value 0)
endif()
target_compile_definitions(csl_${component_name} INTERFACE CSL_MP_TUPLE__IMPLICIT_CONVERSION=${CSL_MP_TUPLE__IMPLICIT_CONVERSION_value})
