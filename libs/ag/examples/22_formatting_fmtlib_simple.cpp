#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/fmt.hpp> // opt-in: fmt::formatter support
#include <csl/typeinfo.hpp>                  // bridge prerequisite (explicit, for godbolt raw-URL include order)
#include <csl/ag/formatting/typeinfo.hpp>    // opt-in: gives csl::ag::io::typenamed clean type names (e.g. "int")

auto main() -> int {
    using namespace csl::ag::io;

    struct A { int i; char c; };
    const A value{ .i = 42, .c = 'x' };

    fmt::println("{}", value);
    fmt::println("{}", value | indented);
    fmt::println("{}", value | indexed);
    fmt::println("{}", value | typenamed);
    fmt::println("{}", value | indented | indexed | typenamed);

    // WIP: view composition
    // constexpr auto view = indented | indexed | typenamed;
    // fmt::println("{}", value | view);
}
