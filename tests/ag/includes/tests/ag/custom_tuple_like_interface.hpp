#pragma once

#include <csl/ag.hpp>
#include <memory>
#include <type_traits>

namespace test::ag::custom_tuple_like_interface::userland {
    struct A{ char c = 'A'; };
    static_assert(csl::ag::concepts::aggregate<A>);
    struct B{}; struct C{}; struct D {}; struct E{};
    struct F{
        template <std::size_t N>
        friend constexpr decltype(auto) get(F &&) noexcept { return 'F'; }
        template <typename T>
        friend constexpr decltype(auto) get(F &&) noexcept {
            static_assert(std::same_as<T, char>);
            return 'F';
        }
    };
    template <std::size_t N>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<userland::D, std::remove_cvref_t<decltype(value)>>
    {
        return 'D';
    }
    template <std::same_as<char>>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<userland::D, std::remove_cvref_t<decltype(value)>>
    {
        return 'D';
    }
    template <std::size_t N>
    constexpr decltype(auto) get(E &&) noexcept
    {
        return 'E';
    }
    template <typename T>
    constexpr decltype(auto) get(E &&) noexcept
    {
        static_assert(std::same_as<T, char>);
        return 'E';
    }
}

namespace std {
// NOLINTBEGIN(cert-dcl58-cpp)
    template <std::size_t N>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<test::ag::custom_tuple_like_interface::userland::B, std::remove_cvref_t<decltype(value)>>
    {
        return 'B';
    }
    template <typename T>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<test::ag::custom_tuple_like_interface::userland::B, std::remove_cvref_t<decltype(value)>>
    {
        static_assert(std::same_as<T, char>);
        return 'B';
    }
    template <std::size_t N>
    constexpr decltype(auto) get(test::ag::custom_tuple_like_interface::userland::C &&) noexcept
    {
        return 'C';
    }
    template <typename T>
    constexpr decltype(auto) get(test::ag::custom_tuple_like_interface::userland::C &&) noexcept
    {
        static_assert(std::same_as<T, char>);
        return 'C';
    }
// NOLINTEND(cert-dcl58-cpp)
}

template <>
struct std::tuple_size<test::ag::custom_tuple_like_interface::userland::F> : std::integral_constant<std::size_t, 1> {};
template <>
struct std::tuple_element<0, test::ag::custom_tuple_like_interface::userland::F> : std::type_identity<char> {};

// --- test ---
namespace test::ag::custom_tuple_like_interface::details {

    template <typename T, char expected>
    constexpr decltype(auto) ensure_unqualified_get() noexcept {
        using csl::ag::get;
        using std::get;
        static_assert(requires{ get<0>(std::declval<T>()); });
        static_assert(requires{ get<char>(std::declval<T>()); });
        static_assert(get<0>(T{}) == expected);
        static_assert(get<char>(T{}) == expected);
    }
}
namespace test::ag::custom_tuple_like_interface {

    template <typename ... Ts>
    constexpr static void valid_get(){
        ((details::ensure_unqualified_get<typename Ts::input, Ts::expected>()), ...);
    }

    template <typename T, char value>
    struct test_case {
        using input = T;
        constexpr static inline auto expected = value;
    };

    constexpr static void test(){
        using namespace userland;
        valid_get<
            test_case<A, 'A'>,
            test_case<B, 'B'>,
            test_case<C, 'C'>,
            test_case<D, 'D'>,
            test_case<E, 'E'>,
            test_case<F, 'F'>
        >();
    }
}
