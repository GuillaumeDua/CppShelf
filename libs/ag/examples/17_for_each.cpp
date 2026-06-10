#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13f };

    csl::ag::for_each([](auto && field){
        std::cout << field << ' ';
    }, value);
    std::cout << '\n';
}
