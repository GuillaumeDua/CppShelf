#include <csl/ag.hpp>

struct A { int i; float & f; const char && c; };

static_assert(std::same_as<int&&,        csl::ag::view_element_t<0, A&&>>);
static_assert(std::same_as<float&,       csl::ag::view_element_t<1, A&&>>);
static_assert(std::same_as<const char&&, csl::ag::view_element_t<2, A&&>>);

static_assert(std::same_as<const int&,   csl::ag::view_element_t<0, const A&>>);
static_assert(std::same_as<float&,       csl::ag::view_element_t<1, const A&>>);
static_assert(std::same_as<const char&&, csl::ag::view_element_t<2, const A&>>);

auto main() -> int {}
