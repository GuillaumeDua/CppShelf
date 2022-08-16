#pragma once

#include <csl/ag.hpp>

namespace mp = csl::ag::details::mp;

namespace test::ag::details::mp_::apply_ref_ {
    static_assert(std::same_as<int,         mp::apply_ref_t<int, int &>>);
    static_assert(std::same_as<int,         mp::apply_ref_t<int, int &&>>);
    static_assert(std::same_as<const int,   mp::apply_ref_t<int, const int &>>);
    static_assert(std::same_as<const int,   mp::apply_ref_t<int, const int &&>>);

    static_assert(std::same_as<int&,        mp::apply_ref_t<int&, int &>>);
    static_assert(std::same_as<int&,        mp::apply_ref_t<int&, int &&>>);
    static_assert(std::same_as<const int&,  mp::apply_ref_t<int&, const int &>>);
    static_assert(std::same_as<const int&,  mp::apply_ref_t<int&, const int &&>>);

    static_assert(std::same_as<int&&,       mp::apply_ref_t<int&&, int &>>);
    static_assert(std::same_as<int&&,       mp::apply_ref_t<int&&, int &&>>);
    static_assert(std::same_as<const int&&, mp::apply_ref_t<int&&, const int &>>);
    static_assert(std::same_as<const int&&, mp::apply_ref_t<int&&, const int &&>>);
}
namespace test::ag::details::mp_::add_const_ {
    static_assert(std::same_as<const int,               mp::add_const_t<int>>);
    static_assert(std::same_as<const int&,              mp::add_const_t<int&>>);
    static_assert(std::same_as<const int&&,             mp::add_const_t<int&&>>);

    static_assert(std::same_as<const int,               mp::add_const_t<const int>>);
    static_assert(std::same_as<const int&,              mp::add_const_t<const int&>>);
    static_assert(std::same_as<const int&&,             mp::add_const_t<const int&&>>);

    static_assert(std::same_as<const volatile int,      mp::add_const_t<volatile int>>);
    static_assert(std::same_as<const volatile int&,     mp::add_const_t<volatile int&>>);
    static_assert(std::same_as<const volatile int&&,    mp::add_const_t<volatile int&&>>);
}
namespace test::ag::details::mp_::add_volatile_ {
    static_assert(std::same_as<volatile int,   mp::add_volatile_t<int>>);
    static_assert(std::same_as<volatile int&,  mp::add_volatile_t<int&>>);
    static_assert(std::same_as<volatile int&&, mp::add_volatile_t<int&&>>);

    static_assert(std::same_as<volatile int,   mp::add_volatile_t<volatile int>>);
    static_assert(std::same_as<volatile int&,  mp::add_volatile_t<volatile int&>>);
    static_assert(std::same_as<volatile int&&, mp::add_volatile_t<volatile int&&>>);

    static_assert(std::same_as<const volatile int,   mp::add_volatile_t<const int>>);
    static_assert(std::same_as<const volatile int&,  mp::add_volatile_t<const int&>>);
    static_assert(std::same_as<const volatile int&&, mp::add_volatile_t<const int&&>>);
}
namespace test::ag::details::mp_::add_cv_ {
    static_assert(std::same_as<const volatile int,   mp::add_cv_t<int>>);
    static_assert(std::same_as<const volatile int&,  mp::add_cv_t<int&>>);
    static_assert(std::same_as<const volatile int&&, mp::add_cv_t<int&&>>);

    static_assert(std::same_as<const volatile int,   mp::add_cv_t<const int>>);
    static_assert(std::same_as<const volatile int&,  mp::add_cv_t<const int&>>);
    static_assert(std::same_as<const volatile int&&, mp::add_cv_t<const int&&>>);

    static_assert(std::same_as<const volatile int,   mp::add_cv_t<volatile int>>);
    static_assert(std::same_as<const volatile int&,  mp::add_cv_t<volatile int&>>);
    static_assert(std::same_as<const volatile int&&, mp::add_cv_t<volatile int&&>>);

    static_assert(std::same_as<const volatile int,   mp::add_cv_t<const volatile int>>);
    static_assert(std::same_as<const volatile int&,  mp::add_cv_t<const volatile int&>>);
    static_assert(std::same_as<const volatile int&&, mp::add_cv_t<const volatile int&&>>);
}
namespace test::ag::details::mp_::apply_cv_ {
    struct A{};
    static_assert(std::same_as<int,                     mp::apply_cv_t<A, int>>);
    static_assert(std::same_as<const int,               mp::apply_cv_t<const A, int>>);
    static_assert(std::same_as<const int&,              mp::apply_cv_t<const A, int&>>);
    static_assert(std::same_as<const int&&,             mp::apply_cv_t<const A, int&&>>);
    static_assert(std::same_as<volatile int,            mp::apply_cv_t<volatile A, int>>);
    static_assert(std::same_as<volatile int&,           mp::apply_cv_t<volatile A, int&>>);
    static_assert(std::same_as<volatile int&&,          mp::apply_cv_t<volatile A, int&&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cv_t<const volatile A, int>>);
    static_assert(std::same_as<const volatile int&,     mp::apply_cv_t<const volatile A, int&>>);
    static_assert(std::same_as<const volatile int&&,    mp::apply_cv_t<const volatile A, int&&>>);

    static_assert(std::same_as<int,                     mp::apply_cv_t<A&, int>>);
    static_assert(std::same_as<const int,               mp::apply_cv_t<const A&, int>>);
    static_assert(std::same_as<const int&,              mp::apply_cv_t<const A&, int&>>);
    static_assert(std::same_as<const int&&,             mp::apply_cv_t<const A&, int&&>>);
    static_assert(std::same_as<volatile int,            mp::apply_cv_t<volatile A&, int>>);
    static_assert(std::same_as<volatile int&,           mp::apply_cv_t<volatile A&, int&>>);
    static_assert(std::same_as<volatile int&&,          mp::apply_cv_t<volatile A&, int&&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cv_t<const volatile A&, int>>);
    static_assert(std::same_as<const volatile int&,     mp::apply_cv_t<const volatile A&, int&>>);
    static_assert(std::same_as<const volatile int&&,    mp::apply_cv_t<const volatile A&, int&&>>);

    static_assert(std::same_as<int,                     mp::apply_cv_t<A&&, int>>);
    static_assert(std::same_as<const int,               mp::apply_cv_t<const A&&, int>>);
    static_assert(std::same_as<const int&,              mp::apply_cv_t<const A&&, int&>>);
    static_assert(std::same_as<const int&&,             mp::apply_cv_t<const A&&, int&&>>);
    static_assert(std::same_as<volatile int,            mp::apply_cv_t<volatile A&&, int>>);
    static_assert(std::same_as<volatile int&,           mp::apply_cv_t<volatile A&&, int&>>);
    static_assert(std::same_as<volatile int&&,          mp::apply_cv_t<volatile A&&, int&&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cv_t<const volatile A&&, int>>);
    static_assert(std::same_as<const volatile int&,     mp::apply_cv_t<const volatile A&&, int&>>);
    static_assert(std::same_as<const volatile int&&,    mp::apply_cv_t<const volatile A&&, int&&>>);
}
namespace test::ag::details::mp_::apply_cvref_ {
    struct A{};

    static_assert(std::same_as<int,                     mp::apply_cvref_t<A, int>>);

    static_assert(std::same_as<const int,               mp::apply_cvref_t<const A, int>>);
    static_assert(std::same_as<const int,               mp::apply_cvref_t<const A, int&>>);
    static_assert(std::same_as<const int,               mp::apply_cvref_t<const A, int&&>>);
    static_assert(std::same_as<volatile int,            mp::apply_cvref_t<volatile A, int>>);
    static_assert(std::same_as<volatile int,            mp::apply_cvref_t<volatile A, int&>>);
    static_assert(std::same_as<volatile int,            mp::apply_cvref_t<volatile A, int&&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cvref_t<const volatile A, int>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cvref_t<const volatile A, int&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cvref_t<const volatile A, int&&>>);

    static_assert(std::same_as<const int &,             mp::apply_cvref_t<const A&, int>>);
    static_assert(std::same_as<const int &,             mp::apply_cvref_t<const A&, int&>>);
    static_assert(std::same_as<const int &,             mp::apply_cvref_t<const A&, int&&>>);
    static_assert(std::same_as<const int &&,            mp::apply_cvref_t<const A&&, int>>);
    static_assert(std::same_as<const int &&,            mp::apply_cvref_t<const A&&, int&>>);
    static_assert(std::same_as<const int &&,            mp::apply_cvref_t<const A&&, int&&>>);

    static_assert(std::same_as<volatile int,            mp::apply_cvref_t<volatile A, int>>);
    static_assert(std::same_as<volatile int,            mp::apply_cvref_t<volatile A, int&>>);
    static_assert(std::same_as<volatile int,            mp::apply_cvref_t<volatile A, int&&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cvref_t<const volatile A, int>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cvref_t<const volatile A, int&>>);
    static_assert(std::same_as<const volatile int,      mp::apply_cvref_t<const volatile A, int&&>>);
}
