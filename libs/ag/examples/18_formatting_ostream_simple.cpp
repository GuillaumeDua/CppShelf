#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/ostream.hpp> // opt-in: operator<<(std::ostream &, ...) support
#include <iostream>

auto main() -> int {
    using namespace csl::ag::formatting;

    struct A { int i; float f; };
    std::cout << A{ .i = 42, .f = .13F };
}
