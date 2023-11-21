#pragma once

#include <csl/ag.hpp>
#include <memory>
#include <type_traits>

namespace test::ag::custom_tuple_like_interface::userland {
    struct A{ int i = 0; };
    static_assert(csl::ag::concepts::aggregate<A>);
    struct B{}; struct C{}; struct D {}; struct E{};
    struct F{
        template <std::size_t N>
        friend constexpr decltype(auto) get(F &&) noexcept
        {
            return 'F';
        }
    };
    template <std::size_t N>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<userland::D, std::remove_cvref_t<decltype(value)>>
    {
        return 'D';
    }
    template <std::size_t N>
    constexpr decltype(auto) get(E && value) noexcept
    {
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
    template <std::size_t N>
    constexpr decltype(auto) get(test::ag::custom_tuple_like_interface::userland::C && value) noexcept
    {
        return 'C';
    }
// NOLINTEND(cert-dcl58-cpp)
}

template <>
struct std::tuple_size<test::ag::custom_tuple_like_interface::userland::F> : std::integral_constant<std::size_t, 1> {};
template <>
struct std::tuple_element<0, test::ag::custom_tuple_like_interface::userland::F> : std::type_identity<char> {};

// --- test ---

namespace test::ag::custom_tuple_like_interface {
    static constexpr void test(){
        
        using csl::ag::get;
        // using std::get;

        [[maybe_unused]] const auto _ = get<0>(userland::A{}) + get<int>(userland::A{});
    }
}

// TODO(Guss): to integrate: https://godbolt.org/z/7eh3hr3Yn
// TODO(Guss): opt-in tuple-like interface
