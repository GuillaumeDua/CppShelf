#pragma once

#include <csl/wf.hpp>

namespace test::wf::chain_ {
    struct rvalue_only_node_type {
        constexpr void operator()() &         = delete;
        constexpr void operator()() &&        {}
        constexpr void operator()() const &   = delete;
        constexpr void operator()() const &&  = delete;
    };
    struct const_lvalue_only_node_type {
        constexpr void operator()() &         = delete;
        constexpr void operator()() &&        = delete;
        constexpr void operator()() const &   {};
        constexpr void operator()() const &&  = delete;
    };
}
namespace test::wf::chain_::invoke_ {

    consteval void cvref_correctness() {
        // rvalues
        csl::wf::chain_invoke(std::tuple{
            rvalue_only_node_type{},
            rvalue_only_node_type{},
            rvalue_only_node_type{}
        }, std::tuple{});

        const auto const_lvalue_only_node = const_lvalue_only_node_type{};
        csl::wf::chain_invoke(std::forward_as_tuple(
            const_lvalue_only_node,
            const_lvalue_only_node,
            const_lvalue_only_node
        ), std::tuple{});
    }
}