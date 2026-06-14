#define CSL_AG__ENABLE_FMTLIB_SUPPORT 1

#include <csl/ag.hpp>

auto main() -> int {
    using namespace csl::ag::io;

    struct A { int i; char c; };
    const A value{ .i = 42, .c = 'x' };

    fmt::println("{}", value);
    fmt::println("{}", value | indented);
    fmt::println("{}", value | indexed);
    fmt::println("{}", value | typenamed);
    fmt::println("{}", value | indented | indexed | typenamed);
}
