#include <csl/ensure.hpp>

#include <cassert>
#include <compare>
#include <unordered_map>

// NOLINTBEGIN(*-assert)
// NOLINTBEGIN(*-avoid-magic-numbers)

// csl::ensure: strong type definitions - same underlying type, incompatible at compile time.

using meters     = csl::ensure::strong_type<int, struct meters_tag>;
using kilograms  = csl::ensure::strong_type<int, struct kilograms_tag>;
using sensor_id  = csl::ensure::strong_type<int, struct sensor_id_tag>;
using calibrator = csl::ensure::strong_type<int(*)(int), struct calibrator_tag>;

// --- compile-time type properties ---

static_assert(    csl::ensure::type_traits::is_strong_type_v<meters>);
static_assert(not csl::ensure::type_traits::is_strong_type_v<int>);
static_assert(    csl::ensure::type_traits::is_strong_type_of_v<meters, int>);
static_assert(    csl::ensure::type_traits::is_tagged_by_v<meters, meters_tag>);
static_assert(std::is_same_v<csl::ensure::type_traits::underlying_type_t<meters>, int>);
static_assert(std::is_same_v<csl::ensure::type_traits::tag_type_t<meters>,        meters_tag>);

static_assert(    csl::ensure::concepts::StrongType<meters>);
static_assert(not csl::ensure::concepts::StrongType<int>);
static_assert(    csl::ensure::concepts::StrongTypeOf<meters, int>);
static_assert(    csl::ensure::concepts::TaggedBy<meters, meters_tag>);

auto main(int, char*[]) -> int
{
    // --- construction: explicit in, implicit out ---
    constexpr auto distance = meters{ 42 };
    [[maybe_unused]] constexpr auto mass = kilograms{ 10 };
    // mass = distance;  // won't compile: incompatible strong types

    // implicit const ref — opt-in escape hatch, e.g. for legacy APIs
    [[maybe_unused]] const int& raw = distance;
    assert(raw                                               == 42);
    static_assert(distance.underlying()                      == 42);
    static_assert(csl::ensure::to_underlying(distance) == 42);

    // unwrap: strips strong type, or passes through unchanged if already raw
    static_assert(csl::ensure::unwrap(distance) == 42);
    static_assert(csl::ensure::unwrap(42)        == 42);

    // --- comparison ---
    constexpr auto near = meters{ 10 };
    constexpr auto far  = meters{ 100 };

    static_assert(near < far);
    static_assert(near != far);
    static_assert(std::is_lt(near <=> far));
    static_assert(near < 50); // cross-type: meters vs raw int

    // --- callable forwarding ---
    // operator() is forwarded to the underlying type when it is callable
    constexpr auto offset = calibrator{ [](int x) { return x + 1; } };
    static_assert(offset(41) == 42);

    // --- STL: std::hash specialised automatically ---
    auto readings = std::unordered_map<sensor_id, meters>{
        { sensor_id{ 7 }, meters{ 250 } },
        { sensor_id{ 9 }, meters{ 480 } },
    };
    assert(readings.at(sensor_id{ 7 }) == meters{ 250 });

    // --- CPO comparators: unwrap-aware equality and ordering ---
    constexpr auto eq = csl::ensure::strong_type_equal_to{};
    static_assert(eq(meters{ 42 }, meters{ 42 }));
    static_assert(eq(meters{ 42 }, 42)); // cross-type comparison via unwrap
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-assert)
