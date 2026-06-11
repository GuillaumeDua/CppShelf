#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {

    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13F }; // NOLINT(*-avoid-magic-numbers)

    // output: "42, 0.13"
    std::cout << csl::ag::get<0>(value) << ", " << csl::ag::get<1>(value) << '\n';

    static_assert(std::same_as<int &,   decltype(csl::ag::get<0>(value))>);
    static_assert(std::same_as<float &, decltype(csl::ag::get<1>(value))>);
}
