#pragma once

#include <csl/ag.hpp>

namespace test::ag::conversion::tuple_ {

    template <typename T>
    struct A {
        T           v0;
        T &         v1;
        T &&        v2;
        const T     v3;
        const T &   v4;
        const T &&  v5;
    };
    using type = A<int>;

    using expected_tuple_type = std::tuple<int, int &, int &&, const int, const int &, const int &&>;
    static_assert(std::same_as<csl::ag::to_tuple_t<type>, expected_tuple_type>);
    static_assert(std::same_as<csl::ag::to_tuple_t<type&>, expected_tuple_type>);
    static_assert(std::same_as<csl::ag::to_tuple_t<type&&>, expected_tuple_type>);
    static_assert(std::same_as<csl::ag::to_tuple_t<const type>, expected_tuple_type>);
    static_assert(std::same_as<csl::ag::to_tuple_t<const type&>, expected_tuple_type>);
    static_assert(std::same_as<csl::ag::to_tuple_t<const type&&>, expected_tuple_type>);

    static_assert(std::same_as<
        csl::ag::to_tuple_t<type>,
        decltype(csl::ag::as_tuple(std::declval<type>()))
    >);
}
