#pragma once

/// @file
/// @brief fmtlib support for structured bindables (opt-in feature header).
///
/// cpp shelf library : aggregates utility - formatting.
///
/// Provides `fmt::formatter` specializations for aggregates and formatted views (see operator|) via `csl::ag::io`.
///
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
///
/// @warning Adds blanket specializations: like @c <fmt/ranges.h>, include this header
///          **consistently across the whole program** (ODR).
///
/// @par Requirements
///      fmtlib (https://github.com/fmtlib/fmt) is provided by the consumer - this header does not fetch it.
///      fmtlib >= 11 is required for the @c :n specifier.
///
/// @par Design
///      - Blanket @c fmt::formatter<T> for any aggregate @c T (non-range, non-decorator) whose fields are all formattable.
///      - Composable @c format_options selected via format-spec letters, or via the view-based @c operator| API - both are equivalent, and mixable:
///         @code
///         fmt::format("{}", value)                                       // default: braced, compact
///         fmt::format("{:n}", value)                                     // flat, naked: no outer brackets or separator
///         fmt::format("{}", value | csl::ag::io::indented)               // multiline, depth-indented
///         fmt::format("{}", value | csl::ag::io::indexed)                // braced with [N] field indexes
///         fmt::format("{}", value | csl::ag::io::typenamed)              // braced with TypeName: prefixes
///         fmt::format("{:ixt}", value)                                   // spec letters compose too
///         @endcode
///      - Options propagate to nested structured_bindable fields (no_braces is outermost-only).
///      - Leaf values consistent with fmtlib: char => 'x', bool => true/false, string => "...".
///
/// @par Usage
///      @code
///      using namespace csl::ag::io;
///      fmt::println("{}", my_aggregate);
///      @endcode

#if not defined(CSL_AG__INCLUDED)
#   if not __has_include(<csl/ag.hpp>)
#       error "[csl::ag] csl/ag/formatting/backend/fmt.hpp : missing <csl/ag.hpp>. This is an opt-in feature header: make <csl/ag.hpp> reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#   endif
#   include <csl/ag.hpp>
#endif

#if not __has_include(<fmt/format.h>)
#   error "[csl::ag] csl/ag/formatting/backend/fmt.hpp : missing <fmt/format.h>. Provide fmtlib in the build - https://github.com/fmtlib/fmt (on Compiler Explorer: Libraries -> add fmt)."
#endif

#include <fmt/ranges.h>
#include <fmt/compile.h>

namespace csl::ag::io::type_traits {

    // formatter_value_type
    template <typename T>
    struct formatter_value_type;
    template <typename T, typename Char>
    struct formatter_value_type<fmt::formatter<T, Char>> : std::type_identity<T>{};
    template <typename T>
    using formatter_value_type_t = formatter_value_type<T>::type;
}

namespace csl::ag::io::details {

    /// \brief normalises fmt::formatter's 3-param signature (T, Char, SFINAE-Enable) to 2 params,
    ///        so it can be passed as a template template parameter to ag_formatter_base.
    ///        Reason: P0522 relaxed template template matching is not the default before Clang 19.
    template <typename T, typename Char = char>
    using fmt_formatter = fmt::formatter<T, Char>;

    template <>
    struct format_error_type<fmt_formatter> : std::type_identity<fmt::format_error>{};
}

namespace csl::ag::io::details::type_traits {

    /// \brief used upstream, not formatter detection
    template <typename T, typename Char>
    struct is_fmt_formattable : fmt::is_formattable<T, Char>{};
    template <csl::ag::concepts::structured_bindable T, typename Char>
    struct is_fmt_formattable<T, Char> {
        constexpr static auto value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and is_fmt_formattable<csl::ag::tuplelike::element_t<indexes, T>, Char>::value);
        }(std::make_index_sequence<csl::ag::tuplelike::size_v<T>>{});
    };
    template <typename T, typename Char>
    constexpr inline static auto is_fmt_formattable_v = is_fmt_formattable<T, Char>::value;
}
namespace csl::ag::io::details::concepts {

    /// \brief used upstream, not formatter detection
    template <typename T, typename Char>
    concept fmt_formattable = type_traits::is_fmt_formattable_v<T, Char>;
}

/// \brief fmt::formatter for plain aggregate T - default (braced, flat) output.
///        WARNING: routes through ag_formatter_base, not fmt's native tuple_join_view formatter,
///        per-element format-spec propagation (fmt feature enabler: FMT_TUPLE_JOIN_SPECIFIERS) is not used/available here.
///        See simplification commit 000e7d2fb749bca03aadc80b22bad7e2f6d27f26
template <csl::ag::concepts::aggregate T, typename Char>
requires (not csl::ag::io::details::concepts::decorator<T>)
and (not std::ranges::range<T>)
class fmt::formatter<T, Char>
    : public csl::ag::io::details::ag_formatter_base<
        csl::ag::io::details::fmt_formatter, T, Char
    >
{};

#pragma region // formatter<formatted_view_t> (composable options)

namespace csl::ag::io::type_traits {
    template <typename T, typename Char>
    struct formatter_value_type<
        fmt::formatter<csl::ag::io::details::decorators::formatted_view_t<T>, Char>
    > : std::type_identity<T>{};
}

// fmt::formatter<formatted_view_t>: composite structured_bindable T
template <
    csl::ag::concepts::structured_bindable T,
    typename Char
>
requires (not csl::ag::io::details::concepts::decorator<T>)
and csl::ag::io::details::concepts::fmt_formattable<T, Char>
class fmt::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::io::details::ag_formatter_base<
        csl::ag::io::details::fmt_formatter, T, Char
    >
{};

// fmt::formatter<formatted_view_t>: non-structured-bindable leaf T
template <
    typename T,
    typename Char
>
requires (not csl::ag::concepts::structured_bindable<T>)
class fmt::formatter<
    csl::ag::io::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::io::details::ag_formatter_base_leaf<
        csl::ag::io::details::fmt_formatter, T, Char
    >
{};
#pragma endregion
