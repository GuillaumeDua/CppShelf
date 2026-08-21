#pragma once

/// @file
/// @brief std::format support for strong types (opt-in feature header).
///
/// cpp shelf library : ensure - formatting.
///
/// Provides a `std::formatter` specialization for `csl::ensure::strong_type<T, tag>`.
///
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
///
/// @warning Like @c <fmt/ranges.h>, include this header **consistently across the whole program** (ODR).
///
/// @par Requirements
///      C++20 (`<format>`).
///
/// @par Design
///      - Delegates to the underlying type's formatter: same output, format-spec included.
///      - Only participates when the underlying type is std-formattable.
///      - Generic over the character type: whichever CharT the underlying type's formatter supports.
///
/// @par Usage
///      @code
///      std::println("{}", my_strong_type);
///      @endcode

#if not defined(CSL_ENSURE__INCLUDED)
#   if not __has_include(<csl/ensure.hpp>)
#       error "[csl::ensure] csl/ensure/formatting/backend/std_format.hpp : missing <csl/ensure.hpp>. This is an opt-in feature header: make <csl/ensure.hpp> reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#   endif
#   include <csl/ensure.hpp>
#endif

#if not (__cplusplus >= 202002L)
#   error "[csl::ensure] csl/ensure/formatting/backend/std_format.hpp : requires C++20 (std::format)."
#endif
#if not __has_include(<format>)
#   error "[csl::ensure] csl/ensure/formatting/backend/std_format.hpp : missing <format> (std::format requires C++20)."
#endif

#include <format>
#include <iterator>
#include <string>

namespace csl::ensure::details::mp::type_traits {
    // NOTE: Cannot use std::formattable (C++23)s.
    //       This context is a concrete instantiation used only to probe format(): std::format_context's iterator type is unspecified.
    template <typename CharT>
    using format_context_for = std::basic_format_context<
        std::back_insert_iterator<std::basic_string<CharT>>, CharT
    >;
}

// NOLINTBEGIN(cert-dcl58-cpp) - std::formatter is a CPO
template <typename T, typename tag, typename CharT>
requires requires (
    const std::formatter<T, CharT> & underlying,
    const T & value,
    csl::ensure::details::mp::type_traits::format_context_for<CharT> & context
) {
    underlying.format(value, context);
}
struct std::formatter<csl::ensure::strong_type<T, tag>, CharT> : formatter<T, CharT> {
    template <typename Context>
    constexpr auto format(const csl::ensure::strong_type<T, tag> & value, Context & context) const {
        return formatter<T, CharT>::format(csl::ensure::to_underlying(value), context);
    }
};
// NOLINTEND(cert-dcl58-cpp)
