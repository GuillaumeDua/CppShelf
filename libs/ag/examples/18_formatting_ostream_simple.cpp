#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1

#include <csl/ag.hpp>
#include <iostream>

auto main() -> int {
    using namespace csl::ag::io;

    struct A { int i; float f; };
    std::cout << A{ .i = 42, .f = .13F };
}
