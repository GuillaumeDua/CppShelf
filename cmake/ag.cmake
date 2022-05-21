set(csl_ag_hpp ${PROJECT_SOURCE_DIR}/includes/ag/csl/ag.hpp)
if (NOT EXISTS ${csl_ag_hpp})
    message(FATAL "[${CMAKE_PROJECT_NAME}] : csl::${component_name} : missing file ${csl_ag_hpp}")
endif()

# Generates "partial" specializations for `as_tuple_impl<N, T>`,
#                                     and `element<N, T>`
# According to the following template :
#
# template <std::size_t N> requires (N == /* 0..AG_MAX_FIELDS_COUNT */)
# auto as_tuple_impl(Aggregate auto && value) {
#	auto & [ v[0..AG_MAX_FIELDS_COUNT]... ] = value;
#	return std::tie(value);
# }

# Handle options ...
## AG_MAX_FIELDS_COUNT_OPTION
set(AG_MAX_FIELDS_COUNT_OPTION "128" CACHE STRING "csl::ag : max fields count for aggregate to reflect")
message(STATUS "[${CMAKE_PROJECT_NAME}] : csl::${component_name} : AG_MAX_FIELDS_COUNT_OPTION set to ${AG_MAX_FIELDS_COUNT_OPTION}")
if (NOT AG_MAX_FIELDS_COUNT_OPTION MATCHES "^[0-9]+$")
    message(FATAL "[${CMAKE_PROJECT_NAME}] : csl::${component_name} : AG_MAX_FIELDS_COUNT_OPTION is not a valid number")
endif()
set(AG_MAX_FIELDS_COUNT ${AG_MAX_FIELDS_COUNT_OPTION})

# Generates specialization as a file ...
set(ag_as_tuple_impl_specialization_filepath ${PROJECT_SOURCE_DIR}/build/generated/ag_as_tuple_impl_specialization.hpp)
file(WRITE
    ${ag_as_tuple_impl_specialization_filepath}
    ""
)

## Generates `CSL_AG_UNFOLD_IDENTITIES_WITH_` (expansions macros) ...
file(APPEND
    ${ag_as_tuple_impl_specialization_filepath}
    "#pragma region CSL_AG_UNFOLD_IDENTITIES_WITH_N\n
    #define CSL_AG_UNFOLD_IDENTITIES_WITH_1(F) F(v0) // NOLINT\n"
)
foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

    math(EXPR ID_MINUS_ONE "${ID}-1")

    if (NOT ${ID} EQUAL 1)
    file(APPEND
        ${ag_as_tuple_impl_specialization_filepath}
        "#define CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(F) F(v${ID_MINUS_ONE}), CSL_AG_UNFOLD_IDENTITIES_WITH_${ID_MINUS_ONE}(F) // NOLINT\n"
    )
    endif()
endforeach()
file(APPEND
    ${ag_as_tuple_impl_specialization_filepath}
    "#pragma endregion\n"
)

## Generates `as_tuple_impl` ...
file(APPEND
    ${ag_as_tuple_impl_specialization_filepath}
    "#pragma region as_tuple_impl
    #define IDS(EXPR) EXPR\n"
)
foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

    file(APPEND
        ${ag_as_tuple_impl_specialization_filepath}
        "template <std::size_t N> requires (N == ${ID}) // NOLINT\n \
constexpr auto as_tuple_impl(concepts\:\:aggregate auto && value) {
\tauto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(IDS) ] = value;
\treturn fwd_tie<decltype(value)>(CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(IDS));
}\n"
    )
endforeach()
file(APPEND
    ${ag_as_tuple_impl_specialization_filepath}
    "#undef IDS
    #pragma endregion\n"
)

## Generates `element<N, T>` specializations ...

file(APPEND
    ${ag_as_tuple_impl_specialization_filepath}
    "#pragma region element<N, T>
    #define IDS(EXPR) EXPR
    #define DECLTYPE_IDS(EXPR) decltype(EXPR)\n"
)
foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

    file(APPEND
        ${ag_as_tuple_impl_specialization_filepath}
        "template <std::size_t N, concepts::aggregate T>\n
        requires (fields_count<T> == ${ID})\n
        struct element<N, T> : decltype([]() -> decltype(auto) {
            auto && [ CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(IDS) ] = declval<T&>();
            // return std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(DECLTYPE_IDS)>(CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(IDS));
            return std::tuple_element<
                N,
                std::tuple<CSL_AG_UNFOLD_IDENTITIES_WITH_${ID}(DECLTYPE_IDS)>
            >{};
        }()){};
    "
    )
endforeach()
file(APPEND
    ${ag_as_tuple_impl_specialization_filepath}
    "#undef IDS\n \
    #undef DECLTYPE_IDS\n \
    #pragma endregion\n"
)

# injects into ag/csl/ag.hpp

file(READ ${ag_as_tuple_impl_specialization_filepath} csl_ag_hpp_to_inject)
file(READ ${csl_ag_hpp} csl_ag_hpp_file_content)
string(REGEX REPLACE
    "(\\/\\/ GENERATED CONTENT, DO NOT EDIT MANUALLY !\n)(.*)(\\/\\/ END OF GENERATED CONTENT)"
    "\\1${csl_ag_hpp_to_inject}\\3"
    csl_ag_hpp_file_content_with_injection  # OUTPUT
    "${csl_ag_hpp_file_content}"            # INPUT
)

if ("${csl_ag_hpp_file_content_with_injection}" STREQUAL "${csl_ag_hpp_file_content}")
    message(AUTHOR_WARNING "[${CMAKE_PROJECT_NAME}] : csl::${component_name} : nothing was injected")
endif()

FILE(WRITE ${csl_ag_hpp} "${csl_ag_hpp_file_content_with_injection}")