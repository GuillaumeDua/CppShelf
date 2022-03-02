#include "./invocable_traits.hpp"
#include "./invoke_apply.hpp"
#include "./bind_front.hpp"
#include "./chain_trait.hpp"
#include "./chain_invoke.hpp"
#include "./route.hpp"
#include "./repeat.hpp"

// syntactic sugar : literals
#include "./literals/times.hpp"

// syntactic sugar : operators
#include "./operators/pipe_overload.hpp"
#include "./operators/shift_equal_continuation.hpp"
#include "./operators/star_repeat.hpp"

#include <csl/wf.hpp>
#include <iostream>
auto main() -> int {

    struct lvalue_node_type {
        constexpr void operator()() &         {}
        constexpr void operator()() &&        {}
        constexpr void operator()() const &   {}
        constexpr void operator()() const &&  {}
    };
    {
        csl::wf::chain_invoke(std::tuple{
            lvalue_node_type{},
            lvalue_node_type{},
            lvalue_node_type{}
        }, std::tuple{});
    }
    {   // lvalue route

        static_assert(
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
    {
        auto node_1 = [](){ return 42; };
        auto node_2 = [](int){};
        auto node_3 = [](){};

        static_assert(csl::wf::mp::chain_trait<
            decltype(node_2), decltype(node_3)
        >::is_invocable<int>);
        static_assert(csl::wf::mp::chain_trait<
            decltype(node_1), decltype(node_2), decltype(node_3)
        >::is_invocable<>);

        csl::wf::chain_invoke(std::tuple{
            node_1, node_2, node_3
        }, std::tuple{});
    }
    {   // fwd arg
        auto route = csl::wf::route {
            [](){ return 42; },
            [](int){  },
            [](){}
        };
        route();
    }

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

    std::cout << gcl::cx::type_name_v<decltype(route)> << '\n';

    auto route_2 = std::move(route) >>= [](){
    // auto route_2 = route >>= [](){
        std::cout << "D\n";
    };
    std::cout << gcl::cx::type_name_v<decltype(route_2)> << '\n';
}
