set(csl_ag_hpp ${PROJECT_SOURCE_DIR}/includes/ag/csl/ag.hpp)
if (NOT EXISTS ${csl_ag_hpp})
    message(FATAL "[${CMAKE_PROJECT_NAME}] csl::${component_name} : missing file ${csl_ag_hpp}")
endif()

# Configuration ...
## CSL_AG_ENABLE_BITFIELDS_SUPPORT
option(CSL_AG_ENABLE_BITFIELDS_SUPPORT "csl::ag : enable bitfields support (slower compilation)" FALSE)
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} : CSL_AG_ENABLE_BITFIELDS_SUPPORT set to [${CSL_AG_ENABLE_BITFIELDS_SUPPORT}]")
if (CSL_AG_ENABLE_BITFIELDS_SUPPORT)
    add_definitions(-DCSL_AG_ENABLE_BITFIELDS_SUPPORT)
else()
    remove_definitions(-DCSL_AG_ENABLE_BITFIELDS_SUPPORT)
endif()

# Generates "partial" specializations for `as_tuple_view_impl<N, T>`,
#                                     and `element<N, T>`
# According to the following template :
#
# template <std::size_t N> requires (N == /* 0..AG_MAX_FIELDS_COUNT */)
# auto as_tuple_view_impl(Aggregate auto && value) {
#	auto && [ v[0..AG_MAX_FIELDS_COUNT]... ] = value;
#	return make_tuple_view<decltype(value)>( v[0..AG_MAX_FIELDS_COUNT]... );
# }

# Handle options ...
## CSL_AG_MAX_FIELDS_COUNT_OPTION
set(CSL_AG_MAX_FIELDS_COUNT_OPTION "128" CACHE STRING "csl::ag : max fields count for aggregate to reflect")
message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} : CSL_AG_MAX_FIELDS_COUNT_OPTION set to [${CSL_AG_MAX_FIELDS_COUNT_OPTION}]")
if (NOT CSL_AG_MAX_FIELDS_COUNT_OPTION MATCHES "^[0-9]+$")
    message(FATAL "[${CMAKE_PROJECT_NAME}] csl::${component_name} : CSL_AG_MAX_FIELDS_COUNT_OPTION is not a valid number")
endif()
set(AG_MAX_FIELDS_COUNT ${CSL_AG_MAX_FIELDS_COUNT_OPTION})

# Generates specialization as a file ...
set(ag_as_tuple_view_impl_specialization_filepath ${PROJECT_SOURCE_DIR}/build/generated/ag_as_tuple_view_impl_specialization.hpp)
file(WRITE
    ${ag_as_tuple_view_impl_specialization_filepath}
    ""
)

## Generates `as_tuple_view_impl` ...
set(identities "v0")
set(identities_decltype "decltype(v0)")
set(identities_fwd "csl_fwd(v0)")
file(APPEND
    ${ag_as_tuple_view_impl_specialization_filepath}
    "#pragma region as_tuple_view_impl<N,T>\n"
)
foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

    file(APPEND
        ${ag_as_tuple_view_impl_specialization_filepath}
        "template <std::size_t N> requires (N == ${ID}) // NOLINT\n \
[[nodiscard]] constexpr auto as_tuple_view_impl(concepts\:\:aggregate auto && value) {
\tauto && [ ${identities} ] = value;
\treturn make_tuple_view<decltype(value), ${identities_decltype}>( ${identities_fwd} );
}\n"
    )
    string(APPEND identities ",v${ID}")
    string(APPEND identities_decltype ",decltype(v${ID})")
    string(APPEND identities_fwd ",csl_fwd(v${ID})")
endforeach()
file(APPEND
    ${ag_as_tuple_view_impl_specialization_filepath}
    "#pragma endregion\n"
)

# Generates `element<N, T>` specializations ...
set(identities "v0")
set(identities_decltype "decltype(v0)")
set(identities_fwd "csl_fwd(v0)")
file(APPEND
    ${ag_as_tuple_view_impl_specialization_filepath}
    "#pragma region element<N, T>\n"
)
foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

    file(APPEND
        ${ag_as_tuple_view_impl_specialization_filepath}
        "\ttemplate <std::size_t N, concepts::aggregate T> requires (fields_count<T> == ${ID}) // NOLINT
    struct element<N, T> : std::tuple_element<
        N,
        std::remove_cvref_t<decltype([]() constexpr {
            auto && [ ${identities} ] = declval<T>();
            return std::tuple<${identities_decltype}>{ ${identities_fwd} };
        }())>>
    {};\n"
    )
    string(APPEND identities ",v${ID}")
    string(APPEND identities_decltype ",decltype(v${ID})")
    string(APPEND identities_fwd ",csl_fwd(v${ID})")
endforeach()
file(APPEND
    ${ag_as_tuple_view_impl_specialization_filepath}
    "#pragma endregion\n"
)

# injects into ag/csl/ag.hpp

file(READ ${ag_as_tuple_view_impl_specialization_filepath} csl_ag_hpp_to_inject)
file(READ ${csl_ag_hpp} csl_ag_hpp_file_content)
string(REGEX REPLACE
    "(\\/\\/ GENERATED CONTENT, DO NOT EDIT MANUALLY !\n)(.*)(\\/\\/ END OF GENERATED CONTENT)"
    "\\1${csl_ag_hpp_to_inject}\\3"
    csl_ag_hpp_file_content_with_injection  # OUTPUT
    "${csl_ag_hpp_file_content}"            # INPUT
)

if ("${csl_ag_hpp_file_content_with_injection}" STREQUAL "${csl_ag_hpp_file_content}")
    message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} : sources was already up-to-date (no new content was injected)")
endif()

FILE(WRITE ${csl_ag_hpp} "${csl_ag_hpp_file_content_with_injection}")
