#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13f };

    std::cout << std::get<0>(value) << ", " << std::get<1>(value) << '\n';

    static_assert(std::same_as<int &,   decltype(std::get<0>(value))>);
    static_assert(std::same_as<float &, decltype(std::get<1>(value))>);
}
