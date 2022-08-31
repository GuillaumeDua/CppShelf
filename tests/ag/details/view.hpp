#pragma once

#include <csl/ag.hpp>

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

namespace mp = csl::ag::details::mp;

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
                mp::copy_cvref_t<T, decltype(v)>,
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

    // Helper function for test purpose
    template <typename owner, typename ... Ts>
    constexpr auto get_view_type(Ts&& ... values) {
        using view_type = std::tuple<mp::field_view_t<owner, Ts>...>;
        return std::type_identity<view_type>{};
    }

    consteval void make_tuple_view_() {

        constexpr auto create_view_type = [](auto && value) constexpr {
            auto && [ v, c_v, ref, c_ref, rref, c_rref ] = value;
            return get_view_type<
                decltype(value),
                decltype(v), decltype(c_v), decltype(ref), decltype(c_ref), decltype(rref), decltype(c_rref)
            >(
                fwd(v), fwd(c_v), fwd(ref), fwd(c_ref), fwd(rref), fwd(c_rref)
            );
        };

        type value;
        static_assert(std::same_as<
            decltype(create_view_type(std::declval<type&>()))::type,
            std::tuple<int&, const int&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<
            decltype(create_view_type(std::declval<type&&>()))::type,
            std::tuple<int&&, const int&&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<
            decltype(create_view_type(std::declval<const type&>()))::type,
            std::tuple<const int&, const int&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<
            decltype(create_view_type(std::declval<const type&&>()))::type,
            std::tuple<const int&&, const int&&, int&, const int&, int&&, const int&&>
        >);

        constexpr auto create = [](auto && value) constexpr {
            auto && [ v, c_v, ref, c_ref, rref, c_rref ] = value;
            return csl::ag::details::make_tuple_view<
                decltype(value),
                decltype(v), decltype(c_v), decltype(ref), decltype(c_ref), decltype(rref), decltype(c_rref)
            >(
                fwd(v), fwd(c_v), fwd(ref), fwd(c_ref), fwd(rref), fwd(c_rref)
            );
        };
        static_assert(std::same_as<decltype(create(std::declval<type&>())),
            std::tuple<int&, const int&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<decltype(create(std::declval<type&&>())),
            std::tuple<int&&, const int&&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<decltype(create(std::declval<const type&>())),
            std::tuple<const int&, const int&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<decltype(create(std::declval<const type&&>())),
            std::tuple<const int&&, const int&&, int&, const int&, int&&, const int&&>
        >);
    }
}
namespace test::ag::details::mp_::field_view_ {

    template <typename T>
    using as_tuple_fields_view_t = typename std::tuple<
        mp::field_view_t<T, decltype(std::declval<T>().v)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_v)>,
        mp::field_view_t<T, decltype(std::declval<T>().ref)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_ref)>,
        mp::field_view_t<T, decltype(std::declval<T>().rref)>,
        mp::field_view_t<T, decltype(std::declval<T>().c_rref)>
    >;

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
// TODO(Guss) : move to tests/ag/public_interface.hpp ?
namespace test::ag::details::as_tuple_view {
    constexpr void check_type() {
        // types
        static_assert(std::same_as<decltype(csl::ag::as_tuple_view(std::declval<type&>())),
            std::tuple<int&, const int&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<decltype(csl::ag::as_tuple_view(std::declval<type&&>())),
            std::tuple<int&&, const int&&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<decltype(csl::ag::as_tuple_view(std::declval<const type&>())),
            std::tuple<const int&, const int&, int&, const int&, int&&, const int&&>
        >);
        static_assert(std::same_as<decltype(csl::ag::as_tuple_view(std::declval<const type&&>())),
            std::tuple<const int&&, const int&&, int&, const int&, int&&, const int&&>
        >);
    }
    constexpr void check_field_values() {
        // values (`assert(false)` produce a)
        constexpr auto check = [](auto && value) constexpr {
            auto view = csl::ag::as_tuple_view(fwd(value));
            if (
                std::addressof(std::get<0>(view)) not_eq std::addressof(value.v) or
                std::addressof(std::get<1>(view)) not_eq std::addressof(value.c_v) or
                std::addressof(std::get<2>(view)) not_eq std::addressof(value.ref) or
                std::addressof(std::get<3>(view)) not_eq std::addressof(value.c_ref) or
                std::addressof(std::get<4>(view)) not_eq std::addressof(value.rref) or
                std::addressof(std::get<5>(view)) not_eq std::addressof(value.c_rref)
            ) throw std::runtime_error{ "test::ag::details::as_tuple_view" };
        };

        auto value = type{ 1, 2 };
        check(value);
        check(std::as_const(value));
        check(std::move(value));
        check(std::move(std::as_const(value)));
    }
}
namespace test::ag::details::view_element_ {
    // consteval void check_fields_view_types() {
    //     static constexpr auto check = []<typename T>(){
    //         []<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //             static_assert((
    //                 std::is_reference_v<csl::ag::view_element_t<indexes, T>> and ...
    //             ));
    //         }(std::make_index_sequence<csl::ag::size_v<T>>{});
    //     };
    //     check.template operator()<type&>();
    //     check.template operator()<type&&>();
    //     check.template operator()<const type&>();
    //     check.template operator()<const type&&>();
    // }
    // consteval void check_type_correctness() {
    //     constexpr auto check = []<typename T>(){
    //         []<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //             static_assert(std::same_as<
    //                 csl::ag::tuple_view_t<T>,
    //                 std::tuple<
    //                     csl::ag::view_element_t<indexes, T>...
    //                 >
    //             >);
    //         }(std::make_index_sequence<csl::ag::size_v<T>>{});
    //     };
    //     check.template operator()<type&>();
    //     check.template operator()<type&&>();
    //     check.template operator()<const type&>();
    //     check.template operator()<const type&&>();
    // }
}

#undef fwd