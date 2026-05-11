# Generates ag_generated.hpp into the build directory.
# Produces partial specializations for:
#   - make_to_tuple<N, T>        (consteval, returns std::type_identity<std::tuple<...>>)
#   - to_tuple_view_impl<N, T>   (constexpr, returns make_tuple_view<...>(...))
# for N in [1 .. CSL_AG__MAX_FIELDS_SUPPORTED_COUNT].
#
# The file is wrapped in namespace csl::ag::details::generated so it can be
# included directly from ag.hpp via __has_include(<csl/ag_generated.hpp>).

function(ag_generate_cpp_code)

    cmake_parse_arguments(arg "" "OUTPUT_FILE" "" ${ARGN})

    if (NOT DEFINED arg_OUTPUT_FILE)
        message(FATAL_ERROR "[ag_generate_cpp_code] OUTPUT_FILE argument is required")
    endif()

    set(N ${CSL_AG__MAX_FIELDS_SUPPORTED_COUNT})

    set(content "")
    string(APPEND content "#pragma once\n")
    string(APPEND content "// GENERATED CONTENT, DO NOT EDIT MANUALLY !\n")
    string(APPEND content "// Generated with CSL_AG__MAX_FIELDS_SUPPORTED_COUNT = ${N}\n")
    string(APPEND content "namespace csl::ag::configuration {\n")
    string(APPEND content "    constexpr static auto max_supported_fields_count = std::size_t{${N}};\n")
    string(APPEND content "}\n")
    string(APPEND content "namespace csl::ag::details::generated {\n")

    # --- make_to_tuple<N, T> ---
    string(APPEND content "#pragma region make_to_tuple<N,T>\n")
    set(bindings "v0")
    set(dtypes "decltype(v0)")
    foreach(ID RANGE 1 ${N})
        string(APPEND content
            "template <std::size_t N> requires (N == ${ID}) // NOLINT\n"
            " [[nodiscard]] consteval auto make_to_tuple(concepts::aggregate auto && value) noexcept {\n"
            "\tauto && [ ${bindings} ] = value;\n"
            "\treturn std::type_identity<std::tuple<${dtypes}>>{};\n"
            "}\n"
        )
        string(APPEND bindings ",v${ID}")
        string(APPEND dtypes ",decltype(v${ID})")
    endforeach()
    string(APPEND content "#pragma endregion\n")

    # --- to_tuple_view_impl<N, T> ---
    string(APPEND content "#pragma region to_tuple_view_impl<N,T>\n")
    set(bindings "v0")
    set(fwds "csl_fwd(v0)")
    foreach(ID RANGE 1 ${N})
        string(APPEND content
            "template <std::size_t N> requires (N == ${ID}) // NOLINT\n"
            " [[nodiscard]] constexpr auto to_tuple_view_impl(concepts::aggregate auto && value) noexcept {\n"
            "\tauto && [ ${bindings} ] = value;\n"
            "\treturn make_tuple_view<decltype(value)>( ${fwds} );\n"
            "}\n"
        )
        string(APPEND bindings ",v${ID}")
        string(APPEND fwds ",csl_fwd(v${ID})")
    endforeach()
    string(APPEND content "#pragma endregion\n")

    string(APPEND content "// END OF GENERATED CONTENT\n")
    string(APPEND content "} // namespace csl::ag::details::generated\n")

    # Only write if content changed (avoids unnecessary rebuilds)
    if (EXISTS "${arg_OUTPUT_FILE}")
        file(READ "${arg_OUTPUT_FILE}" existing)
        if ("${existing}" STREQUAL "${content}")
            return()
        endif()
    endif()

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/csl")
    file(WRITE "${arg_OUTPUT_FILE}" "${content}")
    message(VERBOSE "[ag_generate_cpp_code] Wrote ${arg_OUTPUT_FILE}")

endfunction()
