#pragma once

#include <tuple>
#include <string_view>
#include <array>
#include <vector>

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

    using field_1 = struct { int i; };
    using field_2 = struct { int i; char c; };
    // NOLINTBEGIN(*-avoid-const-or-ref-data-members)
    using field_ref_1 = struct { int & i; };
    using field_ref_2 = struct { int & i; char && c; };
    using field_ref_3 = struct { int & i; char && c; char & cc; };
    // NOLINTEND(*-avoid-const-or-ref-data-members)

    struct field_3_nested {
        int i;
        field_1 f1;
        field_2 f2;
    };
    struct field_3_nested_tuplelike {
        std::tuple<int, char, std::string_view> tu;
        std::array<char, 3> a;
        std::pair<int, int> p;
    };
    struct field_4_nested_range {
        std::string_view sv;
        std::array<char, 3> a_c;
        std::array<int, 3> a_i;
        std::array<std::string_view, 3> a_sv;
        // other cx range types ...
    };
    struct field_5_nested_tuplelike_and_range {
        std::string_view sv;
        std::tuple<int, char, std::string_view> tu;
        std::array<char, 3> a;
        std::pair<int, int> p;
    };
    struct field_everything {
        field_3_nested f1{};
        field_3_nested_tuplelike f2{};
        field_4_nested_range f3{};
    };

    // BUG: struct two_fields_inheritance : two_fields { }; // decomposes into 2 elements, but only 1 name was provided
    // TODO(Guillaume): handle empty ?
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

    using all_t = std::tuple<A, B, C, D, E, F>;
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
