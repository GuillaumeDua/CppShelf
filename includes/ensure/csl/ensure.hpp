#pragma once

// strong_types: implicit conversion is enough for the following operations:
// - Conversion utilities
// - Comparison operators: (==, !=, <, >, etc.)
// - Arithmetic binary operators: (+, -, *, /, etc.)
// -            unary operators:  (++, --, !, implicit bool conversion ?)

// WIP
// - Type-traits, concepts
// - Serialization support: opt-in in csl::srl side

// WIP
//  not_null<ptr_type>
//  bounded_integral<lower, upper>

#include <concepts>
#include <utility>

namespace csl::ensure
{
	template <typename T, typename /*type_tag*/>
    struct strong_types
    {
        using underlying_type = T;
        using reference = T&;
        using const_reference = const T &;

        constexpr explicit strong_types(const_reference arg)
        requires std::copy_constructible<T>
        : value(arg)
        {}
        constexpr explicit strong_types(T&& arg)
        requires std::move_constructible<T>
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        constexpr reference       underlying()        { return value; }
        constexpr const_reference underlying() const  { return value; }

        constexpr operator reference ()               { return underlying(); }
        constexpr operator const_reference () const   { return underlying(); }

    private:
        T value;
    };
}

// TODO: useless ? (implicit conversion is enough)
#if defined(__cpp_impl_three_way_comparison)
// comparisons
template <typename T, std::three_way_comparable_with<T> U, typename T_tag>
constexpr auto operator<=>(
    const csl::ensure::strong_types<T, T_tag> & lhs,
    const U & rhs
){
    return static_cast<const T &>(lhs) <=> rhs;
}
template <typename T, std::three_way_comparable_with<T> U, typename U_tag>
constexpr auto operator<=>(
    const T & lhs,
    const csl::ensure::strong_types<U, U_tag> & rhs
){
    return lhs <=> static_cast<const U &>(rhs);
}
template <typename T, std::three_way_comparable_with<T> U, typename T_tag, typename U_tag>
constexpr auto operator<=>(
    const csl::ensure::strong_types<T, T_tag> & lhs,
    const csl::ensure::strong_types<U, U_tag> & rhs
)
{
    return static_cast<const T &>(lhs) <=> static_cast<const U &>(rhs);
}
#endif

#if defined(__has_include)
#if __has_include(<iostream>)

#include <iostream>
// std::ostream& operator<<
namespace csl::io {
    template <typename T, typename tag>
    requires requires {
        std::declval<std::ostream &>() << std::declval<const T&>();
    }
    std::ostream & operator<<(std::ostream & os, const csl::ensure::strong_types<T, tag> & value){
        const auto & underlying_value = static_cast<const T &>(value);
        return os << underlying_value;
    }
}
#endif
#endif