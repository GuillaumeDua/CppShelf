#include "./invocable_traits.hpp"
#include "./invoke_apply.hpp"
#include "./bind_front.hpp"
#include "./chain_trait.hpp"
#include "./route.hpp"
#include "./repeat.hpp"

// syntactic sugar :
#include "./operators/pipe_overload.hpp"
#include "./operators/shift_equal_continuation.hpp"
#include "./operators/star_repeat.hpp"

#include <iostream>
auto main() -> int {

#pragma region route continuation wip test
    using namespace csl::wf::operators;
    auto route = [](){
        std::cout << "A ";
    } >>=
    [](){
        std::cout << "B ";
    } >>=
    [](){
        std::cout << "C\n";
    };
    route();
    std::cout << gcl::cx::type_name_v<decltype(route)> << '\n';

    auto route_2 = std::move(route) >>= [](){
    // auto route_2 = route >>= [](){
        std::cout << "D\n";
    };
    std::cout << gcl::cx::type_name_v<decltype(route_2)> << '\n';
#pragma endregion
#pragma region repeat wip test

#pragma endregion
}
