#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp> // opt-in: std::formatter support
#include <csl/typeinfo.hpp>                         // bridge prerequisite (explicit, for godbolt raw-URL include order)
#include <csl/ag/formatting/typeinfo.hpp>           // opt-in: gives csl::ag::formatting::typenamed clean type names (e.g. "int")
#include <iostream>                                 // std::print might not be available yet: use `std::cout << std::format(...)`

struct point     { int x; int y; };
struct rectangle { point top_left; point bottom_right; };

// Opting a type in is explicit and per-type: one line, next to the type.
// `point` needs none - it is only a nested field of `rectangle`.
template <>
struct std::formatter<rectangle> : csl::ag::formatting::std_formatter<rectangle>{}; // NOLINT(cert-dcl58-cpp)

static_assert(std::formattable<rectangle, char>);
static_assert(not std::formattable<point, char>);

auto main() -> int {
    using namespace csl::ag::formatting;

    constexpr auto value = rectangle{ .top_left = { 0, 0 }, .bottom_right = { 10, 5 } };

    // opted in: format the value directly, with "{}" or a format-spec letter
    std::cout << std::format("opt-in, default   -> {}\n",   value);
    std::cout << std::format("opt-in, spec :xit -> {}\n\n", std::format("{:xit}", value));

    // no opt-in needed: a decorated view carries its own options
    std::cout << std::format("view | indexed    -> {}\n", value.top_left | indexed);

    // ... and format_options::none reproduces the default output, without any opt-in
    std::cout << std::format("view | none       -> {}\n", value.top_left | format_options::none);
}
