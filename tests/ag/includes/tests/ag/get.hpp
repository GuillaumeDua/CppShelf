#pragma once

#include <csl/ag.hpp>
#include "tests/type.hpp"

namespace test::ag::get_ {

    template <csl::ag::concepts::aggregate T>
    constexpr void ensure_symetric_get_type(){

        static_assert(std::is_reference_v<T>);
        using tuple_t = csl::ag::to_tuple_t<T>;

        [&]<
            typename ... Ts,
            std::size_t ... indexes
        >(
            std::type_identity<std::tuple<Ts...>>,
            std::index_sequence<indexes...>
        ){
            //if constexpr (not std::is_reference_v<T>)
            static_assert((std::same_as<
                csl::ag::details::mp::field_view_t<T, csl::ag::element_t<indexes, T>>,
                decltype(csl::ag::get<indexes>(std::declval<T>()))
            > and ...));

            // get<T> vs. get<std::size_t>
            static_assert((std::same_as<
                std::remove_cvref_t<decltype(csl::ag::get<indexes>(std::declval<T>()))>,
                std::remove_cvref_t<decltype(csl::ag::get<Ts>(std::declval<T>()))>
            > and ...));

            // tuple-like interface
            // csl::get<std::size_t> vs. std::get<std::size_t>
            static_assert((std::same_as<
                std::remove_cvref_t<decltype(std::get<indexes>(std::declval<T>()))>,
                std::remove_cvref_t<decltype(csl::ag::get<indexes>(std::declval<T>()))>
            > and ...));
            // csl::get<T> vs. std::get<T>
            static_assert((std::same_as<
                std::remove_cvref_t<decltype(std::get<Ts>(std::declval<T>()))>,
                std::remove_cvref_t<decltype(csl::ag::get<Ts>(std::declval<T>()))>
            > and ...));
            // std::get<std::size_t>(ag) vs. std::get<std::size_t>(tuple)
            static_assert((std::same_as<
                std::remove_cvref_t<decltype(std::get<indexes>(std::declval<T>()))>,
                std::remove_cvref_t<decltype(std::get<indexes>(std::declval<tuple_t>()))>
            > and ...));
            // std::get<T>(ag) vs. std::get<T>(tuple)
            static_assert((std::same_as<
                std::remove_cvref_t<decltype(std::get<Ts>(std::declval<T>()))>,
                std::remove_cvref_t<decltype(std::get<Ts>(std::declval<tuple_t>()))>
            > and ...));
        }(
            std::type_identity<csl::ag::to_tuple_t<std::remove_cvref_t<T>>>{},
            std::make_index_sequence<csl::ag::size_v<T>>{}
        );
    }

    template <csl::ag::concepts::aggregate ... Ts>
    constexpr auto ensure_symetric_get_types_with_cvref_matrix() -> bool {

        constexpr auto test = []<typename T>(){
            ensure_symetric_get_type<T&>();
            ensure_symetric_get_type<T&&>();
            ensure_symetric_get_type<const T&>();
            ensure_symetric_get_type<const T&&>();
        };
        ((test.template operator()<Ts>()), ...);
        return true;
    }

    constexpr auto _ = ensure_symetric_get_types_with_cvref_matrix<
        test::ag::types::aggregate_1,
        test::ag::types::aggregate_2,
        test::ag::types::aggregate_ref_1,
        test::ag::types::aggregate_ref_2,
        test::ag::types::aggregate_ref_3,
        test::ag::types::aggregate_all_cvref<int>
    >();

}