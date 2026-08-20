#pragma once

/// @file
/// @brief fmtlib support for structured bindables (opt-in feature header).
///
/// cpp shelf library : aggregates utility - formatting.
///
/// Provides the `csl::ag::formatting::fmt_formatter` opt-in base and `fmt::formatter` specializations
/// for formatted views (see operator|) via `csl::ag::formatting`.
///
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
///
/// @warning Include this header **consistently across the whole program** (ODR),
///          as with @c <fmt/ranges.h>.
///          The same contract applies to your own per-type opt-ins:
///          declare them next to the type they format.
///
/// @par Requirements
///      fmtlib (https://github.com/fmtlib/fmt) is provided by the consumer - this header does not fetch it.
///      fmtlib >= 11 is required for the @c :n specifier.
///
/// @par Design
///      - Per-type opt-in: derive @c fmt::formatter<T> from @c csl::ag::formatting::fmt_formatter<T>.
///        Nested structured_bindable fields and decorated views need no opt-in.
///      - Composable @c format_options selected via format-spec letters, or via the view-based @c operator| API - both are equivalent, and mixable:
///         @code
///         fmt::format("{}", value)                                  // default: braced, compact
///         fmt::format("{:n}", value)                                // flat, naked: no outer brackets or separator
///         fmt::format("{}", value | csl::ag::formatting::indented)  // multiline, depth-indented
///         fmt::format("{}", value | csl::ag::formatting::indexed)   // braced with [N] field indexes
///         fmt::format("{}", value | csl::ag::formatting::typenamed) // braced with TypeName: prefixes
///         fmt::format("{:ixt}", value)                              // spec letters compose too
///         @endcode
///      - Options propagate to nested structured_bindable fields (no_braces is outermost-only).
///      - Leaf values consistent with fmtlib: char => 'x', bool => true/false, string => "...".
///
/// @par Usage
///      @code
///      struct my_aggregate { int i; char c; };
///
///      template <>
///      struct fmt::formatter<my_aggregate> : csl::ag::formatting::fmt_formatter<my_aggregate>{};
///
///      using namespace csl::ag::formatting;
///      fmt::println("{}", my_aggregate{ 42, 'x' });
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

namespace csl::ag::formatting::details {

    /// \brief normalises fmt::formatter's 3-param signature (T, Char, SFINAE-Enable) to 2 params,
    ///        so it can be passed as a template template parameter to ag_formatter_base,
    ///        and is consistent with std::formatter<T, CharT>.
    ///        Reason: P0522 relaxed template template matching is not the default before Clang 19.
    template <typename T, typename Char = char>
    using fmt_formatter_adapter = fmt::formatter<T, Char>;

    template <>
    struct format_error_type<fmt_formatter_adapter> : std::type_identity<fmt::format_error>{};
}

namespace csl::ag::formatting::details::type_traits {

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
namespace csl::ag::formatting::details::concepts {

    /// \brief used upstream, not formatter detection
    template <typename T, typename Char>
    concept fmt_formattable = type_traits::is_fmt_formattable_v<T, Char>;
}

namespace csl::ag::formatting {

    /// \brief Opt-in base making T formattable directly by fmt::format.
    ///
    ///        \code
    ///        struct point { int x; int y; };
    ///
    ///        template <>
    ///        struct fmt::formatter<point> : csl::ag::formatting::fmt_formatter<point>{};
    ///        \endcode
    ///
    ///        Only the type formatted directly needs this:
    ///        nested structured_bindable fields, and any decorated view
    ///        (value | csl::ag::formatting::indented, see operator|), need no opt-in.
    ///        `value | csl::ag::formatting::format_options::none` reproduces the default output,
    ///        without any opt-in.
    ///
    /// \note  There is deliberately no blanket fmt::formatter for aggregates:
    ///        it would claim standard aggregates
    ///        (std::monostate, std::identity, std::plus<>, ...),
    ///        and collide with any other library specializing fmt::formatter on its own concept.
    ///        See csl::ag::formatting::std_formatter, which carries the same model -
    ///        plus, for std::formatter, a [namespace.std]/2 constraint.
    /// \note  Known limitation: the formatting machinery is char-based -
    ///        csl::ag::formatting::type_name and details::style::opening_bracket both yield std::string_view -
    ///        so Char is restricted to char.
    ///
    ///        WARNING: routes through ag_formatter_base,
    ///        not fmt's native tuple_join_view formatter:
    ///        per-element format-spec propagation
    ///        (fmt feature enabler: FMT_TUPLE_JOIN_SPECIFIERS) is not used/available here.
    ///        See simplification commit 000e7d2fb749bca03aadc80b22bad7e2f6d27f26
    template <
        csl::ag::concepts::structured_bindable T,
        typename Char = char
    >
    requires std::same_as<Char, char>
        and (not details::concepts::decorator<T>)
    struct fmt_formatter
        : public details::ag_formatter_base<
            details::fmt_formatter_adapter, T, Char
        >
    {
        static_assert(
            details::concepts::fmt_formattable<T, Char>,
            "[csl::ag::formatting::fmt_formatter] at least one of T's fields is not fmt-formattable."
        );
    };
}

#pragma region // formatter<formatted_view_t> (composable options)

// fmt::formatter<formatted_view_t>: composite structured_bindable T
template <
    csl::ag::concepts::structured_bindable T,
    typename Char
>
requires (not csl::ag::formatting::details::concepts::decorator<T>)
and csl::ag::formatting::details::concepts::fmt_formattable<T, Char>
class fmt::formatter<
    csl::ag::formatting::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::formatting::details::ag_formatter_base<
        csl::ag::formatting::details::fmt_formatter_adapter, T, Char
    >
{};

// fmt::formatter<formatted_view_t>: non-structured-bindable leaf T
template <
    typename T,
    typename Char
>
requires (not csl::ag::concepts::structured_bindable<T>)
class fmt::formatter<
    csl::ag::formatting::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::formatting::details::ag_formatter_base_leaf<
        csl::ag::formatting::details::fmt_formatter_adapter, T, Char
    >
{};
#pragma endregion
