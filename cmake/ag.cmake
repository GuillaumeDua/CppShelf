
# Generate "partial" specializations for `as_tuple_impl<N, T>`
# According to the following template :
#
# template <std::size_t N> requires (N == /* 0..AG_MAX_FIELDS_COUNT */)
# auto as_tuple_impl(Aggregate auto && value) {
#	auto & [ v[0..AG_MAX_FIELDS_COUNT]... ] = value;
#	return std::tie(value);
# }

if (NOT AG_MAX_FIELDS_COUNT)
	set(AG_MAX_FIELDS_COUNT 128)
endif()
message(STATUS "[${CMAKE_PROJECT_NAME}] : csl::${component_name} : AG_MAX_FIELDS_COUNT set to ${AG_MAX_FIELDS_COUNT}")

# Create original file
set(ag_as_tuple_impl_specialization_filepath ${PROJECT_SOURCE_DIR}/build/generated/ag_as_tuple_impl_specialization.hpp)
file(WRITE
    ${ag_as_tuple_impl_specialization_filepath}
    "// GENERATED FILE, DO NOT EDIT MANUALLY !\n\n\n"
)

set(IDENTITY_LIST "i0")
foreach (ID RANGE 0 ${AG_MAX_FIELDS_COUNT})

    if (NOT ${ID} EQUAL 0)
        set(IDENTITY_LIST "${IDENTITY_LIST}, i${ID}")
    endif()

    file(APPEND
        ${ag_as_tuple_impl_specialization_filepath}
        "template <std::size_t N> requires (N == ${ID}) \
auto as_tuple_impl(Aggregate auto && value) {
\tauto & [ ${IDENTITY_LIST} ] = value;
\treturn std::tie(value);
}\n"
    )
endforeach()