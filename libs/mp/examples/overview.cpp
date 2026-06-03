#include <csl/mp.hpp>

#include <array>
#include <functional>
#include <tuple>
#include <cstdint>

// NOLINTBEGIN(*-avoid-magic-numbers)

// csl::mp: C++ meta-programming utilities
//  compile-time type-list operations, tuple implementation and tuple-likes algorithms.

// --- tuple: fixed-size heterogeneous sequence ---
static_assert(csl::mp::size_v<csl::mp::tuple<int, float, bool>> == 3);
static_assert(std::is_same_v<csl::mp::element_t<1, csl::mp::tuple<int, float, bool>>, float>);

// --- concepts ---
static_assert(    csl::mp::concepts::tuple_like<std::tuple<int, float>>);
static_assert(    csl::mp::concepts::tuple_like<std::pair<int, float>>);
static_assert(    csl::mp::concepts::tuple_like<std::array<int, 3>>);
static_assert(    csl::mp::concepts::pair_like<std::pair<int, float>>);
static_assert(not csl::mp::concepts::pair_like<std::tuple<int, float, bool>>);
static_assert(    csl::mp::concepts::instance<std::tuple<int, float>, std::tuple>);

// --- type traits ---
static_assert(    csl::mp::type_traits::is_homogeneous_v<std::array<int, 3>>);
static_assert(not csl::mp::type_traits::is_homogeneous_v<std::tuple<int, float>>);

// all field types in a schema satisfy a constraint
using integral_schema = csl::mp::type_traits::is_constrained_by<std::tuple<int, std::int64_t>, std::is_integral>;
static_assert(integral_schema::value);

// negate: inverse of a predicate
using non_integral = csl::mp::type_traits::negate<std::is_integral>;
static_assert(non_integral::type<float>::value);

// --- type-list algorithms ---
// Field types of a sensor event: [sensor_id, value, channel_id, is_valid]
// int appears twice (two separate id fields of the same underlying type)
using event_fields = csl::mp::tuple<int, float, int, bool>;

// query
static_assert(    csl::mp::type_traits::contains_v<event_fields, int>);
static_assert(not csl::mp::type_traits::contains_v<event_fields, double>);
static_assert(csl::mp::type_traits::index_of_v     <event_fields, float> == 1);
static_assert(csl::mp::type_traits::last_index_of_v<event_fields, int>   == 2);
static_assert(csl::mp::type_traits::count_v        <event_fields, int>   == 2);
static_assert(csl::mp::type_traits::count_if_v     <event_fields, std::is_integral> == 3); // int, int, bool

// transformation
using integral_fields  = csl::mp::type_traits::filter_t   <event_fields, std::is_integral>;
static_assert(std::is_same_v<integral_fields, csl::mp::tuple<int, int, bool>>);

using high_res_fields  = csl::mp::type_traits::replace_t   <event_fields, float, double>;
static_assert(std::is_same_v<high_res_fields, csl::mp::tuple<int, double, int, bool>>);

using high_res_fields2 = csl::mp::type_traits::replace_if_t<event_fields, std::is_floating_point, double>;
static_assert(std::is_same_v<high_res_fields2, csl::mp::tuple<int, double, int, bool>>);

using unique_fields    = csl::mp::type_traits::unique_t    <event_fields>;
static_assert(std::is_same_v<unique_fields, csl::mp::tuple<int, float, bool>>);

using extended_fields  = csl::mp::type_traits::push_back_t <csl::mp::tuple<int, float>, bool>;
static_assert(std::is_same_v<extended_fields, csl::mp::tuple<int, float, bool>>);

using payload_fields   = csl::mp::type_traits::pop_front_t <csl::mp::tuple<int, float, bool>>;
static_assert(std::is_same_v<payload_fields, csl::mp::tuple<float, bool>>);

// set operations: readable vs writable capabilities of a device interface
using readable_types  = csl::mp::tuple<int, float>;
using writable_types  = csl::mp::tuple<float, bool>;

using all_types       = csl::mp::type_traits::set_union_t       <readable_types, writable_types>;
using shared_types    = csl::mp::type_traits::set_intersection_t<readable_types, writable_types>;
using read_only_types = csl::mp::type_traits::set_difference_t  <readable_types, writable_types>;

static_assert(std::is_same_v<all_types,       csl::mp::tuple<int, float, bool>>);
static_assert(std::is_same_v<shared_types,    csl::mp::tuple<float>>);
static_assert(std::is_same_v<read_only_types, csl::mp::tuple<int>>);

auto main(int, char*[]) -> int
{
    // --- construction and access ---
    // A sensor reading: (sensor_id, temperature, is_valid)
    constexpr auto reading = csl::mp::tuple{ 7, 36.6F, true };
    static_assert(reading.get<0>()                   == 7);     // by index
    static_assert(reading.get<bool>());                         // by type
    static_assert(reading[csl::mp::index_t<1>{}]     == 36.6F); // operator[]
    static_assert(reading[std::type_identity<int>{}] == 7);

    // --- cat: concatenate two tuples ---
    constexpr auto header  = csl::mp::functions::make_tuple(1, 2);
    constexpr auto payload = csl::mp::tuple{3.0F};
    constexpr auto packet  = csl::mp::functions::cat(header, payload);
    static_assert(csl::mp::size_v<decltype(packet)> == 3);
    static_assert(get<2>(packet) == 3.0F);

    // --- for_each: accumulate across heterogeneous values ---
    constexpr auto total_score = [] {
        int score = 0;
        csl::mp::functions::for_each(
            csl::mp::functions::make_tuple(10, 20, 12),
            [&score](auto v) { score += v; }
        );
        return score;
    }();
    static_assert(total_score == 42);

    // --- for_each_enumerate: index-aware traversal ---
    constexpr auto thresholds_valid = [] {
        bool ok = true;
        csl::mp::functions::for_each_enumerate(
            csl::mp::functions::make_tuple(10, 20, 30),
            [&ok](std::size_t i, auto v) { ok = ok and (v == static_cast<int>((i + 1) * 10)); }
        );
        return ok;
    }();
    static_assert(thresholds_valid);

    // --- fold_left: reduce to a single value ---
    // sum of four quarterly deltas
    static_assert(
        csl::mp::functions::fold_left(
            csl::mp::functions::make_tuple(8, 11, 13, 10),
            std::plus<int>{}, 0
        ) == 42
    );

    // --- all_of / any_of / none_of ---
    constexpr auto channel_ids = csl::mp::functions::make_tuple(2, 4, 6);
    static_assert(csl::mp::functions::all_of (channel_ids, [](int v) { return v % 2 == 0; }));
    static_assert(csl::mp::functions::any_of (channel_ids, [](int v) { return v == 4; }));
    static_assert(csl::mp::functions::none_of(channel_ids, [](int v) { return v > 10; }));

    // --- forward_like (P2445): cast value-category to match a model type ---
    auto owner = 42;
    static_assert(std::is_same_v<
        decltype(csl::mp::forward_like<int&&>(owner)),
        int&&
    >);
}

// NOLINTEND(*-avoid-magic-numbers)
