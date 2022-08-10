#pragma once

#include <csl/ag.hpp>

namespace test::ag::details::mp::apply_ref_ {
    static_assert(std::same_as<int,         csl::ag::details::mp::apply_ref_t<int, int &>>);
    static_assert(std::same_as<int,         csl::ag::details::mp::apply_ref_t<int, int &&>>);
    static_assert(std::same_as<const int,   csl::ag::details::mp::apply_ref_t<int, const int &>>);
    static_assert(std::same_as<const int,   csl::ag::details::mp::apply_ref_t<int, const int &&>>);

    static_assert(std::same_as<int&,        csl::ag::details::mp::apply_ref_t<int&, int &>>);
    static_assert(std::same_as<int&,        csl::ag::details::mp::apply_ref_t<int&, int &&>>);
    static_assert(std::same_as<const int&,  csl::ag::details::mp::apply_ref_t<int&, const int &>>);
    static_assert(std::same_as<const int&,  csl::ag::details::mp::apply_ref_t<int&, const int &&>>);

    static_assert(std::same_as<int&&,       csl::ag::details::mp::apply_ref_t<int&&, int &>>);
    static_assert(std::same_as<int&&,       csl::ag::details::mp::apply_ref_t<int&&, int &&>>);
    static_assert(std::same_as<const int&&, csl::ag::details::mp::apply_ref_t<int&&, const int &>>);
    static_assert(std::same_as<const int&&, csl::ag::details::mp::apply_ref_t<int&&, const int &&>>);
}
namespace test::ag::details::mp::add_const_ {
    static_assert(std::same_as<const int,               csl::ag::details::mp::add_const_t<int>>);
    static_assert(std::same_as<const int&,              csl::ag::details::mp::add_const_t<int&>>);
    static_assert(std::same_as<const int&&,             csl::ag::details::mp::add_const_t<int&&>>);

    static_assert(std::same_as<const int,               csl::ag::details::mp::add_const_t<const int>>);
    static_assert(std::same_as<const int&,              csl::ag::details::mp::add_const_t<const int&>>);
    static_assert(std::same_as<const int&&,             csl::ag::details::mp::add_const_t<const int&&>>);

    static_assert(std::same_as<const volatile int,      csl::ag::details::mp::add_const_t<volatile int>>);
    static_assert(std::same_as<const volatile int&,     csl::ag::details::mp::add_const_t<volatile int&>>);
    static_assert(std::same_as<const volatile int&&,    csl::ag::details::mp::add_const_t<volatile int&&>>);
}
namespace test::ag::details::mp::add_volatile_ {
    static_assert(std::same_as<volatile int,   csl::ag::details::mp::add_volatile_t<int>>);
    static_assert(std::same_as<volatile int&,  csl::ag::details::mp::add_volatile_t<int&>>);
    static_assert(std::same_as<volatile int&&, csl::ag::details::mp::add_volatile_t<int&&>>);

    static_assert(std::same_as<volatile int,   csl::ag::details::mp::add_volatile_t<volatile int>>);
    static_assert(std::same_as<volatile int&,  csl::ag::details::mp::add_volatile_t<volatile int&>>);
    static_assert(std::same_as<volatile int&&, csl::ag::details::mp::add_volatile_t<volatile int&&>>);

    static_assert(std::same_as<const volatile int,   csl::ag::details::mp::add_volatile_t<const int>>);
    static_assert(std::same_as<const volatile int&,  csl::ag::details::mp::add_volatile_t<const int&>>);
    static_assert(std::same_as<const volatile int&&, csl::ag::details::mp::add_volatile_t<const int&&>>);
}
namespace test::ag::details::mp::add_cv_ {
    static_assert(std::same_as<const volatile int,   csl::ag::details::mp::add_cv_t<int>>);
    static_assert(std::same_as<const volatile int&,  csl::ag::details::mp::add_cv_t<int&>>);
    static_assert(std::same_as<const volatile int&&, csl::ag::details::mp::add_cv_t<int&&>>);

    static_assert(std::same_as<const volatile int,   csl::ag::details::mp::add_cv_t<const int>>);
    static_assert(std::same_as<const volatile int&,  csl::ag::details::mp::add_cv_t<const int&>>);
    static_assert(std::same_as<const volatile int&&, csl::ag::details::mp::add_cv_t<const int&&>>);

    static_assert(std::same_as<const volatile int,   csl::ag::details::mp::add_cv_t<volatile int>>);
    static_assert(std::same_as<const volatile int&,  csl::ag::details::mp::add_cv_t<volatile int&>>);
    static_assert(std::same_as<const volatile int&&, csl::ag::details::mp::add_cv_t<volatile int&&>>);

    static_assert(std::same_as<const volatile int,   csl::ag::details::mp::add_cv_t<const volatile int>>);
    static_assert(std::same_as<const volatile int&,  csl::ag::details::mp::add_cv_t<const volatile int&>>);
    static_assert(std::same_as<const volatile int&&, csl::ag::details::mp::add_cv_t<const volatile int&&>>);
}