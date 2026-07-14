#pragma once

/// @file
/// @brief std::ostream support for strong types (opt-in feature header).
///
/// cpp shelf library : ensure - formatting.
///
/// Provides `operator<<(std::ostream &, const csl::ensure::strong_type<T, tag> &)` via `csl::io`.
///
/// @copyright Copyright (c) 2021 Guillaume Dua "Guss". MIT License.
/// @see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE
///
/// @warning Like @c <fmt/ranges.h>, include this header **consistently across the whole program** (ODR).
///
/// @par Design
///      - Delegates to the underlying type's `operator<<`.
///      - Only participates in overload resolution when the underlying type is ostream-shiftable.
///      - C++17 compatible (SFINAE below C++20, constraints otherwise).
///
/// @par Usage
///      @code
///      using namespace csl::io;
///      std::cout << my_strong_type;
///      @endcode

#if not defined(CSL_ENSURE__INCLUDED)
#   if not __has_include(<csl/ensure.hpp>)
#       error "[csl::ensure] csl/ensure/formatting/backend/ostream.hpp : missing <csl/ensure.hpp>. This is an opt-in feature header: make <csl/ensure.hpp> reachable, or #include it before this header (e.g. on Compiler Explorer, using its raw URL)."
#   endif
#   include <csl/ensure.hpp>
#endif

#include <ostream>

#if __cplusplus >= 202002L

namespace csl::io {
    template <typename T, typename tag>
    requires requires (std::ostream & os, const T & value) { os << value; }
    std::ostream & operator<<(std::ostream & os, const csl::ensure::strong_type<T, tag> & value){
        const auto & underlying_value = static_cast<const T &>(value);
        return os << underlying_value;
    }
}

#else

namespace csl::ensure::details::mp::type_traits {
    // is_ostream_shiftable
    template <typename T, class = void>
    struct is_ostream_shiftable : std::false_type{};
    template <typename T>
    struct is_ostream_shiftable<T,  std::void_t<
        decltype(std::declval<std::ostream&>() << std::declval<const T&>())
    >> : std::true_type{};
    template <typename T>
    constexpr inline static bool is_ostream_shiftable_v = is_ostream_shiftable<T>::value;
}
namespace csl::io {
    template <
        typename T, typename tag,
        std::enable_if_t<csl::ensure::details::mp::type_traits::is_ostream_shiftable_v<T>, bool> = true
    >
    std::ostream & operator<<(std::ostream & os, const csl::ensure::strong_type<T, tag> & value){
        const auto & underlying_value = static_cast<const T &>(value);
        return os << underlying_value;
    }
}

#endif
