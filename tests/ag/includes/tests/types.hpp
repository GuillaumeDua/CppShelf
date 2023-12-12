#pragma once

#include <tuple>

namespace test::ag::types {
    template <typename T>
    struct aggregate_all_cvref {
        // NOLINTBEGIN(*-avoid-const-or-ref-data-members)
        T           v0;
        T &         v1;
        T &&        v2;
        const T     v3;
        const T &   v4;
        const T &&  v5;
        // NOLINTEND(*-avoid-const-or-ref-data-members)
        using expected_to_tuple_t = std::tuple<int, int &, int &&, const int, const int &, const int &&>;
    };

    using aggregate_1 = struct { int i; };
    using aggregate_2 = struct { int i; char c; };

    using aggregate_ref_1 = struct { int & i; };
    using aggregate_ref_2 = struct { int & i; char && c; };
    using aggregate_ref_3 = struct { int & i; char && c; char & cc; };
}

namespace test::ag::types::custom_get {

    struct A{ char c = 'A'; };
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
    requires std::same_as<D, std::remove_cvref_t<decltype(value)>>
    {
        return 'D';
    }
    template <std::same_as<char>>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<D, std::remove_cvref_t<decltype(value)>>
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
    requires std::same_as<test::ag::types::custom_get::B, std::remove_cvref_t<decltype(value)>>
    {
        return 'B';
    }
    template <typename T>
    constexpr decltype(auto) get(auto && value) noexcept
    requires std::same_as<test::ag::types::custom_get::B, std::remove_cvref_t<decltype(value)>>
    {
        static_assert(std::same_as<T, char>);
        return 'B';
    }
    template <std::size_t N>
    constexpr decltype(auto) get(test::ag::types::custom_get::C &&) noexcept
    {
        return 'C';
    }
    template <typename T>
    constexpr decltype(auto) get(test::ag::types::custom_get::C &&) noexcept
    {
        static_assert(std::same_as<T, char>);
        return 'C';
    }
// NOLINTEND(cert-dcl58-cpp)
}

template <>
struct std::tuple_size<test::ag::types::custom_get::F> : std::integral_constant<std::size_t, 1> {};
template <>
struct std::tuple_element<0, test::ag::types::custom_get::F> : std::type_identity<char> {};
