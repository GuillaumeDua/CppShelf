#include <csl/ag.hpp>

struct A { int i; char c; };
constexpr auto value = A{ 42, 'c' };
static_assert(csl::ag::get<0>(value) == 42);
static_assert(csl::ag::get<1>(value) == 'c');

auto main() -> int {}
