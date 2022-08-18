#pragma once

#include <csl/ag.hpp>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

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
        T v = {};
        const T c_v = {};
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    };

    using type = S<int>;

    consteval void make_field_view_() {
        constexpr auto test = [](auto && value) constexpr {

            auto && [ v, c_v, ref, c_ref, rref, c_rref ] = fwd(value);
            using T = decltype(value);

            // v
            static_assert(std::same_as<
                mp::apply_cvref_t<T, decltype(v)>,
                decltype(csl::ag::details::make_field_view<T, decltype(v)>(fwd(v)))
            >);
            static_assert(std::same_as<
                mp::field_view_t<T, decltype(v)>,
                decltype(csl::ag::details::make_field_view<T, decltype(v)>(fwd(v)))
            >);
            // c_v
            static_assert(std::same_as<
                mp::field_view_t<T, decltype(c_v)>,
                decltype(csl::ag::details::make_field_view<T, decltype(c_v)>(fwd(c_v)))
            >);

            // ref, c_ref, rref, c_rref
            static_assert(std::same_as<int&,        decltype(csl::ag::details::make_field_view<T, decltype(ref)>(ref))>);
            static_assert(std::same_as<const int&,  decltype(csl::ag::details::make_field_view<T, decltype(c_ref)>(c_ref))>);
            static_assert(std::same_as<int&&,       decltype(csl::ag::details::make_field_view<T, decltype(rref)>(std::move(rref)))>);
            static_assert(std::same_as<const int&&, decltype(csl::ag::details::make_field_view<T, decltype(c_rref)>(std::move(c_rref)))>);
        };
        auto value = type{};
        test(static_cast<      type&>(value));
        test(static_cast<      type&&>(value));
        test(static_cast<const type&>(value));
        test(static_cast<const type&&>(value));
    }
    consteval void make_tuple_view_() {

        constexpr auto create = [](auto && value) constexpr {
            auto && [ v, c_v, ref, c_ref, rref, c_rref ] = value;
            // return csl::ag::details::make_tuple_view<
            //     decltype(value),
            //     decltype(v), decltype(c_v), decltype(ref), decltype(c_ref), decltype(rref), decltype(c_rref)
            // >(
            //     fwd(v), fwd(c_v), fwd(ref), fwd(c_ref), fwd(rref), fwd(c_rref)
            // );
        };

        // static_assert(std::same_as<
        //     std::tuple<int&, const int&, int&, const int&, int&&, const int&&>,
        //     decltype(create(std::declval<type&>()))
        // >);
        // static_assert(std::same_as<
        //     std::tuple<int&&, const int&&, int&, const int&, int&&, const int&&>,
        //     decltype(create(std::declval<type&&>()))
        // >);
        // static_assert(std::same_as<
        //     std::tuple<const int&, const int&, int&, const int&, int&&, const int&&>,
        //     decltype(create(std::declval<const type&>()))
        // >);
        // static_assert(std::same_as<
        //     std::tuple<const int&&, const int&&, int&, const int&, int&&, const int&&>,
        //     decltype(create(std::declval<const type&&>()))
        // >);
    }
}

#undef fwd