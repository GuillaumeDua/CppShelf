#include <csl/typeinfo.hpp> // optional: gives csl::ag::io::typenamed clean type names (e.g. "int")
#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT 1

#include <csl/ag.hpp>
#include <format>
#include <iostream>

auto main() -> int {
    using namespace csl::ag::io;

    struct A { int i; char c; };
    const A value{ .i = 42, .c = 'x' };

    // csl::ag::io options are reachable through two equivalent syntaxes, which produce identical outputs:
    //   - a format-spec letter,   e.g. std::format("{:i}", value)
    //   - a composable view,      e.g. value | indented

    // NOTE: prefer std::println, when available
    std::cout << std::format("default:          -> {}\n", value);

    std::cout << std::format("spec \"{{:i}}\"       -> {}\n", std::format("{:i}", value));
    std::cout << std::format("view | indented   -> {}\n", value | indented);

    std::cout << std::format("spec \"{{:x}}\"       -> {}\n", std::format("{:x}", value));
    std::cout << std::format("view | indexed    -> {}\n", value | indexed);

    std::cout << std::format("spec \"{{:t}}\"       -> {}\n", std::format("{:t}", value));
    std::cout << std::format("view | typenamed  -> {}\n", value | typenamed);

    // options compose: combine several letters in one spec, or | several views together
    std::cout << std::format("spec \"{{:ixt}}\"     -> {}\n", std::format("{:ixt}", value));
    constexpr auto composed_view = indented | indexed | typenamed;
    std::cout << std::format("composed view     -> {}\n", value | composed_view);
}
