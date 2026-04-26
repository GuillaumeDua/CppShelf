#pragma once

#include <csl/ag.hpp>
#include "tests/types.hpp"

namespace test::ag::conversion::tuple_ {

    using type = test::ag::types::aggregate_all_cvref<int>;

    // csl::ag::to_tuple_t
    static_assert(std::same_as<      type::expected_to_tuple_t,     csl::ag::to_tuple_t<type>>);
    static_assert(std::same_as<      type::expected_to_tuple_t&,    csl::ag::to_tuple_t<type&>>);
    static_assert(std::same_as<      type::expected_to_tuple_t&&,   csl::ag::to_tuple_t<type&&>>);
    static_assert(std::same_as<const type::expected_to_tuple_t,     csl::ag::to_tuple_t<const type>>);
    static_assert(std::same_as<const type::expected_to_tuple_t&,    csl::ag::to_tuple_t<const type&>>);
    static_assert(std::same_as<const type::expected_to_tuple_t&&,   csl::ag::to_tuple_t<const type&&>>);

    // csl::ag::to_tuple (owning)
    static_assert(std::same_as<type::expected_to_tuple_t, decltype(csl::ag::to_tuple(std::declval<type>()))>);
    static_assert(std::same_as<type::expected_to_tuple_t, decltype(csl::ag::to_tuple(std::declval<type&>()))>);
    static_assert(std::same_as<type::expected_to_tuple_t, decltype(csl::ag::to_tuple(std::declval<type&&>()))>);
    static_assert(std::same_as<type::expected_to_tuple_t, decltype(csl::ag::to_tuple(std::declval<const type>()))>);
    static_assert(std::same_as<type::expected_to_tuple_t, decltype(csl::ag::to_tuple(std::declval<const type&>()))>);
    static_assert(std::same_as<type::expected_to_tuple_t, decltype(csl::ag::to_tuple(std::declval<const type&&>()))>);

    // so csl::ag::to_tuple_t same_as decltype(csl::ag::as_tuple)
}
