#pragma once

#include <csl/ag.hpp>
#include "tests/types.hpp"

namespace test::ag::element_ {

    using type = test::ag::types::aggregate_all_cvref<int>;
    using as_tuple_t = csl::ag::to_tuple_t<type>;
    static_assert(std::is_same_v<as_tuple_t, type::expected_to_tuple_t>); // see tests/conversion/to_tuple.hpp
    static_assert(csl::ag::size_v<type> == std::tuple_size_v<as_tuple_t>); // see tests/size.hpp

    template <std::size_t index>
    constexpr void expect_symetric_element(){

        // tuple_element
        static_assert(std::is_same_v<
            csl::ag::element<index, type>,
            std::tuple_element<index, as_tuple_t>
        >);
        static_assert(std::is_same_v<
            csl::ag::tuple_element<index, type>,
            std::tuple_element<index, as_tuple_t>
        >);

        // tuple_element_t
        static_assert(std::is_same_v<
            csl::ag::element_t<index, type>,
            std::tuple_element_t<index, as_tuple_t>
        >);
        static_assert(std::is_same_v<
            csl::ag::tuple_element_t<index, type>,
            std::tuple_element_t<index, as_tuple_t>
        >);
    };

    static_assert(csl::ag::size_v<type> == csl::ag::tuple_size_v<type>);
    constexpr static auto _ = []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool {
        ((expect_symetric_element<indexes>()), ...);
        return {};
    }(std::make_index_sequence<csl::ag::size_v<type>>{});
}
