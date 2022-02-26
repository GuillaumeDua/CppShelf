#pragma once

#include <csl/wf.hpp>

namespace test::wf::route_ {
    const auto node_1 = []()      { return 41; /* NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers) */ };
    const auto node_2 = [](int i) { return i + 1; };
    const auto node_3 = []()      { };

#pragma region fwd (node_1 >[int]> node_2)
    // std::invocable
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1 })
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2 })
    >);

    // invoke_result_t
    static_assert(std::same_as<
        std::invoke_result_t<decltype(csl::wf::route{ node_1 })>,
        int
    >);
    static_assert(std::same_as<
        std::invoke_result_t<decltype(csl::wf::route{ node_1, node_2 })>,
        int
    >);
#pragma endregion
#pragma region fwd (node_2 >[int]> node_2 >[int]> node_2) 
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_2, node_2 }),
        int
    >);
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_2, node_2, node_2 }),
        int
    >);
#pragma endregion
#pragma region discard (node_1 >[int]> node_3)
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_3 })
    >);
#pragma endregion
#pragma region fwd, discard (node_1 >[int]> node_2 >> node_3)
   
    static_assert(std::invocable<
        decltype(csl::wf::route{ node_1, node_2, node_3 })
    >);
#pragma endregion

    consteval void call_cvref_correctness() {

        struct lvalue_node_type {
            void operator()() & {}
            void operator()() const & = delete;
        };
        struct const_lvalue_node_type {
            void operator()() & = delete;
            void operator()() const & {}
        };
        struct rvalue_node_type {
            void operator()() && {};
            void operator()() & = delete;
        };
        struct const_rvalue_node_type {
            void operator()() && = delete;
            void operator()() const && {}
        };

        {   // lvalue route
            auto route = csl::wf::route {
                lvalue_node_type{},
                lvalue_node_type{},
                lvalue_node_type{}
            };
            route();
        }
        {   // rvalue route
            csl::wf::route {
                rvalue_node_type{},
                rvalue_node_type{},
                rvalue_node_type{}
            }();
        }
        {   // const lvalue route
            const auto route = csl::wf::route {
                const_lvalue_node_type{},
                const_lvalue_node_type{},
                const_lvalue_node_type{}
            };
            route();
        }
        {   // const rvalue route
            const auto route = csl::wf::route {
                const_rvalue_node_type{},
                const_rvalue_node_type{},
                const_rvalue_node_type{}
            };
            std::move(route)();
        }
    }

    consteval void constexpr_nodes() {
        constexpr auto route = csl::wf::route {
            []() constexpr { return 40; },
            [](int i) constexpr {return i + 1; },
            [](int i) constexpr {return i + 1; }
        };
        static_assert(route() == 42);
    }
}
namespace test::wf::route_noexcept {

    constexpr auto may_thow_route = csl::wf::route{ []() noexcept {}, []() noexcept {}, [](){} };
    static_assert(std::invocable<decltype(may_thow_route)>);
    static_assert(not std::is_nothrow_invocable_v<decltype(may_thow_route)>);

    constexpr auto nothow_route = csl::wf::route{ []() noexcept {}, []() noexcept {}, []() noexcept {} };
    static_assert(std::invocable<decltype(nothow_route)>);
    static_assert(std::is_nothrow_invocable_v<decltype(nothow_route)>);
}