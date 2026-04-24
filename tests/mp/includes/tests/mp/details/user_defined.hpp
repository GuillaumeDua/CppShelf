#pragma once

#include <tuple>

namespace test::tuples::user_defined {
    struct well_formed {
        int a; float b; double c;
    };
    struct ill_formed {
        int a; float b;
    };
}

// test::tuples::user_defined::ill_formed
template <> struct std::tuple_size<test::tuples::user_defined::ill_formed>
    : std::integral_constant<std::size_t, 2>
{};
template <> struct std::tuple_element<0, test::tuples::user_defined::ill_formed> { using type = int; };

// test::tuples::user_defined::well_formed
template <> struct std::tuple_size<test::tuples::user_defined::well_formed>
    : std::integral_constant<std::size_t, 3>
{};

template <> struct std::tuple_element<0, test::tuples::user_defined::well_formed> { using type = int; };
template <> struct std::tuple_element<1, test::tuples::user_defined::well_formed> { using type = float; };
template <> struct std::tuple_element<2, test::tuples::user_defined::well_formed> { using type = double; };

namespace test::tuples::user_defined {

template <std::size_t I> auto get(test::tuples::user_defined::well_formed & value) noexcept 
-> std::tuple_element_t<I, test::tuples::user_defined::well_formed> &
{
    if constexpr (I == 0)       return value.a;
    else if constexpr (I == 1)  return value.b;
    else if constexpr (I == 2)  return value.c;
    else static_assert(false, "invalid index: out of bound");
}
template <std::size_t I> auto get(const test::tuples::user_defined::well_formed & value) noexcept
-> const std::tuple_element_t<I, test::tuples::user_defined::well_formed> &
{
    if constexpr (I == 0)       return value.a;
    else if constexpr (I == 1)  return value.b;
    else if constexpr (I == 2)  return value.c;
    else static_assert(false, "invalid index: out of bound");
}
template <std::size_t I> auto get(test::tuples::user_defined::well_formed && value) noexcept
-> std::tuple_element_t<I, test::tuples::user_defined::well_formed> &&
{
    if constexpr (I == 0)       return std::move(value.a);
    else if constexpr (I == 1)  return std::move(value.b);
    else if constexpr (I == 2)  return std::move(value.c);
    else static_assert(false, "invalid index: out of bound");
}
} // namespace test::tuples::user_defined

#include <csl/mp.hpp>

static_assert(csl::mp::concepts::tuple_like<test::tuples::user_defined::well_formed>);
static_assert(not csl::mp::concepts::tuple_like<test::tuples::user_defined::ill_formed>);