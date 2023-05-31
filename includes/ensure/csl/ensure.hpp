#pragma once

// strong_type: implicit conversion is enough for the following operations:
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
    struct strong_type
    {
        using underlying_type = T;
        using reference = T&;
        using const_reference = const T &;

        constexpr explicit strong_type(const_reference arg)
        requires std::copy_constructible<T>
        : value(arg)
        {}
        constexpr explicit strong_type(T&& arg)
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
namespace csl::ensure::type_traits {
// is_strong_type
    template <typename>
    struct is_strong_type : std::false_type{};
    template <typename T, typename tag>
    struct is_strong_type<csl::ensure::strong_type<T, tag>> : std::true_type{};
    template <typename T>
    constexpr bool is_strong_type_v = is_strong_type<T>::value;

// is_strong_type_of
    template <typename, typename>
    struct is_strong_type_of : std::false_type{};
    template <typename T, typename strong_underlying_t, typename strong_tag_t>
    struct is_strong_type_of<
        csl::ensure::strong_type<strong_underlying_t, strong_tag_t>,
        T
    > : std::is_same<T, strong_underlying_t>{};
    template <typename strong_type, typename T>
    constexpr bool is_strong_type_of_v = is_strong_type_of<strong_type, T>::value;

// underlying_type
    template <typename>
    struct underlying_type;
    template <typename T, typename tag>
    struct underlying_type<csl::ensure::strong_type<T, tag>> : std::type_identity<T>{};
    template <typename T>
    using underlying_type_t = typename underlying_type<T>::type;

// tag_type
    template <typename>
    struct tag_type;
    template <typename T, typename tag>
    struct tag_type<csl::ensure::strong_type<T, tag>> : std::type_identity<tag>{};
    template <typename T>
    using tag_type_t = typename tag_type<T>::type;
}
namespace csl::ensure::concepts {
    template <typename T>
    concept StrongType = csl::ensure::type_traits::is_strong_type_v<T>;
    template <typename T>
    concept NotStrongType = not csl::ensure::type_traits::is_strong_type_v<T>;
    template <typename strong_type, typename T>
    concept StrongTypeOf = csl::ensure::type_traits::is_strong_type_of_v<strong_type, T>;
}

// TODO: useless ? (implicit conversion is enough)
#if defined(__cpp_impl_three_way_comparison)
// comparisons
template <typename T, std::three_way_comparable_with<T> U, typename T_tag>
constexpr auto operator<=>(
    const csl::ensure::strong_type<T, T_tag> & lhs,
    const U & rhs
){
    return static_cast<const T &>(lhs) <=> rhs;
}
template <typename T, std::three_way_comparable_with<T> U, typename U_tag>
constexpr auto operator<=>(
    const T & lhs,
    const csl::ensure::strong_type<U, U_tag> & rhs
){
    return lhs <=> static_cast<const U &>(rhs);
}
template <typename T, std::three_way_comparable_with<T> U, typename T_tag, typename U_tag>
constexpr auto operator<=>(
    const csl::ensure::strong_type<T, T_tag> & lhs,
    const csl::ensure::strong_type<U, U_tag> & rhs
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
    std::ostream & operator<<(std::ostream & os, const csl::ensure::strong_type<T, tag> & value){
        const auto & underlying_value = static_cast<const T &>(value);
        return os << underlying_value;
    }
}
#endif
#endif