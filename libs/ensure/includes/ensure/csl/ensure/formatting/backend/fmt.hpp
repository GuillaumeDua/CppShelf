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

template <typename T, typename tag>
requires requires (const fmt::formatter<T> & underlying, const T & value, fmt::format_context & context) {
    underlying.format(value, context);
}
struct fmt::formatter<csl::ensure::strong_type<T, tag>> : formatter<T> {
    constexpr auto format(const csl::ensure::strong_type<T, tag> & value, fmt::format_context & context) const {
        return formatter<T>::format(csl::ensure::to_underlying(value), context);
    }
};

#else

namespace csl::ensure::details::mp::type_traits {
    // has_fmt_formatter
    template <typename T, class = void>
    struct has_fmt_formatter : std::false_type{};
    template <typename T>
    struct has_fmt_formatter<T, std::void_t<decltype(
        std::declval<const fmt::formatter<T> &>().format(std::declval<const T &>(), std::declval<fmt::format_context&>())
    )>> : std::true_type{};
    template <typename T>
    constexpr inline static bool has_fmt_formatter_v = has_fmt_formatter<T>::value;
}

template <typename T, typename tag>
struct fmt::formatter<
    csl::ensure::strong_type<T, tag>,
    std::enable_if_t<csl::ensure::details::mp::type_traits::has_fmt_formatter_v<T>, char>
> : formatter<T> {
    constexpr auto format(const csl::ensure::strong_type<T, tag> & value, format_context & context) const {
        return fmt::formatter<T>::format(csl::ensure::to_underlying(value), context);
    }
};

#endif
