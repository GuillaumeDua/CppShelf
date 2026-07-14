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

// NOLINTBEGIN(cert-dcl58-cpp) - std::formatter is a CPO
template <typename T, typename tag>
requires requires (const std::formatter<T> & underlying, const T & value, std::format_context & context) {
    underlying.format(value, context);
}
struct std::formatter<csl::ensure::strong_type<T, tag>> : formatter<T> {
    template <typename Context>
    constexpr auto format(const csl::ensure::strong_type<T, tag> & value, Context & context) const {
        return formatter<T>::format(csl::ensure::to_underlying(value), context);
    }
};
// NOLINTEND(cert-dcl58-cpp)
