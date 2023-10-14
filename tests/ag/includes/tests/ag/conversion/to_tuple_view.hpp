#pragma once

#include <csl/ag.hpp>
#include "tests/type.hpp"

namespace test::ag::conversion::tuple_view_ {

    using type = test::ag::type<int>;

    // csl::ag::view_t
    static_assert(std::same_as<
        csl::ag::view_t<type&>,
        std::tuple<int&, int &, int &&, const int&, const int &, const int &&>
    >);
    static_assert(std::same_as<
        csl::ag::view_t<type&&>,
        std::tuple<int&&, int &, int &&, const int&&, const int &, const int &&>
    >);
    static_assert(std::same_as<
        csl::ag::view_t<const type&>,
        std::tuple<const int&, int &, int &&, const int&, const int &, const int &&>
    >);
    static_assert(std::same_as<
        csl::ag::view_t<const type&&>,
        std::tuple<const int&&, int &, int &&, const int&&, const int &, const int &&>
    >);
}
