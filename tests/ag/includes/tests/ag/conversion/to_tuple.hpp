#pragma once

#include <csl/ag.hpp>
#include "tests/type.hpp"

namespace test::ag::conversion::tuple_ {

    using type = test::ag::type<int>;
    using expected_tuple_type = std::tuple<int, int &, int &&, const int, const int &, const int &&>;

    // csl::ag::to_tuple_t
    static_assert(std::same_as<expected_tuple_type, csl::ag::to_tuple_t<type>>);
    static_assert(std::same_as<expected_tuple_type, csl::ag::to_tuple_t<type&>>);
    static_assert(std::same_as<expected_tuple_type, csl::ag::to_tuple_t<type&&>>);
    static_assert(std::same_as<expected_tuple_type, csl::ag::to_tuple_t<const type>>);
    static_assert(std::same_as<expected_tuple_type, csl::ag::to_tuple_t<const type&>>);
    static_assert(std::same_as<expected_tuple_type, csl::ag::to_tuple_t<const type&&>>);

    // csl::ag::to_tuple
    static_assert(std::same_as<expected_tuple_type, decltype(csl::ag::to_tuple(std::declval<type>()))>);
    static_assert(std::same_as<expected_tuple_type, decltype(csl::ag::to_tuple(std::declval<type&>()))>);
    static_assert(std::same_as<expected_tuple_type, decltype(csl::ag::to_tuple(std::declval<type&&>()))>);
    static_assert(std::same_as<expected_tuple_type, decltype(csl::ag::to_tuple(std::declval<const type>()))>);
    static_assert(std::same_as<expected_tuple_type, decltype(csl::ag::to_tuple(std::declval<const type&>()))>);
    static_assert(std::same_as<expected_tuple_type, decltype(csl::ag::to_tuple(std::declval<const type&&>()))>);

    // so csl::ag::to_tuple_t same_as decltype(csl::ag::as_tuple)
}
