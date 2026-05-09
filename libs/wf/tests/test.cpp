// primitives
#include "./invocable_traits.hpp"
#include "./invoke_apply.hpp"

// mp
#include "./mp/tuple_view.hpp"

// types
#include "./repeat.hpp"
#include "./bind_front.hpp"
#include "./bind_back.hpp"
#include "./function_ref_and_view.hpp"
// type : chain/route
#include "./chain_trait.hpp"
#include "./chain_invoke.hpp"
#include "./route.hpp"

// syntactic sugar : literals
#include "./literals/times.hpp"

// syntactic sugar : operators
#include "./operators/factories/pipe_overload.hpp"
#include "./operators/factories/shift_equal_continuation.hpp"
#include "./operators/factories/star_repeat.hpp"
#include "./operators/views_refs/pipe_function_view.hpp"
#include "./operators/views_refs/pipe_function_cref.hpp"
#include "./operators/views_refs/pipe_function_ref.hpp"

#include <csl/wf.hpp>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

namespace {
    struct lvalue_node_type {
        constexpr void operator()() &        {}
        constexpr void operator()() &&       {}
        constexpr void operator()() const &  {}
        constexpr void operator()() const && {}
    };
}

TEST_CASE("wf::chain_trait - basic lambda", "[wf][compile_time][runtime]") {
    auto func = []<typename ...>(){};
    using func_type = decltype(func);

    using trait = csl::wf::chain_trait<func_type>;
    STATIC_REQUIRE(trait::is_invocable<>);

    csl::wf::chain_invoke(std::forward_as_tuple(func), std::tuple<>{});
    csl::wf::chain_invoke(std::forward_as_tuple(func), std::tuple<int>{});

    using namespace csl::wf::mp;
    STATIC_REQUIRE(trait::is_invocable<ttps<>>);
    STATIC_REQUIRE(trait::is_invocable<ttps<>&>);
    STATIC_REQUIRE(trait::is_invocable<ttps<int>>);
}

TEST_CASE("wf::chain_invoke - lvalue_node", "[wf][runtime]") {
    csl::wf::chain_invoke(std::tuple{
        lvalue_node_type{},
        lvalue_node_type{},
        lvalue_node_type{}
    }, std::tuple{});
}

TEST_CASE("wf::route - lvalue", "[wf][compile_time][runtime]") {
    STATIC_REQUIRE(
        csl::wf::details::mp::is_chain_invocable_v<
            csl::wf::mp::tuple_view_t<
                std::tuple<
                    lvalue_node_type,
                    lvalue_node_type,
                    lvalue_node_type
                >
            >,
            decltype(std::forward_as_tuple())
        >
    );

    auto route = csl::wf::route {
        lvalue_node_type{},
        lvalue_node_type{},
        lvalue_node_type{},
        lvalue_node_type{}
    };
    route();
}

TEST_CASE("wf::chain_invoke - node chain", "[wf][compile_time][runtime]") {
    auto node_1 = [](){ return 42; }; // NOLINT
    auto node_2 = [](int){};
    auto node_3 = [](){};

    STATIC_REQUIRE(csl::wf::mp::chain_trait<
        decltype(node_2), decltype(node_3)
    >::is_invocable<int>);
    STATIC_REQUIRE(csl::wf::mp::chain_trait<
        decltype(node_1), decltype(node_2), decltype(node_3)
    >::is_invocable<>);

    csl::wf::chain_invoke(std::tuple{
        node_1, node_2, node_3
    }, std::tuple{});
}

TEST_CASE("wf::route - forward arg", "[wf][runtime]") {
    auto route = csl::wf::route {
        [](){ return 42; }, // NOLINT
        [](int){  },
        [](){}
    };
    route();
}

TEST_CASE("wf::operators - >>= composition", "[wf][runtime]") {
    using namespace csl::wf::operators;

    auto route = [](){
        std::cout << "A";
    } >>=
    [](){
        std::cout << "B";
    } >>=
    [](){
        std::cout << "C\n";
    };
    route();
    std::move(route)();
    std::as_const(route)();
    std::move(std::as_const(route))();

    auto route_2 = std::move(route) >>= [](){
        std::cout << "D\n";
    };
    route_2();
}
