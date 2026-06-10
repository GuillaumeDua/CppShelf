#include <csl/ag.hpp>

struct type { int lvalue; int & llvalue; const int & const_lvalue; int && rvalue; };

// field 0 is NOT a reference: cvref-qualifier of the aggregate propagates
static_assert(std::same_as<int &&,       csl::ag::view_element_t<0, type&&>>);
static_assert(std::same_as<int &,        csl::ag::view_element_t<0, type&>>);
static_assert(std::same_as<const int &&, csl::ag::view_element_t<0, const type&&>>);
static_assert(std::same_as<const int &,  csl::ag::view_element_t<0, const type&>>);

// field 1 IS a reference: no cvref-qualifier propagation
static_assert(std::same_as<int &, csl::ag::view_element_t<1, type&&>>);
static_assert(std::same_as<int &, csl::ag::view_element_t<1, type&>>);
static_assert(std::same_as<int &, csl::ag::view_element_t<1, const type&&>>);
static_assert(std::same_as<int &, csl::ag::view_element_t<1, const type&>>);

auto main() -> int {}
