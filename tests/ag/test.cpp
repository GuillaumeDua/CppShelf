#include <csl/ag.hpp>

#include <iostream>
#include <cassert>
#include <memory>

struct toto{ int i; char c; };

auto main() -> int {
    using namespace csl::ag;

    // TODO : wrap that in a test function

    auto value = toto{};
    auto as_tuple = csl::ag::as_tuple(value);

    static_assert(std::same_as<
        int&,
        std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
    >);
    static_assert(std::same_as<
        char&,
        std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
    >);

    assert(
        std::addressof(std::get<0>(as_tuple)) ==
        std::addressof(value.i)
    );
    assert(
        std::addressof(std::get<1>(as_tuple)) ==
        std::addressof(value.c)
    );
}
