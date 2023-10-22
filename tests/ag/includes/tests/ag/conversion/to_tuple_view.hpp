#pragma once

#include <csl/ag.hpp>
#include "tests/type.hpp"

namespace test::ag::conversion::tuple_view_ {

    using type = test::ag::types::aggregate_all_cvref<int>;

     // clang-format off
    using expected_from_lvalue       = std::tuple<      int&,   int &, int &&, const int&,  const int &, const int &&>;
    using expected_from_rvalue       = std::tuple<      int &&, int &, int &&, const int&&, const int &, const int &&>;
    using expected_from_const_lvalue = std::tuple<const int &,  int &, int &&, const int&,  const int &, const int &&>;
    using expected_from_const_rvalue = std::tuple<const int &&, int &, int &&, const int&&, const int &, const int &&>;
     // clang-format on

    // csl::ag::view_t
    static_assert(std::same_as<expected_from_lvalue, csl::ag::view_t<type&>>);
    static_assert(std::same_as<expected_from_rvalue, csl::ag::view_t<type&&>>);
    static_assert(std::same_as<expected_from_const_lvalue, csl::ag::view_t<const type&>>);
    static_assert(std::same_as<expected_from_const_rvalue, csl::ag::view_t<const type&&>>);

    // csl::ag::to_tuple_view
    static_assert(std::same_as<expected_from_lvalue, std::remove_cvref_t<decltype(csl::ag::to_tuple_view(std::declval<type&>()))>>);
    static_assert(std::same_as<expected_from_rvalue, std::remove_cvref_t<decltype(csl::ag::to_tuple_view(std::declval<type&&>()))>>);
    static_assert(std::same_as<expected_from_const_lvalue, std::remove_cvref_t<decltype(csl::ag::to_tuple_view(std::declval<const type&>()))>>);
    static_assert(std::same_as<expected_from_const_rvalue, std::remove_cvref_t<decltype(csl::ag::to_tuple_view(std::declval<const type&&>()))>>);

    // csl::ag::views::tuplelike
    // static_assert(std::same_as<expected_from_lvalue, std::remove_cvref_t<decltype(std::declval<type&>() | csl::ag::views::all)>>);

    // so csl::ag::view_t same_as decltype(csl::ag::to_tuple_view())
    // (nothing to do)
}
