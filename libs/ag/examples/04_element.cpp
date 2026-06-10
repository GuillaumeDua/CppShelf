#include <csl/ag.hpp>

struct A { int i; float f; };
static_assert(std::same_as<int,   csl::ag::element_t<0, A>>);
static_assert(std::same_as<float, csl::ag::element_t<1, A>>);

auto main() -> int {}
