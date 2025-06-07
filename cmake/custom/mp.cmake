# opt-in: CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT
option(CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT "[${CMAKE_PROJECT_NAME}] csl::${component_name}: drop-in replace for std::tuple" OFF)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT set to [${CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT}]")
if (${CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT})
    target_compile_definitions(csl_${component_name} INTERFACE CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT)
endif()

# opt-in: CSL_MP_TUPLE__IMPLICIT_CONVERSION
option(CSL_MP_TUPLE__IMPLICIT_CONVERSION "[${CMAKE_PROJECT_NAME}] csl::${component_name}: allow implicit conversions" ${CSL_MP_TUPLE__STD_DROP_IN_REPLACEMENT})
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name}: CSL_MP_TUPLE__IMPLICIT_CONVERSION set to [${CSL_MP_TUPLE__IMPLICIT_CONVERSION}]")
if (${CSL_MP_TUPLE__IMPLICIT_CONVERSION})
    target_compile_definitions(csl_${component_name} INTERFACE CSL_MP_TUPLE__IMPLICIT_CONVERSION)
endif()
