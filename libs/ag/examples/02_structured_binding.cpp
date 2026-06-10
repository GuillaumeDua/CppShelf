#include <csl/ag.hpp>
#include <cassert>

auto main() -> int {
    struct type { int i; char c; };
    auto value = type{ 42, 'A' }; // NOLINT

    [[maybe_unused]] auto && [v0, v1] = value;
    assert(v0 == 42);
    assert(v1 == 'A');
}
