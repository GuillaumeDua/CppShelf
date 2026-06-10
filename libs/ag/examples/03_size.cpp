#include <csl/ag.hpp>

struct A { int i; float f; };
static_assert(csl::ag::size<A>::value == 2);
static_assert(csl::ag::size_v<A>      == 2);

auto main() -> int {}
