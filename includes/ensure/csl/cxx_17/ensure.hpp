#pragma once

#include <type_traits>
#include <utility>

namespace csl::ensure::details::mp {
#if defined(__cpp_lib_type_identity)
    template <typename T>
    using type_identity = typename std::type_identity<T>;
#else
    template <typename T>
    struct type_identity{ using type = T; };
#endif
}
namespace csl::ensure::details::mp::type_traits {
    // is_equality_comparable_with
    template <class, class, class = void>
    struct is_equality_comparable_with : std::false_type {};
    template <class T, class U>
    struct is_equality_comparable_with<T, U, std::void_t<
        decltype(std::declval<const T&>() == std::declval<const U&>())
    >> : std::true_type {};
    template <typename T, typename U>
    constexpr bool is_equality_comparable_with_v = is_equality_comparable_with<T, U>::value;

    // is_equality_comparable
    template <class T>
    struct is_equality_comparable : is_equality_comparable_with<T, T> {};
    template <typename T>
    constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;
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

        template <typename ...Ts, typename = std::enable_if_t<std::is_constructible_v<underlying_type, Ts&&...>>>
        constexpr explicit strong_type(Ts && ... values)
        noexcept(std::is_nothrow_constructible_v<underlying_type, Ts&&...>)
        : value(std::forward<decltype(values)>(values)...)
        {}
        template <typename = std::enable_if_t<std::is_copy_constructible_v<underlying_type>>>
        constexpr explicit strong_type(const_lvalue_reference arg)
        noexcept(std::is_nothrow_copy_constructible_v<underlying_type>)
        : value(arg)
        {}
        
        template <typename = std::enable_if_t<std::is_move_constructible_v<underlying_type>>>
        constexpr explicit strong_type(underlying_type&& arg)
        noexcept(std::is_nothrow_move_constructible_v<underlying_type>)
        : value{ std::forward<decltype(arg)>(arg) }
        {}

        // TODO: assign operators

        constexpr lvalue_reference       underlying()        & noexcept { return value; }
        constexpr const_lvalue_reference underlying() const  & noexcept { return value; }
        constexpr rvalue_reference       underlying()        && noexcept { return static_cast<rvalue_reference>(value); }
        constexpr const_rvalue_reference underlying() const  && noexcept { return static_cast<const_rvalue_reference>(value); }

        constexpr operator lvalue_reference ()               & noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr operator const_lvalue_reference () const   & noexcept { return underlying(); }  // NOLINT not explicit on purpose
        constexpr operator rvalue_reference ()               && noexcept { return static_cast<strong_type&&>(*this).underlying(); }  // NOLINT not explicit on purpose
        constexpr operator const_rvalue_reference () const   && noexcept { return static_cast<const strong_type&&>(*this).underlying(); }  // NOLINT not explicit on purpose

        // TODO: comparisons

        template <
            std::enable_if_t<details::mp::type_traits::is_equality_comparable_v<T>, bool> = true
        >
        constexpr auto operator==(const type & other) const
        noexcept(noexcept(value == other.value))
        -> bool
        {
            return value == other.value;
        }
        template <
            typename other_type,
            std::enable_if_t<details::mp::type_traits::is_equality_comparable_with_v<T, other_type>, bool> = true
        >
        constexpr auto operator==(const other_type & arg) const
        noexcept(noexcept(value == arg))
        -> bool
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
#endif

// TODO(Guss): fmt
