
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
