#pragma once
// cpp shelf library : aggregates utility - formatting: std::format support (opt-in feature header)
// under MIT License - Copyright (c) 2021 Guillaume Dua "Guss"
// https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE

// Provides std::formatter specializations for aggregates and formatted views (see operator|) via csl::ag::io.

#if not defined(CSL_AG__INCLUDED)
#   include <csl/ag.hpp>
#endif

#if not __has_include(<format>)
#   error "csl/ag/formatting/backend/std_format.hpp : missing <format> (std::format requires C++20)."
#endif

#include <format>

namespace csl::ag::io::details {
    template <>
    struct format_error_type<std::formatter> : std::type_identity<std::format_error>{};
}

/// \brief std::formatter for plain aggregate: default (braced, flat) output.
template <csl::ag::concepts::aggregate T, typename Char>
requires (not csl::ag::io::details::concepts::decorator<T>)
    and  (not std::ranges::range<T>)
struct std::formatter<T, Char>
    : public csl::ag::io::details::ag_formatter_base<
        std::formatter, T, Char
    >
{};

namespace csl::ag::io::details::type_traits {

    /// \brief used upstream: not formatter detection
    ///       Recurses through structured_bindable elements std::formattable is consulted at leaves only.
    ///       NOTE: this library's view machinery formats those itself, so the STL's P2286 tuple-like/range formatters must NOT be required (e.g. absent in libstdc++ 13). 
    template <typename T, typename Char>
    struct is_std_formattable : std::bool_constant<std::formattable<T, Char>>{};
    template <csl::ag::concepts::structured_bindable T, typename Char>
    struct is_std_formattable<T, Char> {
        constexpr static auto value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and is_std_formattable<csl::ag::tuplelike::element_t<indexes, T>, Char>::value);
        }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});
    };
    template <typename T, typename Char>
    constexpr inline static auto is_std_formattable_v = is_std_formattable<T, Char>::value;
}
namespace csl::ag::io::details::concepts {

    /// \brief used upstream: not formatter detection
    template <typename T, typename Char>
    concept std_formattable = type_traits::is_std_formattable_v<T, Char>;
}

// NOLINTBEGIN(cert-dcl58-cpp) - std::formatter is a CPO
// std::formatter<formatted_view_t>: composite structured_bindable T
template <
    csl::ag::concepts::structured_bindable T,
    typename Char
>
requires (not csl::ag::io::details::concepts::decorator<T>)
and csl::ag::io::details::concepts::std_formattable<T, Char>
struct std::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::io::details::ag_formatter_base<
        std::formatter, T, Char
    >
{};

// std::formatter<formatted_view_t>: non-structured-bindable leaf T
template <
    typename T,
    typename Char
>
requires (not csl::ag::concepts::structured_bindable<T>)
struct std::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::io::details::ag_formatter_base_leaf<
        std::formatter, T, Char
    >
{};
// NOLINTEND(cert-dcl58-cpp)
