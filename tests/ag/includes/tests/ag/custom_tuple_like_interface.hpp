#pragma once

#include <csl/ag.hpp>
#include <memory>
#include <type_traits>

namespace test::ag::custom_tuple_like_interface {
    struct type{
        int i = 0;
    };
    static_assert(csl::ag::concepts::aggregate<type>);
}

#include <memory>
using qwe = std::allocator_traits<char>;

namespace std {
// NOLINTBEGIN(cert-dcl58-cpp)
    template <std::size_t N>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<std::remove_cvref_t<decltype(value)>, test::ag::custom_tuple_like_interface::type>
    {
        return value.i;
    }
    template <typename T>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<std::remove_cvref_t<decltype(value)>, test::ag::custom_tuple_like_interface::type>
    {
        return value.i;
    }

    template <std::size_t N>
    struct tuple_element<N, test::ag::custom_tuple_like_interface::type> : std::type_identity<int>{};
// NOLINTEND(cert-dcl58-cpp)
}

namespace test::ag::custom_tuple_like_interface {
    static constexpr void test(){
        
        static_assert(std::same_as<int, std::tuple_element_t<0, type>>);
        auto value = type{};
        [[maybe_unused]] const auto _ = std::get<0>(value) + std::get<int>(value);
    }
}

// WIP/MVE: https://godbolt.org/z/Tfdhj5vjc
