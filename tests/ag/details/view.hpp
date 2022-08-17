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
        constexpr auto check_expected_field_type = [](auto && value) constexpr {

            auto && [ v, c_v, ref, c_ref, rref, c_rref ] = std::forward<decltype(value)>(value);
            using T = decltype(value);

            // v
            static_assert(std::same_as<
                mp::apply_cvref_t<T, decltype(v)>,
                decltype(csl::ag::details::make_field_view<T>(v))
            >);
            static_assert(std::same_as<
                mp::field_view_t<T, decltype(v)>,
                decltype(csl::ag::details::make_field_view<T, decltype(v)>(v))
            >);
            // c_v
            static_assert(std::same_as<
                mp::field_view_t<T, decltype(c_v)>,
                decltype(csl::ag::details::make_field_view<T, decltype(c_v)>(c_v))
            >);

            // ref, c_ref, rref, c_rref
            static_assert(std::same_as<int&,        decltype(csl::ag::details::make_field_view<T, decltype(ref)>(ref))>);
            static_assert(std::same_as<const int&,  decltype(csl::ag::details::make_field_view<T, decltype(c_ref)>(c_ref))>);
            static_assert(std::same_as<int&&,       decltype(csl::ag::details::make_field_view<T, decltype(rref)>(std::move(rref)))>);
            static_assert(std::same_as<const int&&, decltype(csl::ag::details::make_field_view<T, decltype(c_rref)>(std::move(c_rref)))>);
        };
        auto value = type{};
        check_expected_field_type(static_cast<      type&>(value));
        check_expected_field_type(static_cast<      type&&>(value));
        // check_expected_field_type(static_cast<const type&>(value));
        // check_expected_field_type(static_cast<const type&&>(value));

        // int value = {};
        // static_assert(std::addressof(value) == std::addressof(csl::ag::details::make_field_view<type&>(value)));
        // static_assert(std::addressof(value) == std::addressof(csl::ag::details::make_field_view<type&, const int&>(value)));
    }
    consteval void make_tuple_view_() {
        // static_assert(std::same_as<
        //     decltype(csl::ag::details::make_tuple_view<type&>(std::declval<>()),
        //     std::tuple<int&, const int&, int&, const int&, int&&, const int&&>
        // >);
        // static_assert(std::same_as<
        //     decltype(csl::ag::details::make_tuple_view(std::declval<type&&>())),
        //     std::tuple<int&&, const int&&, int&, const int&, int&&, const int&&>
        // >);
        // static_assert(std::same_as<
        //     decltype(csl::ag::details::make_tuple_view(std::declval<const type&>())),
        //     std::tuple<const int&, const int&, int&, const int&, int&&, const int&&>
        // >);
        // static_assert(std::same_as<
        //     decltype(csl::ag::details::make_tuple_view(std::declval<const type&&>())),
        //     std::tuple<const int&&, const int&&, int&, const int&, int&&, const int&&>
        // >);
    }
}
