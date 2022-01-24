#include "./invocable_traits.hpp"
#include "./invoke_apply.hpp"
#include "./bind_front.hpp"
#include "./chain_trait.hpp"
//#include "./route.hpp"
#include "./repeat.hpp"

// syntactic sugar :
#include "./operators/pipe_overload.hpp"
#include "./operators/shift_equal_continuation.hpp"
#include "./operators/star_repeat.hpp"

#include <csl/wf.hpp>
#include <iostream>
auto main() -> int {

    struct lvalue_node_type {
        void operator()() &         { std::puts("&"); }
        void operator()() &&        { std::puts("&&"); }
        void operator()() const &   { std::puts("const &"); }
        void operator()() const &&  { std::puts("const &&"); }
    };
    {
        csl::wf::chain_invoke(std::tuple{
            lvalue_node_type{},
            lvalue_node_type{},
            lvalue_node_type{}
        }, std::tuple{});
    }
    {   // lvalue route
        auto route = csl::wf::route {
            lvalue_node_type{},
            lvalue_node_type{},
            lvalue_node_type{},
            lvalue_node_type{}
        };
        route();
    }

    // using namespace csl::wf::operators;

    // auto route = [](){
    //     std::cout << "A";
    // } >>=
    // [](){
    //     std::cout << "B";
    // } >>=
    // [](){
    //     std::cout << "C\n";
    // };
    // route();
    // std::move(route)();
    // std::as_const(route)();
    // std::move(std::as_const(route))();

    // std::cout << gcl::cx::type_name_v<decltype(route)> << '\n';

    // auto route_2 = std::move(route) >>= [](){
    // // auto route_2 = route >>= [](){
    //     std::cout << "D\n";
    // };
    // std::cout << gcl::cx::type_name_v<decltype(route_2)> << '\n';
}
