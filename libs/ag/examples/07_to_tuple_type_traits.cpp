#include <csl/ag.hpp>
#include <utility>

struct A { int i; float f; };

static_assert(std::same_as<
    std::tuple<int, float>,
    csl::ag::to_tuple_t<A>
>);

static_assert(std::same_as<int, csl::ag::element_t<0, A>>);
static_assert(std::same_as<int, std::tuple_element_t<0, csl::ag::to_tuple_t<A>>>);
static_assert(std::same_as<int, std::tuple_element_t<0, decltype(csl::ag::to_tuple(A{}))>>);

auto main() -> int {
    constexpr auto value = A{ .i = 42, .f = 0.13f };
    constexpr auto value_as_tuple = csl::ag::to_tuple(std::move(value));

    static_assert(42    == std::get<0>(value_as_tuple));
    static_assert(0.13f == std::get<1>(value_as_tuple));
}
