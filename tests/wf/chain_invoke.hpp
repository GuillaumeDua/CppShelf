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
    struct nothrow_rvalue_only_node_type {
        constexpr void operator()() &         = delete;
        constexpr void operator()() && noexcept {}
        constexpr void operator()() const &   = delete;
        constexpr void operator()() const &&  = delete;
    };
    struct nothrow_const_lvalue_only_node_type {
        constexpr void operator()() &         = delete;
        constexpr void operator()() &&        = delete;
        constexpr void operator()() const & noexcept {};
        constexpr void operator()() const &&  = delete;
    };
}
namespace test::wf::chain_::invocable {
    consteval void is_chain_invocable() {
        static_assert(csl::wf::mp::chain_trait<
                rvalue_only_node_type,
                rvalue_only_node_type,
                rvalue_only_node_type
            >::is_invocable<>
        );
        static_assert(csl::wf::mp::chain_trait<
                rvalue_only_node_type &&,
                rvalue_only_node_type &&,
                rvalue_only_node_type &&
            >::is_invocable<>
        );
        static_assert(csl::wf::details::mp::is_chain_invocable_v<
            std::tuple<
                const const_lvalue_only_node_type &,
                const const_lvalue_only_node_type &,
                const const_lvalue_only_node_type &
            >,
            std::tuple<>
        >);
        static_assert(csl::wf::details::mp::is_chain_invocable_v<
            std::tuple<
                rvalue_only_node_type,
                const const_lvalue_only_node_type&,
                rvalue_only_node_type
            >,
            std::tuple<>
        >);
        static_assert(not csl::wf::details::mp::is_chain_invocable_v<
            std::tuple<
                rvalue_only_node_type,
                const_lvalue_only_node_type, // not const &
                rvalue_only_node_type
            >,
            std::tuple<>
        >);
    }

    consteval void is_chain_nothrow_invocable_KO() {
        // not noexcept
        static_assert(not csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                rvalue_only_node_type,
                rvalue_only_node_type,
                rvalue_only_node_type
            >,
            std::tuple<>
        >);
        static_assert(not csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                const const_lvalue_only_node_type &,
                const const_lvalue_only_node_type &,
                const const_lvalue_only_node_type &
            >,
            std::tuple<>
        >);
        static_assert(not csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                        rvalue_only_node_type,
                const   const_lvalue_only_node_type&,
                        rvalue_only_node_type
            >,
            std::tuple<>
        >);

        // not even invocable
        static_assert(not csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                rvalue_only_node_type,
                const_lvalue_only_node_type, // not const &
                rvalue_only_node_type
            >,
            std::tuple<>
        >);
        // one node is not noexcept
        static_assert(not csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                        nothrow_rvalue_only_node_type,
                const   nothrow_const_lvalue_only_node_type &,
                        rvalue_only_node_type // not noexcept
            >,
            std::tuple<>
        >);
    }
    consteval void is_chain_nothrow_invocable_OK() {
        static_assert(csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                nothrow_rvalue_only_node_type,
                nothrow_rvalue_only_node_type,
                nothrow_rvalue_only_node_type
            >,
            std::tuple<>
        >);
        static_assert(csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                const nothrow_const_lvalue_only_node_type &,
                const nothrow_const_lvalue_only_node_type &,
                const nothrow_const_lvalue_only_node_type &
            >,
            std::tuple<>
        >);
        static_assert(csl::wf::details::mp::is_chain_nothrow_invocable_v<
            std::tuple<
                        nothrow_rvalue_only_node_type,
                const   nothrow_const_lvalue_only_node_type&,
                        nothrow_rvalue_only_node_type
            >,
            std::tuple<>
        >);
    }
}
namespace test::wf::chain_::invoke_ {

    consteval void cvref_correctness() {
        // rvalues
        csl::wf::chain_invoke(std::tuple{
            rvalue_only_node_type{},
            rvalue_only_node_type{},
            rvalue_only_node_type{}
        }, std::tuple{});

        // const-lvalues
        const auto const_lvalue_only_node = const_lvalue_only_node_type{};
        csl::wf::chain_invoke(std::forward_as_tuple(
            const_lvalue_only_node,
            const_lvalue_only_node,
            const_lvalue_only_node
        ), std::tuple{});
    }
}