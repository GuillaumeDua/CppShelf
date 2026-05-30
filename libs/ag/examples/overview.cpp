#include <csl/ag.hpp>

#include <array>
#include <tuple>

struct point   { int x; int y; };
struct rgb     { uint8_t r; uint8_t g; uint8_t b; };
struct single  { float value; };

// size and element type
static_assert(csl::ag::size_v<point>  == 2);
static_assert(csl::ag::size_v<rgb>    == 3);
static_assert(csl::ag::size_v<single> == 1);

static_assert(std::is_same_v<csl::ag::element_t<0, point>, int>);
static_assert(std::is_same_v<csl::ag::element_t<1, point>, int>);
static_assert(std::is_same_v<csl::ag::element_t<2, rgb>,   uint8_t>);

// concepts
static_assert(    csl::ag::concepts::aggregate<point>);
static_assert(not csl::ag::concepts::aggregate<std::tuple<int>>); // not an aggregate

auto main(int, char*[]) -> int
{
    // element access
    constexpr point p{ 3, 4 };
    static_assert(csl::ag::get<0>(p) == 3);
    static_assert(csl::ag::get<1>(p) == 4);

    // apply: unpack fields into a callable
    constexpr auto distance_sq = [](int x, int y) { return x * x + y * y; };
    static_assert(csl::ag::apply(distance_sq, p) == 25);

    // for_each: visit every field
    int sum = 0;
    csl::ag::for_each([&sum](auto v) { sum += static_cast<int>(v); }, rgb{10, 20, 30});
    assert(sum == 60); // NOLINT(*-assert)

    // view: aggregate as a tuple-like via operator|
    constexpr auto view = p | csl::ag::all;
    static_assert(std::tuple_size_v<decltype(view)> == 2);
    static_assert(std::get<0>(view) == 3);

    // structured bindings (STL tuple interface)
    auto [x, y] = p;
    assert(x == 3 and y == 4); // NOLINT(*-assert)
}
