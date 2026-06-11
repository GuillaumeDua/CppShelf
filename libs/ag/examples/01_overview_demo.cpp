#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1
#include <csl/ag.hpp>
#include <iostream>

struct S { char c; int i; };

static_assert(
    csl::ag::concepts::aggregate<S> and
    csl::ag::size_v<S> == 2
);
static_assert(std::same_as<char, csl::ag::element_t<0, S>>);
static_assert(std::same_as<int,  csl::ag::element_t<1, S>>);

auto main() -> int {
    S value{ 'A', 41 };
    ++csl::ag::get<1>(value);

    using namespace csl::ag::io;
    std::cout << "value: " << value << '\n';
}
