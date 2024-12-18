# Generates "partial" specializations for `to_tuple_view_impl<N, T>`,
#                                     and `element<N, T>`
# According to the following template :
#
# template <std::size_t N> requires (N == /* 0..AG_MAX_FIELDS_COUNT */)
# auto to_tuple_view_impl(Aggregate auto && value) {
#	auto && [ v[0..AG_MAX_FIELDS_COUNT]... ] = value;
#	return make_tuple_view<decltype(value)>( v[0..AG_MAX_FIELDS_COUNT]... );
# }

function(ag_generate_cpp_code)

    set(csl_ag_hpp_path ${PROJECT_SOURCE_DIR}/includes/ag/csl/ag.hpp)
    if (NOT EXISTS ${csl_ag_hpp_path})
        message(FATAL "[${CMAKE_PROJECT_NAME}] csl::${component_name} : missing file ${csl_ag_hpp_path}")
    endif()

    set(AG_MAX_FIELDS_COUNT ${CSL_AG__MAX_FIELDS_SUPPORTED_COUNT})

    # Generates specialization as a file ...
    set(csl_ag__cmake_generated_code__filepath ${PROJECT_SOURCE_DIR}/build/generated/csl_ag.hpp)
    file(WRITE
        ${csl_ag__cmake_generated_code__filepath}
        ""
    )

    # TODO: one target per generated block

    file(APPEND
        ${csl_ag__cmake_generated_code__filepath}
        "// Generated code with CSL_AG__MAX_FIELDS_SUPPORTED_COUNT = ${CSL_AG__MAX_FIELDS_SUPPORTED_COUNT}\n"
    )

    ## Generates `make_to_tuple` ...
    set(identities "v0")
    set(identities_decltype "decltype(v0)")
    file(APPEND
        ${csl_ag__cmake_generated_code__filepath}
        "#pragma region make_to_tuple<N,T>\n"
    )
    foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

        file(APPEND
            ${csl_ag__cmake_generated_code__filepath}
            "template <std::size_t N> requires (N == ${ID}) // NOLINT\n \
    [[nodiscard]] consteval auto make_to_tuple(concepts\:\:aggregate auto && value) noexcept {
    \tauto && [ ${identities} ] = value;
    \treturn std::type_identity<std::tuple<${identities_decltype}>>{};
    }\n"
        )
        string(APPEND identities ",v${ID}")
        string(APPEND identities_decltype ",decltype(v${ID})")
    endforeach()
    file(APPEND
        ${csl_ag__cmake_generated_code__filepath}
        "#pragma endregion\n"
    )

    ## Generates `to_tuple_view_impl` ...
    set(identities "v0")
    set(identities_decltype "decltype(v0)")
    set(identities_fwd "csl_fwd(v0)")
    file(APPEND
        ${csl_ag__cmake_generated_code__filepath}
        "#pragma region to_tuple_view_impl<N,T>\n"
    )
    foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

        file(APPEND
            ${csl_ag__cmake_generated_code__filepath}
            "template <std::size_t N> requires (N == ${ID}) // NOLINT\n \
    [[nodiscard]] constexpr static auto to_tuple_view_impl(concepts\:\:aggregate auto && value) noexcept {
    \tauto && [ ${identities} ] = value;
    \treturn make_tuple_view<decltype(value)>( ${identities_fwd} );
    }\n"
        )
        string(APPEND identities ",v${ID}")
        string(APPEND identities_decltype ",decltype(v${ID})")
        string(APPEND identities_fwd ",csl_fwd(v${ID})")
    endforeach()
    file(APPEND
        ${csl_ag__cmake_generated_code__filepath}
        "#pragma endregion\n"
    )

    # Generates `element<N, T>` specializations ...
    # set(identities "v0")
    # set(identities_decltype "decltype(v0)")
    # set(identities_fwd "csl_fwd(v0)")
    # file(APPEND
    #     ${csl_ag__cmake_generated_code__filepath}
    #     "#pragma region element<N, T>\n"
    # )
    # foreach (ID RANGE 1 ${AG_MAX_FIELDS_COUNT})

    #     file(APPEND
    #         ${csl_ag__cmake_generated_code__filepath}
    #         "\ttemplate <std::size_t N, concepts::aggregate T> requires (fields_count<T> == ${ID}) // NOLINT
    #     struct element<N, T> : std::tuple_element<
    #         N,
    #         std::remove_cvref_t<decltype([]() constexpr {
    #             auto && [ ${identities} ] = declval<T>();
    #             return std::tuple<${identities_decltype}>{ ${identities_fwd} };
    #         }())>>
    #     {};\n"
    #     )
    #     string(APPEND identities ",v${ID}")
    #     string(APPEND identities_decltype ",decltype(v${ID})")
    #     string(APPEND identities_fwd ",csl_fwd(v${ID})")
    # endforeach()
    # file(APPEND
    #     ${csl_ag__cmake_generated_code__filepath}
    #     "#pragma endregion\n"
    # )

    # injects into ag/csl/ag.hpp

    file(READ ${csl_ag__cmake_generated_code__filepath} csl_ag_hpp_to_inject)
    file(READ ${csl_ag_hpp_path} csl_ag_hpp_file_content)
    string(REGEX REPLACE
        "(\\/\\/ GENERATED CONTENT, DO NOT EDIT MANUALLY !\n)(.*)(\\/\\/ END OF GENERATED CONTENT)"
        "\\1${csl_ag_hpp_to_inject}\\3"
        csl_ag_hpp_file_content_with_injection  # OUTPUT
        "${csl_ag_hpp_file_content}"            # INPUT
    )

    if ("${csl_ag_hpp_file_content_with_injection}" STREQUAL "${csl_ag_hpp_file_content}")
        message(STATUS "[${CMAKE_PROJECT_NAME}] csl::${component_name} : sources already up-to-date (no new content was injected)")
        return()
    endif()

    FILE(WRITE ${csl_ag_hpp_path} "${csl_ag_hpp_file_content_with_injection}")
endfunction()
