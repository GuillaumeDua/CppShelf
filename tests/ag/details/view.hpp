#pragma once

#include <csl/ag.hpp>

namespace mp = csl::ag::details::mp;

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
    using as_tuple_fields_view_t = typename std::tuple<
        mp::field_view_t<T, decltype(std::declval<T>().v)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_v)>,
        mp::field_view_t<T, decltype(std::declval<T>().ref)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_ref)>,
        mp::field_view_t<T, decltype(std::declval<T>().rref)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_rref)>
    >;

    using type = S<int>;
    static_assert(std::same_as<as_tuple_fields_view_t<type&>,
        std::tuple<int&, const int&, int&, const int&, int&&, const int&&>
    >);
    static_assert(std::same_as<as_tuple_fields_view_t<const type&>,
        std::tuple<const int&, const int&, int&, const int&, int&&, const int&&>
    >);
    static_assert(std::same_as<as_tuple_fields_view_t<type&&>,
        std::tuple<int&&, const int&&, int&, const int&, int&&, const int&&>
    >);
    static_assert(std::same_as<as_tuple_fields_view_t<const type&&>,
        std::tuple<const int&&, const int&&, int&, const int&, int&&, const int&&>
    >);
}
namespace test::ag::details {
    template <typename T>
    struct S {
        T v;
        const T c_v;
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    };

    using type = S<int>;

    consteval void make_field_view_() {
        static_assert(std::same_as<int&,        decltype(csl::ag::details::make_field_view<type&>(std::declval<int&>()))>);
        static_assert(std::same_as<int&&,       decltype(csl::ag::details::make_field_view<type&>(std::declval<int&&>()))>);
        static_assert(std::same_as<const int&,  decltype(csl::ag::details::make_field_view<type&>(std::declval<const int&>()))>);
        static_assert(std::same_as<const int&&, decltype(csl::ag::details::make_field_view<type&>(std::declval<const int&&>()))>);

        
    }
    consteval void make_tuple_view_() {

    }
}
