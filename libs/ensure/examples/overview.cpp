#include <csl/ensure.hpp>

#include <cassert>

using meters  = csl::ensure::strong_type<int, struct meter_tag>;
using seconds = csl::ensure::strong_type<int, struct second_tag>;
using fn_t    = csl::ensure::strong_type<int(*)(int), struct fn_tag>;

// type traits
static_assert(    csl::ensure::type_traits::is_strong_type_v<meters>);
static_assert(not csl::ensure::type_traits::is_strong_type_v<int>);
static_assert(    csl::ensure::type_traits::is_strong_type_of_v<meters, int>);
static_assert(    csl::ensure::type_traits::is_tagged_by_v<meters, meter_tag>);
static_assert(std::is_same_v<csl::ensure::type_traits::underlying_type_t<meters>, int>);
static_assert(std::is_same_v<csl::ensure::type_traits::tag_type_t<meters>, meter_tag>);

// concepts
static_assert(    csl::ensure::concepts::StrongType<meters>);
static_assert(not csl::ensure::concepts::StrongType<int>);
static_assert(    csl::ensure::concepts::StrongTypeOf<meters, int>);
static_assert(    csl::ensure::concepts::TaggedBy<meters, meter_tag>);

auto main(int, char*[]) -> int
{
    // explicit construction, implicit conversion to underlying
    meters  m{ 42 };
    [[maybe_unused]] seconds s{ 10 };
    // m = s;  // error: incompatible types

    int& ref = m;               // implicit lvalue ref
    assert(ref == 42);          // NOLINT(*-assert)
    assert(m.underlying() == 42); // NOLINT(*-assert)
    assert(csl::ensure::to_underlying(m) == 42); // NOLINT(*-assert)

    // unwrap: strips strong type or passes through
    assert(csl::ensure::unwrap(m) == 42); // NOLINT(*-assert)
    assert(csl::ensure::unwrap(42)  == 42); // NOLINT(*-assert)

    // comparison
    meters a{1}, b{2};
    assert(a < b);                   // NOLINT(*-assert)
    assert(a != b);                  // NOLINT(*-assert)
    assert(std::is_lt(a <=> b));      // NOLINT(*-assert)
    assert(a < 2);                   // vs raw underlying // NOLINT(*-assert)

    // invocation: operator() forwarded when T is callable
    fn_t cb{ [](int x) { return x * 2; } };
    assert(cb(21) == 42); // NOLINT(*-assert)

    // STL: std::hash specialised automatically
    std::unordered_map<meters, int> map;
    map[meters{1}] = 100;
    assert(map.at(meters{1}) == 100); // NOLINT(*-assert)

    // CPO comparators
    csl::ensure::strong_type_equal_to eq;
    assert(eq(meters{5}, meters{5}));  // NOLINT(*-assert)
    assert(eq(meters{5}, 5));          // cross-type via unwrap // NOLINT(*-assert)
}
