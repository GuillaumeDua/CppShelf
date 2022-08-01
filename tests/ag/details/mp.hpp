#pragma once

#include <csl/ag.hpp>

namespace test::ag::details::mp {
    static_assert(std::same_as<int,         csl::ag::details::mp::apply_ref<int, int &>::type>);
    static_assert(std::same_as<int,         csl::ag::details::mp::apply_ref<int, int &&>::type>);
    static_assert(std::same_as<const int,   csl::ag::details::mp::apply_ref<int, const int &>::type>);
    static_assert(std::same_as<const int,   csl::ag::details::mp::apply_ref<int, const int &&>::type>);

    static_assert(std::same_as<int&,        csl::ag::details::mp::apply_ref<int&, int &>::type>);
    static_assert(std::same_as<int&,        csl::ag::details::mp::apply_ref<int&, int &&>::type>);
    static_assert(std::same_as<const int&,  csl::ag::details::mp::apply_ref<int&, const int &>::type>);
    static_assert(std::same_as<const int&,  csl::ag::details::mp::apply_ref<int&, const int &&>::type>);

    static_assert(std::same_as<int&&,       csl::ag::details::mp::apply_ref<int&&, int &>::type>);
    static_assert(std::same_as<int&&,       csl::ag::details::mp::apply_ref<int&&, int &&>::type>);
    static_assert(std::same_as<const int&&, csl::ag::details::mp::apply_ref<int&&, const int &>::type>);
    static_assert(std::same_as<const int&&, csl::ag::details::mp::apply_ref<int&&, const int &&>::type>);
}