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
#include <type_traits>
#include <utility>

namespace csl::ensure
{
	template <typename T, typename tag>
    struct strong_type
    {   // explicit constructor, implicit conversion
        using type = strong_type<T, tag>;

        using underlying_type = T;
        using tag_type = tag;
        using reference = T&;
        using const_reference = const T &;

        constexpr explicit strong_type(auto && ... values)
        noexcept(std::is_nothrow_constructible_v<underlying_type, decltype(std::forward<decltype(values)>(values))...>)
        requires (std::constructible_from<underlying_type, decltype(std::forward<decltype(values)>(values))...>)
        : value(std::forward<decltype(values)>(values)...)
        {}
        constexpr explicit strong_type(const_reference arg)
        noexcept(std::is_nothrow_copy_constructible_v<underlying_type>)
        requires std::copy_constructible<underlying_type>
        : value(arg)
        {}
        constexpr explicit strong_type(underlying_type&& arg)
        noexcept(std::is_nothrow_move_constructible_v<underlying_type>)
        requires std::move_constructible<underlying_type>
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        constexpr explicit strong_type()
        noexcept(std::is_nothrow_default_constructible_v<underlying_type>)
        requires std::is_default_constructible_v<underlying_type>
        = default;
        constexpr ~strong_type()
        noexcept(std::is_nothrow_destructible_v<underlying_type>)
        requires std::is_destructible_v<underlying_type>
        = default;

        constexpr reference       underlying()        noexcept { return value; }
        constexpr const_reference underlying() const  noexcept { return value; }

        constexpr operator reference ()               noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr operator const_reference () const   noexcept { return underlying(); }  // NOLINT not explicit on purpose

        constexpr auto operator<=>(const type & other) const
        noexcept(noexcept(value <=> other.value))
        requires std::three_way_comparable<underlying_type> {
            return value <=> other.value;
        }
        constexpr auto operator<=>(const auto & arg) const
        noexcept(noexcept(value <=> arg))
        requires (not std::same_as<std::remove_cvref_t<decltype(arg)>, type>
                  and std::three_way_comparable_with<underlying_type, decltype(arg)>)
        {
            return value <=> arg;
        }

        constexpr auto operator==(const type & other) const
        noexcept(noexcept(value == other.value))
        -> bool
        requires std::equality_comparable<underlying_type>
        {
            return value == other.value;
        }
        constexpr auto operator==(const auto & arg) const
        noexcept(noexcept(value == arg))
        -> bool
        requires (not std::same_as<std::remove_cvref_t<decltype(arg)>, type>
                  and std::equality_comparable_with<T, decltype(arg)>)
        {
            return value == arg;
        }

    private:
        T value;
    };

    template <typename T, typename tag>
    T & to_underlying(strong_type<T, tag> & value) noexcept {
        return static_cast<T&>(value);
    }
    template <typename T, typename tag>
    const T & to_underlying(const strong_type<T, tag> & value) noexcept {
        return static_cast<const T&>(value);
    }
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
    template <typename strong_underlying_t, typename strong_tag_t>
    struct is_strong_type_of<
        csl::ensure::strong_type<strong_underlying_t, strong_tag_t>,
        strong_underlying_t
    > : std::true_type{};
    template <typename strong_type, typename T>
    constexpr bool is_strong_type_of_v = is_strong_type_of<strong_type, T>::value;

// is_tagged_by
    template <typename, typename>
    struct is_tagged_by : std::false_type{};
    template <typename strong_underlying_t, typename strong_tag_t>
    struct is_tagged_by<
        csl::ensure::strong_type<strong_underlying_t, strong_tag_t>,
        strong_tag_t
    > : std::true_type{};
    template <typename T, typename U>
    constexpr bool is_tagged_by_v = is_tagged_by<T, U>::value;

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
    template <typename strong_type, typename T>
    concept TaggedBy = StrongType<T> and csl::ensure::type_traits::is_tagged_by_v<strong_type, T>;
}

// STL compatibility/interoperability
#include <functional>
namespace csl::ensure {
    // CPO - hasher
    struct strong_type_hasher {
        auto operator()(const csl::ensure::concepts::StrongType auto & value) const {
        // TODO(Guss): requires hashable
            using type = std::remove_cvref_t<decltype(value)>;
            using hasher = std::hash<csl::ensure::type_traits::underlying_type_t<type>>;
            return std::invoke(hasher{}, value);
        }
    };
    // CPO - comparator
    struct strong_type_comparator
    {
        template <csl::ensure::concepts::StrongType T>
        requires std::equality_comparable<csl::ensure::type_traits::underlying_type_t<T>>
        constexpr bool operator()(const T & lhs, const T & rhs) const
        {
            using comparator = std::equal_to<csl::ensure::type_traits::underlying_type_t<T>>;
            return std::invoke(comparator{}, lhs, rhs);
        }
    };
}
// CPO - std::hash
template <typename T, typename tag>
struct std::hash<csl::ensure::strong_type<T, tag>> : csl::ensure::strong_type_hasher{}; // NOLINT(cert-dcl58-cpp)

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

// CPO - fmt::formatter
#if defined (FMT_CORE_H_)
template <typename T, typename tag>
requires requires { std::declval<fmt::formatter<T>>().format(std::declval<T>()); }
struct fmt::formatter<csl::ensure::strong_type<T, tag>> : formatter<T> {
  auto format(const csl::ensure::strong_type<T, tag> & value, format_context & context) {
    return formatter<T>::format(value.to_underlying(), context);
  }
};
#endif

#endif
#endif
