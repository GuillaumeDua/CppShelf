#pragma once

#include "../../workflow/workflow.hpp"

namespace test::functional::chain_trait_ {
    using namespace workflow::functional;

    struct type_1{};
    struct type_2{};

    auto nothing_to_type1       = []()                { return type_1{}; }; using nothing_to_type1_t      = decltype(nothing_to_type1);
    auto nothing_to_type1_ne    = []() noexcept       { return type_1{}; }; using nothing_to_type1_ne_t   = decltype(nothing_to_type1_ne);
    auto type2_to_type1         = [](type_2)          { return type_1{}; }; using type2_to_type1_t        = decltype(type2_to_type1);
    auto type2_to_type1_ne      = [](type_2) noexcept { return type_1{}; }; using type2_to_type1_ne_t     = decltype(type2_to_type1_ne);
    auto type1_to_type2         = [](type_1)          { return type_2{}; }; using type1_to_type2_t        = decltype(type1_to_type2);
    auto type1_to_type2_ne      = [](type_1) noexcept { return type_2{}; }; using type1_to_type2_ne_t     = decltype(type1_to_type2_ne);
}
namespace test::functional::chain_trait_::single_node::_nothing_to_type1 {
    using trait = chain_trait<nothing_to_type1_t>;
    static_assert(trait::is_invocable<>);
    static_assert(trait::is_nodiscard_invocable<>);
    static_assert(not trait::is_nothrow_invocable<>);
    static_assert(not trait::is_nothrow_nodiscard_invocable<>);
    static_assert(std::is_same_v<type_1, trait::invoke_result_t<>>);
}
namespace test::functional::chain_trait_::single_node::_nothing_to_type1_ne {
    using trait = chain_trait<nothing_to_type1_ne_t>;
    static_assert(trait::is_invocable<>);
    static_assert(trait::is_nodiscard_invocable<>);
    static_assert(trait::is_nothrow_invocable<>);
    static_assert(trait::is_nothrow_nodiscard_invocable<>);
    static_assert(std::is_same_v<type_1, trait::invoke_result_t<>>);
}
namespace test::functional::chain_trait_::single_node::_type2_to_type1 {
    using trait = chain_trait<type2_to_type1_t>;
    static_assert(trait::is_invocable<type_2>);
    static_assert(trait::is_nodiscard_invocable<type_2>);
    static_assert(not trait::is_nothrow_invocable<type_2>);
    static_assert(not trait::is_nothrow_nodiscard_invocable<type_2>);
    static_assert(std::is_same_v<type_1, trait::invoke_result_t<type_2>>);
}
namespace test::functional::chain_trait_::single_node::_type2_to_type1_ne {
    using trait = chain_trait<type2_to_type1_ne_t>;
    static_assert(trait::is_invocable<type_2>);
    static_assert(trait::is_nodiscard_invocable<type_2>);
    static_assert(trait::is_nothrow_invocable<type_2>);
    static_assert(trait::is_nothrow_nodiscard_invocable<type_2>);
    static_assert(std::is_same_v<type_1, trait::invoke_result_t<type_2>>);
}
namespace test::functional::chain_trait_::two_nodes {
    using trait = chain_trait<nothing_to_type1_t, type1_to_type2_ne_t>;
    
    static_assert(trait::is_invocable<>);
    static_assert(trait::is_nodiscard_invocable<>);
    static_assert(not trait::is_nothrow_invocable<>);
    static_assert(not trait::is_nothrow_nodiscard_invocable<>);
    static_assert(std::is_same_v<type_2, trait::invoke_result_t<>>);
}
namespace test::functional::chain_trait_::two_nodes_ne {
    using trait = chain_trait<nothing_to_type1_ne_t, type1_to_type2_ne_t>;
    
    static_assert(trait::is_invocable<>);
    static_assert(trait::is_nodiscard_invocable<>);
    static_assert(trait::is_nothrow_invocable<>);
    static_assert(trait::is_nothrow_nodiscard_invocable<>);
    static_assert(std::is_same_v<type_2, trait::invoke_result_t<>>);
}
namespace test::functional::chain_trait_::route_no_discard {
    using trait = chain_trait<type2_to_type1_ne_t, type1_to_type2_t, type2_to_type1_ne_t, type1_to_type2_ne_t>; // one is not noexcept
    
    static_assert(trait::is_invocable<type_2>);
    static_assert(trait::is_nodiscard_invocable<type_2>);
    static_assert(not trait::is_nothrow_invocable<type_2>);
    static_assert(not trait::is_nothrow_nodiscard_invocable<type_2>);
    static_assert(std::is_same_v<type_2, trait::invoke_result_t<type_2>>);
}
namespace test::functional::chain_trait_::route_no_discard_ne {
    using trait = chain_trait<
        type2_to_type1_ne_t,
        type1_to_type2_ne_t,
        type2_to_type1_ne_t,
        type1_to_type2_ne_t
    >;
    
    static_assert(trait::is_invocable<type_2>);
    static_assert(trait::is_nodiscard_invocable<type_2>);
    static_assert(trait::is_nothrow_invocable<type_2>);
    static_assert(trait::is_nothrow_nodiscard_invocable<type_2>);
    static_assert(std::is_same_v<type_2, trait::invoke_result_t<type_2>>);
}
namespace test::functional::chain_trait_::route_discard_at_entry {
    using trait = chain_trait<
        nothing_to_type1_t,
        type1_to_type2_t
    >;
    
    static_assert(trait::is_invocable<type_2>);
    static_assert(trait::is_nodiscard_invocable<>);
    static_assert(not trait::is_nodiscard_invocable<type_2>);
    static_assert(std::is_same_v<type_2, trait::invoke_result_t<>>);
}

namespace test::functional::chain_trait_::route_discard {
    using trait = chain_trait<
        type2_to_type1_t,
        type1_to_type2_t,
        nothing_to_type1_t,
        type1_to_type2_t
    >;
    
    static_assert(trait::is_invocable<type_2>);
    static_assert(not trait::is_nodiscard_invocable<type_2>);
    static_assert(not trait::is_nothrow_invocable<type_2>);
    static_assert(not trait::is_nothrow_nodiscard_invocable<type_2>);
    static_assert(std::is_same_v<type_2, trait::invoke_result_t<type_2>>);
}