#pragma once

/// @file
/// @brief std::format support for structured bindables (opt-in feature header).
///
/// cpp shelf library : aggregates utility - formatting.
///
/// Provides the `csl::ag::formatting::std_formatter` opt-in base and `std::formatter` specializations
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
/// @par Design
///      - Per-type opt-in: derive @c std::formatter<T> from @c csl::ag::formatting::std_formatter<T>.
///        Nested structured_bindable fields and decorated views need no opt-in.
///      - Composable @c format_options selected via format-spec letters, or via the view-based @c operator| API - both are equivalent, and mixable:
///         @code
///         std::format("{}", value)                                  // default: braced, compact
///         std::format("{:n}", value)                                // flat, naked: no outer brackets or separator
///         std::format("{}", value | csl::ag::formatting::indented)  // multiline, depth-indented
///         std::format("{}", value | csl::ag::formatting::indexed)   // braced with [N] field indexes
///         std::format("{}", value | csl::ag::formatting::typenamed) // braced with TypeName: prefixes
///         std::format("{:ixt}", value)                              // spec letters compose too
///         @endcode
///      - Options propagate to nested structured_bindable fields (no_braces is outermost-only).
///      - Leaf values consistent with fmtlib: char => 'x', bool => true/false, string => "...".
///      - Tuple-like and range fields are formatted by this library's own machinery:
///        no dependency on the STL's P2286 formatters (e.g. absent in libstdc++ 13).
///
/// @par Usage
///      @code
///      struct my_aggregate { int i; char c; };
///
///      template <>
///      struct std::formatter<my_aggregate> : csl::ag::formatting::std_formatter<my_aggregate>{};
///
///      using namespace csl::ag::formatting;
///      std::println("{}", my_aggregate{ 42, 'x' });
///      @endcode

#if not defined(CSL_AG__INCLUDED)
#   if not __has_include(<csl/ag.hpp>)
#       error "[csl::ag] csl/ag/formatting/backend/std_format.hpp : missing <csl/ag.hpp>. This is an opt-in feature header: make <csl/ag.hpp> reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#   endif
#   include <csl/ag.hpp>
#endif

#if not __has_include(<format>)
#   error "csl/ag/formatting/backend/std_format.hpp : missing <format> (std::format requires C++20)."
#endif

#include <format>

namespace csl::ag::formatting::details {
    template <>
    struct format_error_type<std::formatter> : std::type_identity<std::format_error>{};
}

namespace csl::ag::formatting::details::type_traits {

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
namespace csl::ag::formatting::details::concepts {

    /// \brief used upstream: not formatter detection
    template <typename T, typename Char>
    concept std_formattable = type_traits::is_std_formattable_v<T, Char>;
}

namespace csl::ag::formatting {

    /// \brief Opt-in base making T formattable directly by std::format.
    ///
    ///        \code
    ///        struct point { int x; int y; };
    ///
    ///        template <>
    ///        struct std::formatter<point> : csl::ag::formatting::std_formatter<point>{};
    ///        \endcode
    ///
    ///        Only the type formatted directly needs this:
    ///        nested structured_bindable fields, and any decorated view
    ///        (value | csl::ag::formatting::indented, see operator|), need no opt-in.
    ///        `value | csl::ag::formatting::format_options::none` reproduces the default output,
    ///        without any opt-in.
    ///
    /// \note  There is deliberately no blanket std::formatter for aggregates:
    ///        [namespace.std]/2 requires an added declaration to depend on a program-defined
    ///        *type*, and a concept is not one.
    ///        A blanket would also claim standard aggregates
    ///        (std::monostate, std::identity, std::plus<>, ...),
    ///        and collide with any other library specializing std::formatter on its own concept.
    /// \note  Known limitation: the formatting machinery is char-based -
    ///        csl::ag::formatting::type_name and details::style::opening_bracket both yield
    ///        std::string_view - so Char is restricted to char.
    template <
        csl::ag::concepts::structured_bindable T,
        typename Char = char
    >
    requires std::same_as<Char, char>
        and (not details::concepts::decorator<T>)
    struct std_formatter
        : public details::ag_formatter_base<
            std::formatter, T, Char
        >
    {
        static_assert(
            details::concepts::std_formattable<T, Char>,
            "[csl::ag::formatting::std_formatter] at least one of T's fields is not std-formattable."
        );
    };
}

// NOLINTBEGIN(cert-dcl58-cpp) - std::formatter is a CPO
// std::formatter<formatted_view_t>: composite structured_bindable T
template <
    csl::ag::concepts::structured_bindable T,
    typename Char
>
requires (not csl::ag::formatting::details::concepts::decorator<T>)
and csl::ag::formatting::details::concepts::std_formattable<T, Char>
struct std::formatter<
    csl::ag::formatting::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::formatting::details::ag_formatter_base<
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
    csl::ag::formatting::details::decorators::formatted_view_t<T>,
    Char
> : public csl::ag::formatting::details::ag_formatter_base_leaf<
        std::formatter, T, Char
    >
{};
// NOLINTEND(cert-dcl58-cpp)
