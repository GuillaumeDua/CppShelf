#pragma once

#include <csl/ag.hpp>

namespace test::ag::conversion::tuple_view_ {
    template <typename T>
    struct S {
        T v = {};
        const T c_v = {};
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    };
    using type = S<int>;

    static_assert(std::same_as<
        csl::ag::to_tuple_t<type>,
        std::tuple<int, const int, int&, const int&, int&&, const int&&>
    >);
    static_assert(std::same_as<
        csl::ag::to_tuple_t<type>,
        decltype(csl::ag::as_tuple(type{1,2}))
    >);
}
