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
namespace test::ag::details::mp_::field_view_ {
    template <typename T>
    struct S {
        T v;
        const T c_v;
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    };

    template <typename T>
    using as_tuple_fields_view_t = std::tuple<
        mp::field_view_t<T, decltype(std::declval<T>().v)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_v)>,
        mp::field_view_t<T, decltype(std::declval<T>().ref)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_ref)>,
        mp::field_view_t<T, decltype(std::declval<T>().rref)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_rref)>
    >;

}