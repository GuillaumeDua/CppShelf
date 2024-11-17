#pragma once

#include <csl/ag.hpp>
#include <tests/types.hpp>

namespace test::ag::concepts_ {

    namespace concepts = csl::ag::concepts;

    static_assert(concepts::aggregate<test::ag::types::field_1>);
    static_assert(concepts::aggregate<test::ag::types::field_2>);
    static_assert(concepts::aggregate<test::ag::types::field_ref_1>);
    static_assert(concepts::aggregate<test::ag::types::field_ref_2>);
    static_assert(concepts::aggregate<test::ag::types::field_ref_3>);
    static_assert(concepts::aggregate<test::ag::types::aggregate_all_cvref<int>>);
   
    static_assert(concepts::aggregate<test::ag::types::field_1>);
    
    
}