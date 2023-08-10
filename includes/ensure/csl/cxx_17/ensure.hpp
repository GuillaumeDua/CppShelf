#pragma once

#include <type_traits>
#include <utility>
#include <functional>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)                     // NOLINT(cppcoreguidelines-macro-usage)

namespace csl::ensure::details::mp {
#if defined(__cpp_lib_type_identity)
    template <typename T>
    using type_identity = typename std::type_identity<T>;
#else
    template <typename T>
    struct type_identity{ using type = T; };
#endif
}
namespace csl::ensure::details::mp::type_traits::impl {
    // is_aggregate_constructible_impl
    template <class, typename = void, typename ...>
    struct is_aggregate_constructible_impl : std::false_type{};
    template <class T, typename ... args_ts>
    struct is_aggregate_constructible_impl<T, std::void_t<
        decltype(T{std::declval<args_ts>()...})
    >, args_ts...> : std::true_type{};
}
namespace csl::ensure::details::mp::type_traits {

    // is_aggregate_constructible
    template <class T, typename ... args_ts>
    struct is_aggregate_constructible : impl::is_aggregate_constructible_impl<T, void, args_ts...>{};
    template <class T, typename ... args_ts>
    constexpr bool is_aggregate_constructible_v = is_aggregate_constructible<T, args_ts...>::value;
}
namespace csl::ensure::details::mp::type_traits::comparison {
    // is_equality_comparable_with
    template <class, class, class = void>
    struct is_equality_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_equality_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() == std::declval<const U&>())
    >> : std::is_convertible<bool, decltype(std::declval<const T&>() == std::declval<const U&>())> {};
    template <typename T, typename U>
    constexpr bool is_equality_comparable_with_v = is_equality_comparable_with<T, U>::value;

    // operator==
    template <class T>
    struct is_equality_comparable : is_equality_comparable_with<T, T> {};
    template <typename T>
    constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

    // is_not_equality_comparable_with
    template <class, class, class = void>
    struct is_not_equality_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_not_equality_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() not_eq std::declval<const U&>())
    >> : std::is_convertible<bool, decltype(std::declval<const T&>() not_eq std::declval<const U&>())> {};
    template <typename T, typename U>
    constexpr bool is_not_equality_comparable_with_v = is_not_equality_comparable_with<T, U>::value;

    // operator not_eq
    template <class T>
    struct is_not_equality_comparable : is_not_equality_comparable_with<T, T>{};
    template <typename T>
    constexpr bool is_not_equality_comparable_v = is_not_equality_comparable<T>::value;

    // is_less_than_comparable_with
    template <class, class, class = void>
    struct is_less_than_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_less_than_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() < std::declval<const U&>())
    >> : std::is_convertible<bool, decltype(std::declval<const T&>() < std::declval<const U&>())> {};
    template <typename T, typename U>
    constexpr bool is_less_than_comparable_with_v = is_less_than_comparable_with<T, U>::value;

    // operator <
    template <class T>
    struct is_less_than_comparable : is_less_than_comparable_with<T, T>{};
    template <typename T>
    constexpr bool is_less_than_comparable_v = is_less_than_comparable<T>::value;

    // is_more_than_comparable_with
    template <class, class, class = void>
    struct is_more_than_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_more_than_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() > std::declval<const U&>())
    >> : std::is_convertible<bool, decltype(std::declval<const T&>() > std::declval<const U&>())> {};
    template <typename T, typename U>
    constexpr bool is_more_than_comparable_with_v = is_more_than_comparable_with<T, U>::value;

    // operator >
    template <class T>
    struct is_more_than_comparable : is_more_than_comparable_with<T, T>{};
    template <typename T>
    constexpr bool is_more_than_comparable_v = is_more_than_comparable<T>::value;

    // is_less_equal_comparable
    template <class, class, class = void>
    struct is_less_equal_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_less_equal_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() <= std::declval<const U&>())
    >> : std::is_convertible<bool, decltype(std::declval<const T&>() <= std::declval<const U&>())> {};
    template <typename T, typename U>
    constexpr bool is_less_equal_comparable_with_v = is_less_equal_comparable_with<T, U>::value;

    // operator <=
    template <class T>
    struct is_less_equal_comparable : is_less_equal_comparable_with<T, T>{};
    template <typename T>
    constexpr bool is_less_equal_comparable_v = is_less_equal_comparable<T>::value;

    // is_more_equal_comparable
    template <class, class, class = void>
    struct is_more_equal_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_more_equal_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() >= std::declval<const U&>())
    >> : std::is_convertible<bool, decltype(std::declval<const T&>() >= std::declval<const U&>())> {};
    template <typename T, typename U>
    constexpr bool is_more_equal_comparable_with_v = is_more_equal_comparable_with<T, U>::value;

    // operator <=
    template <class T>
    struct is_more_equal_comparable : is_more_equal_comparable_with<T, T>{};
    template <typename T>
    constexpr bool is_more_equal_comparable_v = is_more_equal_comparable<T>::value;
}
namespace csl::ensure::details::mp::type_traits::arythmetic {
    // operator+(T,U)
    template<class, class, class = void>
    struct supports_op_plus_with : std::false_type {};
    template <class T, class U>
    struct supports_op_plus_with<T, U, std::void_t<
        decltype(std::declval<const T&>() + std::declval<U &&>())>
    > : std::true_type {};
    template <class T, class U>
    constexpr bool supports_op_plus_with_v = supports_op_plus_with<T,U>::value;
    // operator+(T, T)
    template <class T>
    struct supports_op_plus : supports_op_plus_with<T, T> {};
    template <typename T>
    constexpr bool supports_op_plus_v = supports_op_plus<T>::value;
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

        // constructor: aggregate-initialization
        template <std::enable_if_t<
            std::is_default_constructible_v<underlying_type>
        , bool> = true
        >
        constexpr explicit strong_type()
        noexcept(std::is_nothrow_default_constructible_v<underlying_type>)
        {}
        template <typename ...Ts, std::enable_if_t<
            std::is_aggregate_v<T> and details::mp::type_traits::is_aggregate_constructible_v<underlying_type, Ts&&...>
        , bool> = true
        >
        // constructor: values
        constexpr explicit strong_type(Ts && ... values)
        noexcept(noexcept(underlying_type{ std::forward<decltype(values)>(values)... }))
        : value{ std::forward<decltype(values)>(values)... }
        {}
        template <typename ...Ts, std::enable_if_t<
            std::is_constructible_v<underlying_type, Ts&&...>
        , bool> = true
        >
        constexpr explicit strong_type(Ts && ... values)
        noexcept(std::is_nothrow_constructible_v<underlying_type, Ts&&...>)
        : value(std::forward<decltype(values)>(values)...)
        {}
        // constructor: copy
        template <std::enable_if_t<std::is_copy_constructible_v<underlying_type>, bool> = true>
        constexpr explicit strong_type(const_lvalue_reference arg)
        noexcept(std::is_nothrow_copy_constructible_v<underlying_type>)
        : value(arg)
        {}
        // constructor: move
        template <std::enable_if_t<std::is_move_constructible_v<underlying_type>, bool> = true>
        constexpr explicit strong_type(underlying_type&& arg)
        noexcept(std::is_nothrow_move_constructible_v<underlying_type>)
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        constexpr lvalue_reference       underlying()        & noexcept { return value; }
        constexpr const_lvalue_reference underlying() const  & noexcept { return value; }
        constexpr rvalue_reference       underlying()        && noexcept { return static_cast<rvalue_reference>(value); }
        constexpr const_rvalue_reference underlying() const  && noexcept { return static_cast<const_rvalue_reference>(value); }

        constexpr /*explicit*/ operator lvalue_reference ()               & noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr /*explicit*/ operator const_lvalue_reference () const   & noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr /*explicit*/ operator rvalue_reference ()               && noexcept { return static_cast<strong_type&&>(*this).underlying(); }  // NOLINT not explicit on purpose
        constexpr /*explicit*/ operator const_rvalue_reference () const   && noexcept { return static_cast<const strong_type&&>(*this).underlying(); }  // NOLINT not explicit on purpose

        // TODO: arythmetic operators
        //  +, -, *, /,
        //  +=, -=, *=, /=

        constexpr type & operator=(const type & other)
        noexcept(std::is_nothrow_assignable_v<lvalue_reference, const_lvalue_reference>) = default;
        constexpr type & operator=(type && other)
        noexcept(std::is_nothrow_assignable_v<lvalue_reference, rvalue_reference>) = default;

        template <typename arg_type,
            std::enable_if_t<std::is_assignable_v<underlying_type&, const arg_type&>, bool> = true
        >
        constexpr type & operator=(const arg_type & arg)
        noexcept(std::is_nothrow_assignable_v<underlying_type&, decltype(arg)>)
        {
            value = arg;
            return *this;
        }
        template <typename arg_type,
            std::enable_if_t<std::is_assignable_v<underlying_type&, arg_type &&>, bool> = true
        >
        constexpr type & operator=(arg_type && arg)
        noexcept(std::is_nothrow_assignable_v<underlying_type&, decltype(fwd(arg))>)
        {
            value = fwd(value);
            return *this;
        }

        // TODO: comparisons
        // <,>,
        // <=, >=

#pragma region comparison
        template <
            typename ... arguments_ts,
            std::enable_if_t<
                std::is_invocable_v<lvalue_reference, arguments_ts&&...>
            , bool> = true
        >
        constexpr std::invoke_result_t<lvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) &
        noexcept(std::is_nothrow_invocable_v<lvalue_reference, arguments_ts&&...>)
        {
            return std::invoke(value, fwd(args)...);
        }
        template <
            typename ... arguments_ts,
            std::enable_if_t<
                std::is_invocable_v<const_lvalue_reference, arguments_ts&&...>
            , bool> = true
        >
        constexpr std::invoke_result_t<const_lvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) const &
        noexcept(std::is_nothrow_invocable_v<const_lvalue_reference, arguments_ts&&...>)
        {
            return std::invoke(value, fwd(args)...);
        }
        template <
            typename ... arguments_ts,
            std::enable_if_t<
                std::is_invocable_v<rvalue_reference, arguments_ts&&...>
            , bool> = true
        >
        constexpr std::invoke_result_t<rvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) &&
        noexcept(std::is_nothrow_invocable_v<rvalue_reference, arguments_ts&&...>)
        {
            return std::invoke(value, fwd(args)...);
        }
        template <
            typename ... arguments_ts,
            std::enable_if_t<
                std::is_invocable_v<const_rvalue_reference, arguments_ts&&...>
            , bool> = true
        >
        constexpr std::invoke_result_t<const_rvalue_reference, arguments_ts&&...>
        operator()(arguments_ts && ... args) const &&
        noexcept(std::is_nothrow_invocable_v<const_rvalue_reference, arguments_ts&&...>)
        {
            return std::invoke(value, fwd(args)...);
        }
#pragma endregion
#pragma region comparison
        // comparison: operator ==
        template <
            typename other_type,
            std::enable_if_t<
                details::mp::type_traits::comparison::is_equality_comparable_with_v<T, other_type>
            , bool> = true
        >
        constexpr auto operator==(const other_type & arg) const
        noexcept(noexcept(value == arg))
        -> bool
        {
            return value == arg;
        }
        // comparison: operator not_eq
        template <
            typename other_type,
            std::enable_if_t<
                details::mp::type_traits::comparison::is_not_equality_comparable_with_v<T, other_type>
            , bool> = true
        >
        constexpr auto operator not_eq(const other_type & arg) const
        noexcept(noexcept(value not_eq arg))
        -> bool
        {
            return value not_eq arg;
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
    struct underlying_type<csl::ensure::strong_type<T, tag>> : details::mp::type_identity<T>{};
    template <typename T>
    using underlying_type_t = typename underlying_type<T>::type;

// tag_type
    template <typename>
    struct tag_type;
    template <typename T, typename tag>
    struct tag_type<csl::ensure::strong_type<T, tag>> : details::mp::type_identity<tag>{};
    template <typename T>
    using tag_type_t = typename tag_type<T>::type;
}

// STL compatibility/interoperability
#include <functional>
namespace csl::ensure {
    // CPO - hasher
    struct strong_type_hasher {
        template <
            typename T,
            std::enable_if_t<
                csl::ensure::type_traits::is_strong_type_v<T>
            , bool> = true
        >
        auto operator()(const T & value) const {
        // TODO(Guss): requires hashable
            using type = std::decay_t<decltype(value)>;
            using hasher = std::hash<csl::ensure::type_traits::underlying_type_t<type>>;
            return std::invoke(hasher{}, value);
        }
    };
    // CPO - comparator
    struct strong_type_comparator
    {
        template <
            typename T,
            std::enable_if_t<
                    csl::ensure::type_traits::is_strong_type_v<T>
                and csl::ensure::details::mp::type_traits::comparison::is_equality_comparable_v<
                    typename csl::ensure::type_traits::underlying_type_t<T>
                >
            , bool> = true
        >
        constexpr bool operator()(const T & lhs, const T & rhs) const
        {
            using comparator = std::equal_to<csl::ensure::type_traits::underlying_type_t<T>>;
            return comparator{}(to_underlying(lhs), to_underlying(rhs));
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
namespace csl::ensure::details::mp::type_traits {
    // is_ostream_shiftable
    template <typename T, class = void>
    struct is_ostream_shiftable : std::false_type{};
    template <typename T>
    struct is_ostream_shiftable<T,  std::void_t<
        decltype(std::declval<std::ostream&>() << std::declval<const T&>())
    >> : std::true_type{};
    template <typename T>
    constexpr bool is_ostream_shiftable_v = is_ostream_shiftable<T>::value;
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

#if __has_include(<fmt/core.h>) and __has_include(<fmt/format.h>)
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/format.h>

namespace csl::ensure::details::mp::type_traits {
    template <typename T, class = void>
    struct has_fmt_formatter : std::false_type{};
    template <typename T>
    struct has_fmt_formatter<T, std::void_t<decltype(
        std::declval<fmt::formatter<T>>().format(std::declval<const T &>(), std::declval<fmt::format_context&>())
    )>> : std::true_type{};
    template <typename T>
    constexpr bool has_fmt_formatter_v = has_fmt_formatter<T>::value;
}

template <typename T, typename tag>
struct fmt::formatter<
    csl::ensure::strong_type<T, tag>,
    std::enable_if_t<csl::ensure::details::mp::type_traits::has_fmt_formatter_v<T>, char>
> : formatter<T> {
    static auto format(const csl::ensure::strong_type<T, tag> & value, format_context & context) {
        return fmt::formatter<T>{}.format(csl::ensure::to_underlying(value), context);
    }
};

#endif

#undef fwd
