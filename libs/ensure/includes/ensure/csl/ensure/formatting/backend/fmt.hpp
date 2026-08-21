#pragma once

/// @file
/// @brief fmtlib support for strong types (opt-in feature header).
///
/// cpp shelf library : ensure - formatting.
///
/// Provides a `fmt::formatter` specialization for `csl::ensure::strong_type<T, tag>`.
///
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
///
/// @warning Like @c <fmt/ranges.h>, include this header **consistently across the whole program** (ODR).
///
/// @par Requirements
///      fmtlib (https://github.com/fmtlib/fmt) is provided by the consumer - this header does not fetch it.
///
/// @par Design
///      - Delegates to the underlying type's formatter: same output, format-spec included.
///      - Only participates when the underlying type is fmt-formattable.
///      - C++17 compatible (SFINAE below C++20, constraints otherwise).
///      - Generic over the character type: whichever CharT the underlying type's formatter supports.
///        Wide formatting additionally requires fmtlib's <fmt/xchar.h>, which the consumer provides.
///
/// @par Usage
///      @code
///      fmt::println("{}", my_strong_type);
///      @endcode

#if not defined(CSL_ENSURE__INCLUDED)
#   if not __has_include(<csl/ensure.hpp>)
#       error "[csl::ensure] csl/ensure/formatting/backend/fmt.hpp : missing <csl/ensure.hpp>. This is an opt-in feature header: make <csl/ensure.hpp> reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#   endif
#   include <csl/ensure.hpp>
#endif

#if not __has_include(<fmt/format.h>)
#   error "[csl::ensure] csl/ensure/formatting/backend/fmt.hpp : missing <fmt/format.h>. Provide fmtlib in the build - https://github.com/fmtlib/fmt (on Compiler Explorer: Libraries -> add fmt)."
#endif

#include <fmt/format.h>

#if __cplusplus >= 202002L

template <typename T, typename tag, typename CharT>
requires (fmt::is_formattable<T, CharT>::value)
struct fmt::formatter<csl::ensure::strong_type<T, tag>, CharT> : formatter<T, CharT> {
    template <typename Context>
    constexpr auto format(const csl::ensure::strong_type<T, tag> & value, Context & context) const {
        return formatter<T, CharT>::format(csl::ensure::to_underlying(value), context);
    }
};

#else

// NOTE: the SFINAE payload belongs in fmt::formatter's third (Enable) parameter, leaving the second (CharT) free.
template <typename T, typename tag, typename CharT>
struct fmt::formatter<
    csl::ensure::strong_type<T, tag>, CharT,
    std::enable_if_t<fmt::is_formattable<T, CharT>::value>
> : formatter<T, CharT> {
    template <typename Context>
    constexpr auto format(const csl::ensure::strong_type<T, tag> & value, Context & context) const {
        return fmt::formatter<T, CharT>::format(csl::ensure::to_underlying(value), context);
    }
};

#endif
