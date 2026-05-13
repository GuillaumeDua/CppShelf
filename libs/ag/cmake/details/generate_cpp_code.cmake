# Generates two headers into ${OUTPUT_DIR}/csl/:
#
#   ag_configuration.hpp
#     namespace csl::ag::configuration { constexpr static auto max_supported_fields_count = N; }
#
#   ag_generated.hpp
#     Partial specializations for:
#       - make_to_tuple<N, T>        (consteval, returns std::type_identity<std::tuple<...>>)
#       - to_tuple_view_impl<N, T>   (constexpr, returns make_tuple_view<...>(...))
#     for N in [1 .. CSL_AG__MAX_SUPPORTED_FIELDS_COUNT].
#
# Both files are consumed by ag.hpp via __has_include.
#
#   ag_configuration.hpp is included early (before fields_count).
#   ag_generated.hpp is included late (after make_tuple_view, inside namespace csl::ag::details::generated).

function(ag_generate_cpp_code)

    cmake_parse_arguments(arg "" "OUTPUT_DIR" "" ${ARGN})

    if (NOT DEFINED arg_OUTPUT_DIR)
        message(FATAL_ERROR "[ag_generate_cpp_code] OUTPUT_DIR argument is required")
    endif()

    set(N ${CSL_AG__MAX_SUPPORTED_FIELDS_COUNT})
    set(outdir "${arg_OUTPUT_DIR}/csl")
    file(MAKE_DIRECTORY "${outdir}")

    # --- ag_configuration.hpp ---
    set(config_file "${outdir}/ag_configuration.hpp")
    set(config_content "")
    string(APPEND config_content "#pragma once\n")
    string(APPEND config_content "// GENERATED CONTENT, DO NOT EDIT MANUALLY !\n")
    string(APPEND config_content "// Generated with CSL_AG__MAX_SUPPORTED_FIELDS_COUNT = ${N}\n")
    string(APPEND config_content "namespace csl::ag::configuration {\n")
    string(APPEND config_content "    constexpr static auto max_supported_fields_count = std::size_t{${N}};\n")
    string(APPEND config_content "}\n")

    if (EXISTS "${config_file}")
        file(READ "${config_file}" existing)
        if (NOT "${existing}" STREQUAL "${config_content}")
            file(WRITE "${config_file}" "${config_content}")
            message(VERBOSE "[ag_generate_cpp_code] Wrote ${config_file}")
        endif()
    else()
        file(WRITE "${config_file}" "${config_content}")
        message(VERBOSE "[ag_generate_cpp_code] Wrote ${config_file}")
    endif()

    # --- ag_generated.hpp ---
    set(impl_file "${outdir}/ag_generated.hpp")
    set(impl_content "")
    string(APPEND impl_content "#pragma once\n")
    string(APPEND impl_content "// GENERATED CONTENT, DO NOT EDIT MANUALLY !\n")
    string(APPEND impl_content "// Generated with CSL_AG__MAX_SUPPORTED_FIELDS_COUNT = ${N}\n")
    string(APPEND impl_content "namespace csl::ag::details::generated {\n")

    # --- make_to_tuple<N, T> ---
    string(APPEND impl_content "#pragma region make_to_tuple<N,T>\n")
    set(bindings "v0")
    set(dtypes "decltype(v0)")
    foreach(ID RANGE 1 ${N})
        string(APPEND impl_content
            "template <std::size_t N> requires (N == ${ID}) // NOLINT\n"
            " [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {\n"
            "\tauto && [ ${bindings} ] = value;\n"
            "\treturn std::type_identity<std::tuple<${dtypes}>>{};\n"
            "}\n"
        )
        string(APPEND bindings ",v${ID}")
        string(APPEND dtypes ",decltype(v${ID})")
    endforeach()
    string(APPEND impl_content "#pragma endregion\n")

    # --- to_tuple_view_impl<N, T> ---
    string(APPEND impl_content "#pragma region to_tuple_view_impl<N,T>\n")
    set(bindings "v0")
    set(fwds "csl_fwd(v0)")
    foreach(ID RANGE 1 ${N})
        string(APPEND impl_content
            "template <std::size_t N> requires (N == ${ID}) // NOLINT\n"
            " [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {\n"
            "\tauto && [ ${bindings} ] = value;\n"
            "\treturn make_tuple_view<decltype(value)>( ${fwds} );\n"
            "}\n"
        )
        string(APPEND bindings ",v${ID}")
        string(APPEND fwds ",csl_fwd(v${ID})")
    endforeach()
    string(APPEND impl_content "#pragma endregion\n")

    string(APPEND impl_content "// END OF GENERATED CONTENT\n")
    string(APPEND impl_content "} // namespace csl::ag::details::generated\n")

    if (EXISTS "${impl_file}")
        file(READ "${impl_file}" existing)
        if (NOT "${existing}" STREQUAL "${impl_content}")
            file(WRITE "${impl_file}" "${impl_content}")
            message(VERBOSE "[ag_generate_cpp_code] Wrote ${impl_file}")
        endif()
    else()
        file(WRITE "${impl_file}" "${impl_content}")
        message(VERBOSE "[ag_generate_cpp_code] Wrote ${impl_file}")
    endif()

endfunction()
