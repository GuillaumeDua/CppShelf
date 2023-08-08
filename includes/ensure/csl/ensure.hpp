#pragma once

#if not __cplusplus >= 202002L
#pragma error csl/ensure.hpp requires C++20. Use  csl/cxx_17/ensure.hpp
#endif

// TODO(Guss): strong_type: opt-ins
// - Implicit conversion utilities to underlying
// - operators support
//  - invocation
//  - comparison (==, !=, <, >, etc.)
//  - arithmetic binary operators: (+, -, *, /, etc.)
// -            unary operators:  (++, --, not, implicit bool conversion ?)

// TODO(Guss): conversion between strong_type<T> and strong_type<U> where T != U
//  must be explicit and user-defined
//  exemple: millimeter{42} < meter{1}, where both are strong_type int

// TODO(Guss): Serialization support: opt-in in csl::srl side

// TODO(Guss): : ensure
//  not_null<ptr_type>
//  clamped/bounded_integral<lower, upper>

#include <concepts>
#include <type_traits>
#include <utility>
#include <functional>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)                     // NOLINT(cppcoreguidelines-macro-usage)

namespace csl::ensure::details::concepts::comparison {
// not using std concepts (std::equality_comparable) here,
// as given type comparison might not be symetrical for some reasons

    // operator ==
    template <typename T, typename U>
    concept equality_with = requires (const T & lhs, const U & rhs){
        { lhs == rhs } -> std::convertible_to<bool>;
    }
    and std::common_reference_with<
        const std::remove_reference_t<T>&,
        const std::remove_reference_t<U>&
    >;

    // operator not_eq
    template <typename T, typename U>
    concept not_equality_with = requires (const T & lhs, const U & rhs){
        { lhs not_eq rhs } -> std::convertible_to<bool>;
    }
    and std::common_reference_with<
        const std::remove_reference_t<T>&,
        const std::remove_reference_t<U>&
    >;

    // operator<
    template <typename T, typename U>
    concept less_than_comparable_with = requires (const T & lhs, const U & rhs){
        { lhs < rhs } -> std::convertible_to<bool>;
    };
    // operator>
    template <typename T, typename U>
    concept greater_than_comparable_with = requires (const T & lhs, const U & rhs){
        { lhs > rhs } -> std::convertible_to<bool>;
    };

    // operator<=
    template <typename T, typename U>
    concept less_than_or_equal_to_comparable_with = requires (const T & lhs, const U & rhs){
        { lhs <= rhs } -> std::convertible_to<bool>;
    };
    // operator>=
    template <typename T, typename U>
    concept greater_than_or_equal_comparable_with = requires (const T & lhs, const U & rhs){
        { lhs >= rhs } -> std::convertible_to<bool>;
    };
}
namespace csl::ensure
{
	template <typename T, typename tag>
    struct strong_type
    {   // explicit constructor, implicit conversion
        using type = strong_type<T, tag>;

        using underlying_type = T;
        using tag_type = tag;
        using lvalue_reference = T&;
        using rvalue_reference = T&&;
        using const_lvalue_reference = const T &;
        using const_rvalue_reference = const T &&;

        constexpr explicit strong_type(auto && ... values)
        noexcept(std::is_nothrow_constructible_v<underlying_type, decltype(std::forward<decltype(values)>(values))...>)
        requires (std::constructible_from<underlying_type, decltype(std::forward<decltype(values)>(values))...>)
        : value(std::forward<decltype(values)>(values)...)
        {}
        constexpr explicit strong_type(const_lvalue_reference arg)
        noexcept(std::is_nothrow_copy_constructible_v<underlying_type>)
        requires std::copy_constructible<underlying_type>
        : value(arg)
        {}
        constexpr explicit strong_type(underlying_type&& arg)
        noexcept(std::is_nothrow_move_constructible_v<underlying_type>)
        requires std::move_constructible<underlying_type>
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        constexpr strong_type(strong_type&&)
        noexcept(std::is_nothrow_move_constructible_v<underlying_type>)
        requires std::move_constructible<underlying_type>
        = default;
        constexpr strong_type(const strong_type&)
        noexcept(std::is_nothrow_copy_constructible_v<underlying_type>)
        requires std::copy_constructible<underlying_type>
        = default;

        constexpr explicit strong_type()
        noexcept(std::is_nothrow_default_constructible_v<underlying_type>)
        requires std::is_default_constructible_v<underlying_type>
        = default;
        constexpr ~strong_type()
        noexcept(std::is_nothrow_destructible_v<underlying_type>)
        requires std::is_destructible_v<underlying_type>
        = default;

        constexpr lvalue_reference       underlying()        & noexcept { return value; }
        constexpr const_lvalue_reference underlying() const  & noexcept { return value; }
        constexpr rvalue_reference       underlying()        && noexcept { return static_cast<rvalue_reference>(value); }
        constexpr const_rvalue_reference underlying() const  && noexcept { return static_cast<const_rvalue_reference>(value); }

        // TODO(Guss): default: explicit, opt-in: implicit
        constexpr /*explicit*/ operator lvalue_reference ()               & noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr /*explicit*/ operator const_lvalue_reference () const   & noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr /*explicit*/ operator rvalue_reference ()               && noexcept { return static_cast<strong_type&&>(*this).underlying(); }  // NOLINT not explicit on purpose
        constexpr /*explicit*/ operator const_rvalue_reference () const   && noexcept { return static_cast<const strong_type&&>(*this).underlying(); }  // NOLINT not explicit on purpose

        // TODO: arythmetic operators
        //  +, -, *, /,
        //  +=, -=, *=, /=

        // TODO: logic operators ?

        constexpr type & operator=(const type & other)
        noexcept(std::is_nothrow_assignable_v<lvalue_reference, const_lvalue_reference>)
        requires std::is_assignable_v<lvalue_reference, const_lvalue_reference>
        = default;
        constexpr type & operator=(type && other)
        noexcept(std::is_nothrow_assignable_v<lvalue_reference, rvalue_reference>)
        requires std::is_assignable_v<lvalue_reference, rvalue_reference>
        = default;

        constexpr type & operator=(const auto & arg)
        noexcept(std::is_nothrow_assignable_v<underlying_type&, decltype(arg)>)
        requires std::assignable_from<underlying_type&, decltype(arg)>
        {
            value = arg;
            return *this;
        }
        constexpr type & operator=(auto && arg)
        noexcept(std::is_nothrow_assignable_v<underlying_type&, decltype(fwd(arg))>)
        requires std::assignable_from<underlying_type&, decltype(fwd(arg))>
        {
            value = fwd(value);
            return *this;
        }

#pragma region invocation

        // WIP: tests
        // WIP: C++17 retro-compatiblity

        template <typename ... arguments_ts>
        constexpr std::invoke_result_t<lvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) &
        noexcept(std::is_nothrow_invocable_v<lvalue_reference, arguments_ts&&...>)
        requires std::invocable<lvalue_reference, arguments_ts&&...>
        {
            return std::invoke(value, fwd(args)...);
        }
        template <typename ... arguments_ts>
        constexpr std::invoke_result_t<const_lvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) const &
        noexcept(std::is_nothrow_invocable_v<const_lvalue_reference, arguments_ts&&...>)
        requires std::invocable<const_lvalue_reference, arguments_ts&&...>
        {
            return std::invoke(value, fwd(args)...);
        }
        template <typename ... arguments_ts>
        constexpr std::invoke_result_t<rvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) &&
        noexcept(std::is_nothrow_invocable_v<rvalue_reference, arguments_ts&&...>)
        requires std::invocable<rvalue_reference, arguments_ts&&...>
        {
            return std::invoke(static_cast<underlying_type&&>(value), fwd(args)...);
        }
        template <typename ... arguments_ts>
        constexpr std::invoke_result_t<const_rvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) const &&
        noexcept(std::is_nothrow_invocable_v<const_rvalue_reference, arguments_ts&&...>)
        requires std::invocable<const_rvalue_reference, arguments_ts&&...>
        {
            return std::invoke(static_cast<const underlying_type&&>(value), fwd(args)...);
        }
#pragma endregion

#pragma region comparison

        constexpr auto operator<=>(const type & other) const
        noexcept(noexcept(value <=> other.underlying()))
        requires std::three_way_comparable<underlying_type> {
            return value <=> other.underlying();
        }
        constexpr auto operator<=>(const auto & arg) const
        noexcept(noexcept(value <=> arg))
        requires (not std::same_as<std::remove_cvref_t<decltype(arg)>, type>
                  and std::three_way_comparable_with<underlying_type, decltype(arg)>)
        {
            return value <=> arg;
        }

        constexpr auto operator==(const type & arg) const
        noexcept(noexcept(value == arg.underlying()))
        -> bool
        requires details::concepts::comparison::equality_with<underlying_type, underlying_type>
        {
            return value == arg.underlying();
        }
        constexpr auto operator==(const auto & arg) const
        noexcept(noexcept(value == arg))
        -> bool
        requires details::concepts::comparison::equality_with<underlying_type, decltype(arg)>
        {
            return value == arg;
        }
        constexpr auto operator not_eq(const type & arg) const
        noexcept(noexcept(value not_eq arg.underlying()))
        -> bool
        requires details::concepts::comparison::not_equality_with<underlying_type, underlying_type>
        {
            return value not_eq arg.underlying();
        }
        constexpr auto operator not_eq(const auto & arg) const
        noexcept(noexcept(value not_eq arg))
        -> bool
        requires details::concepts::comparison::not_equality_with<underlying_type, decltype(arg)>
        {
            return value not_eq arg;
        }

        constexpr auto operator <(const type & arg) const
        noexcept(noexcept(value < arg.underlying()))
        -> bool
        requires details::concepts::comparison::less_than_comparable_with<underlying_type, underlying_type>
        {
            return value < arg.underlying();
        }
        constexpr auto operator <(const auto & arg) const
        noexcept(noexcept(value < arg))
        -> bool
        requires details::concepts::comparison::less_than_comparable_with<underlying_type, decltype(arg)>
        {
            return value < arg;
        }

        constexpr auto operator >(const type & arg) const
        noexcept(noexcept(value > arg.underlying()))
        -> bool
        requires details::concepts::comparison::greater_than_comparable_with<underlying_type, underlying_type>
        {
            return value > arg.underlying();
        }
        constexpr auto operator >(const auto & arg) const
        noexcept(noexcept(value > arg))
        -> bool
        requires details::concepts::comparison::greater_than_comparable_with<underlying_type, decltype(arg)>
        {
            return value > arg;
        }


        constexpr auto operator <=(const type & arg) const
        noexcept(noexcept(value <= arg.underlying()))
        -> bool
        requires details::concepts::comparison::less_than_or_equal_to_comparable_with<underlying_type, underlying_type>
        {
            return value <= arg.underlying();
        }
        constexpr auto operator <=(const auto & arg) const
        noexcept(noexcept(value <= arg))
        -> bool
        requires details::concepts::comparison::less_than_or_equal_to_comparable_with<underlying_type, decltype(arg)>
        {
            return value <= arg;
        }

        constexpr auto operator >=(const type & arg) const
        noexcept(noexcept(value >= arg.underlying()))
        -> bool
        requires details::concepts::comparison::greater_than_or_equal_comparable_with<underlying_type, underlying_type>
        {
            return value >= arg.underlying();
        }
        constexpr auto operator >=(const auto & arg) const
        noexcept(noexcept(value >= arg))
        -> bool
        requires details::concepts::comparison::greater_than_or_equal_comparable_with<underlying_type, decltype(arg)>
        {
            return value >= arg;
        }

#pragma endregion

    private:
        underlying_type value;
    };

    template <typename T, typename tag>
    constexpr T & to_underlying(strong_type<T, tag> & value) noexcept {
        return static_cast<T&>(value);
    }
    template <typename T, typename tag>
    constexpr const T & to_underlying(const strong_type<T, tag> & value) noexcept {
        return static_cast<const T&>(value);
    }
    template <typename T, typename tag>
    constexpr T && to_underlying(strong_type<T, tag> && value) noexcept {
        return static_cast<T&&>(value);
    }
    template <typename T, typename tag>
    constexpr const T && to_underlying(const strong_type<T, tag> && value) noexcept {
        return static_cast<const T&&>(value);
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
    concept StrongTypeOf = StrongType<strong_type> and csl::ensure::type_traits::is_strong_type_of_v<strong_type, T>;
    template <typename strong_type, typename T>
    concept TaggedBy = StrongType<strong_type> and csl::ensure::type_traits::is_tagged_by_v<strong_type, T>;
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
#endif
#endif

// CPO - fmt::formatter
// TODO(Guss) as opt-in
#if defined (FMT_CORE_H_)
template <typename T, typename tag>
requires requires { std::declval<fmt::formatter<T>>().format(std::declval<T>()); }
struct fmt::formatter<csl::ensure::strong_type<T, tag>> : formatter<T> {
  auto format(const csl::ensure::strong_type<T, tag> & value, format_context & context) {
    return formatter<T>::format(value.to_underlying(), context);
  }
};
#endif

#undef fwd
