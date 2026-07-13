#pragma once

/// @file
/// @brief std::ostream support for structured bindables (opt-in feature header).
/// 
/// cpp shelf library : aggregates utility - formatting.
/// 
/// Provides `operator<<(std::ostream &, structured_bindable)` via `csl::ag::io`.
/// 
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
/// 
/// @warning Prefer @c <csl/ag/formatting/backend/std_format.hpp> (std::format) or
///          @c <csl/ag/formatting/backend/fmt.hpp> (fmtlib) over this when available.
/// 
/// @par Compile-time cost
///      Including @c <ostream> is one of the heaviest standard headers, with
///      significantly slower build times.
/// 
/// @par Runtime cost
///      @c std::ios_base::xalloc() and @c std::ios_base::iword() are used to store
///      the active options on the stream. Both involve a global mutex and can be a
///      bottleneck in hot paths or multi-threaded code.
/// 
/// @par Design
///      - Composable @c format_options bitmask selected via IO manipulators
///        (one-shot, reset after each print) or via the view-based @c operator| API (bypasses iword entirely):
///         @code
///         os << value                                                    // default: braced, compact
///         os << csl::ag::io::no_braces << value                          // flat, naked: no outer brackets or separator
///         os << csl::ag::io::indented  << value                          // multiline, depth-indented
///         os << csl::ag::io::indexed   << value                          // braced with [N] field indexes
///         os << csl::ag::io::typenamed << value                          // braced with TypeName: prefixes
///         os << (value | csl::ag::io::indented | csl::ag::io::indexed)   // view-based, composable
///         @endcode
///      - Options propagate to nested structured_bindable fields (no_braces is outermost-only).
///      - Leaf values consistent with fmtlib: char => 'x', bool => true/false, string => "...".
/// 
/// @par Usage
///      @code
///      using namespace csl::ag::io;
///      std::cout << my_aggregate;
///      @endcode

#if not defined(CSL_AG__INCLUDED)
#   if not __has_include(<csl/ag.hpp>)
#       error "[csl::ag] csl/ag/formatting/backend/ostream.hpp : missing <csl/ag.hpp>. This is an opt-in feature header: make <csl/ag.hpp> reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#   endif
#   include <csl/ag.hpp>
#endif

#include <ostream>

#define csl_fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace csl::ag::io::details {

    /// \brief T has an operator<<(std::ostream &, T) reachable WITHOUT csl::ag::io in scope.
    // This prevents this library operator<< from satisfying the concept (depend on self).
    template <typename T>
    concept ostream_formattable = requires(std::ostream & os, const std::remove_cvref_t<T> & v) {
        os << v;
    };

    // NOTE: external linkage (inline, not static) is required here:
    //       the local static must be the same object in every TU ([dcl.inline]/6),
    //       or each TU would xalloc() its own iword slot.
    inline auto mode_index() noexcept -> int {
        static const int index = std::ios_base::xalloc();
        return index;
    }

    inline void write_indent(std::ostream & os, std::size_t depth) {
        constexpr std::size_t max_depth = 32;
        static constexpr auto buf =
            []<std::size_t... Is>(std::index_sequence<Is...>) {
                return std::array<char, sizeof...(Is)>{ ((void)Is, ' ')... };
            }(std::make_index_sequence<max_depth * style::indentation_width>{});
        os.write(buf.data(), static_cast<std::streamsize>(std::min(max_depth, depth) * style::indentation_width));
    }

    struct format_options_view {

        bool is_indented;
        bool is_no_braces;
        bool is_indexed;
        bool is_typenamed;
        format_options nested; // no_braces is outermost-only

        constexpr explicit format_options_view(format_options options) noexcept
            : is_indented  { bool(options & format_options::indented)  }
            , is_no_braces { bool(options & format_options::no_braces) }
            , is_indexed   { bool(options & format_options::indexed)   }
            , is_typenamed { bool(options & format_options::typenamed) }
            , nested       { options & ~format_options::no_braces      }
        {}
    };

    // NOTE: Forward declaration: print and print_field_value are mutually recursive.
    template <csl::ag::concepts::structured_bindable T>
    void print(std::ostream & os, T && value, format_options options, std::size_t depth)
    requires (not std::is_array_v<std::remove_cvref_t<T>>);

    /// \brief print_field_value: write one field value of type T to an ostream value.
    // Quoting is consistent with fmtlib: char => 'x', bool => true/false, string => "...".
    // WARNING(Known limitation) range-like and tuple-like field values are not recursively pretty-printed;
    //   they require a user-provided operator<< or they hit the static_assert.
    // TODO: get rid of such a limitation, for consistency sake ?
    template <typename T>
    void print_field_value(std::ostream & os, T && value, format_options options, std::size_t depth) {
        using type = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<type, bool>)
            os << (value ? "true" : "false");
        else if constexpr (std::is_same_v<type, char>)
            os << '\'' << value << '\'';
        else if constexpr (std::is_same_v<type, std::string_view> or std::is_same_v<type, std::string>)
            os << '"' << value << '"';
        else if constexpr (ostream_formattable<type>)
            os << value;
        else if constexpr (csl::ag::concepts::structured_bindable<type> and not std::is_array_v<type>)
            print(os, csl_fwd(value), options, depth);
        else
            static_assert(false, "[csl::ag::io] field type is not printable: provide operator<<(std::ostream &, T)");
    }

    template <csl::ag::concepts::structured_bindable T>
    void print(std::ostream & os, T && value, format_options options, std::size_t depth)
    requires (not std::is_array_v<std::remove_cvref_t<T>>)
    {
        using type = std::remove_cvref_t<T>;
        constexpr auto size = csl::ag::tuplelike::size_v<type>;

        const auto opt = format_options_view{ options };

        if (not opt.is_no_braces) os << style::opening_bracket<type>();
        if (opt.is_indented)      os << '\n';

        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
            ([&] {
                
                if constexpr (indexes > 0) {
                    if (not opt.is_no_braces)       os << ',';
                    if (opt.is_indented)            os << '\n';
                    else if (not opt.is_no_braces)  os << ' ';
                }
                
                if (opt.is_indented)
                    write_indent(os, depth + 1);

                if (opt.is_indexed)
                    os << '[' << indexes << "] ";
                
                if (opt.is_typenamed) {
                    using field_type = csl::ag::tuplelike::element_t<indexes, type>;
                    os << type_name_v<field_type> << ": ";
                }
                print_field_value(
                    os,
                    csl::ag::tuplelike::get<indexes>(csl_fwd(value)),
                    opt.nested,
                    depth + 1
                );
            }(), ...);
        }(std::make_index_sequence<size>{});

        if (opt.is_indented){
            os << '\n';
            write_indent(os, depth);
        }
        if (not opt.is_no_braces)
            os << style::closing_bracket<type>();
    }
}

namespace csl::ag::io {

    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - indented
    inline auto operator<<(std::ostream & os, indented_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::indented);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - no_braces
    inline auto operator<<(std::ostream & os, no_braces_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::no_braces);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - indexed
    inline auto operator<<(std::ostream & os, indexed_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::indexed);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - typenamed
    inline auto operator<<(std::ostream & os, typenamed_t) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(format_options::typenamed);
        return os;
    }
    /// \brief Composable std::ostream manipulator (one-shot, reset after use) - precomputed bitmask
    ///        Mirrors the tag-based manipulators above, for a `format_options` combined ahead of time, e.g.
    ///        @code
    ///        constexpr auto options = indented | indexed | typenamed;
    ///        os << options << value;
    ///        @endcode
    inline auto operator<<(std::ostream & os, format_options options) -> std::ostream & {
        os.iword(details::mode_index()) |= std::to_underlying(options);
        return os;
    }

    /// \brief std::ostream formatting using formatted_view. Effectively bypasses iword.
    template <typename T>
    auto operator<<(std::ostream & os, details::decorators::formatted_view_t<T> const & view)
    -> std::ostream &
    {
        details::print(os, view.value, view.options, view.depth);
        return os;
    }

    /// \brief Format a structured_bindable into an std::ostream, using iword options (one-shot) and prints.
    /// User-defined operator<<(std::ostream &, T) wins via overload resolution (exact match).
    auto operator<<(std::ostream & os, const csl::ag::concepts::structured_bindable auto & value)
    -> std::ostream &
    requires (not std::is_array_v<std::remove_cvref_t<decltype(value)>>)
    and (not details::concepts::decorator<std::remove_cvref_t<decltype(value)>>)
    {
        auto options = static_cast<format_options>(os.iword(details::mode_index()));
        os.iword(details::mode_index()) = 0; // reset (one-shot semantics)
        details::print(os, value, options, 0);
        return os;
    }

}

#undef csl_fwd
