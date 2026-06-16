#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT 1

#include <csl/ag.hpp>
#include <format>
#include <iostream>

auto main() -> int {
    using namespace csl::ag::io;

    struct A { int i; char c; };
    const A value{ .i = 42, .c = 'x' };

    // NOTE: prefer std::print, if available
    std::cout
        << std::format("{}\n", value)
        << std::format("{}\n", value | indented)
        << std::format("{}\n", value | indexed)
        << std::format("{}\n", value | typenamed)
        << std::format("{}\n", value | indented | indexed | typenamed)
    ;

    // WIP: view composition
    // auto view = indented | indexed | typenamed;
    // std::cout << std::format("{}", value | view);
}
