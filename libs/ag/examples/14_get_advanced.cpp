#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    struct A { int i; float f; };
    auto value = A{ .i = 42, .f = 0.13F }; // NOLINT(*-avoid-magic-numbers)

    // output: "42 0.13 "
    [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((std::cout << csl::ag::get<indexes>(value) << ' '), ...);
    }(std::make_index_sequence<csl::ag::size_v<A>>{});
}
