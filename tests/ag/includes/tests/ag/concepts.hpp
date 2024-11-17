#pragma once

#include <csl/ag.hpp>
#include <tests/types.hpp>

#include <string>
#include <tuple>
#include <array>
#include <vector>

namespace test::ag::concepts_::aggregate_ {

    namespace concepts = csl::ag::concepts;

    // user-defined aggregate types
    static_assert(concepts::aggregate<test::ag::types::field_1>);
    static_assert(concepts::aggregate<test::ag::types::field_2>);
    static_assert(concepts::aggregate<test::ag::types::field_ref_1>);
    static_assert(concepts::aggregate<test::ag::types::field_ref_2>);
    static_assert(concepts::aggregate<test::ag::types::field_ref_3>);
    static_assert(concepts::aggregate<test::ag::types::aggregate_all_cvref<int>>);

    static_assert(concepts::aggregate<test::ag::types::field_3_nested>);
    static_assert(concepts::aggregate<test::ag::types::field_3_nested_tuplelike>);
    static_assert(concepts::aggregate<test::ag::types::field_4_nested_range>);
    static_assert(concepts::aggregate<test::ag::types::field_5_nested_tuplelike_and_range>);
    static_assert(concepts::aggregate<test::ag::types::field_everything>);
   
    // rangelikes
    static_assert(not concepts::aggregate<std::string>);
    static_assert(not concepts::aggregate<std::vector<int>>);
    static_assert(not concepts::aggregate<std::vector<std::string>>);

    // tuplelikes
    static_assert(concepts::aggregate<std::array<int, 3>>); // range + tuplelike
    static_assert(not concepts::aggregate<std::pair<int, int>>);
    static_assert(not concepts::aggregate<std::tuple<int, int>>);
}

namespace test::ag::concepts_::tuple_like_ {

    namespace concepts = csl::ag::concepts;

}

namespace test::ag::concepts_::structured_bindable_ {

    namespace concepts = csl::ag::concepts;

}