#pragma once

#include <csl/ag.hpp>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace test::ag::to_tuple_ {
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
        decltype(csl::ag::to_tuple(type{1,2}))
    >);
}