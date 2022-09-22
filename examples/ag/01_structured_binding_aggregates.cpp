// godbolt API
// clang_trunk, gsnapshot

// TODO : ifdef compiler-explorer : abs path to raw file
//  otherwise <csl/ag.hpp>
#include "../includes/ag/csl/ag.hpp"
#include <cassert>

auto main() -> int {
    struct type{ int i; char c; };
    auto value = type{ 42, 'A' }; // NOLINT

    [[maybe_unused]] auto && [ v0, v1 ] = value;
    assert(v0 == 42);   // pass
    assert(v1 == 'A');  // pass
}
