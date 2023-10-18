#pragma once

#include <csl/ag.hpp>

namespace test::ag::get_::types {

    using aggregate_1 = struct { int i; };
    using aggregate_2 = struct { int i; char c; };

    using aggregate_ref_1 = struct { int & i; };
    using aggregate_ref_2 = struct { int & i; char && c; };
    using aggregate_ref_3 = struct { int & i; char && c; char & cc; };
    
    template <typename T>
    struct aggregate_all_cvref {
    // NOLINTBEGIN(*-avoid-const-or-ref-data-members)
        T v;
        const T c_v;
        T & ref = v;
        const T & c_ref = c_v;
        T && rref = std::move(v);
        const T && c_rref = std::move(c_v);
    // NOLINTEND(*-avoid-const-or-ref-data-members)
    };
}
namespace test::ag::get_ {

    template <csl::ag::concepts::aggregate T>
    constexpr void ensure_symetric_get_type(){

        static_assert(std::is_reference_v<T>);

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

            // and same as tuple
        }(
            std::type_identity<csl::ag::to_tuple_t<T>>{},
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
        types::aggregate_1,
        types::aggregate_2,
        types::aggregate_ref_1,
        types::aggregate_ref_2,
        types::aggregate_ref_3,
        types::aggregate_all_cvref<int>
    >();

}