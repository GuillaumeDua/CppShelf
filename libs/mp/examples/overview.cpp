#include <csl/mp.hpp>

#include <array>
#include <tuple>

// tuple
static_assert(csl::mp::size_v<csl::mp::tuple<int, float, bool>> == 3);
static_assert(std::is_same_v<csl::mp::element_t<1, csl::mp::tuple<int, float, bool>>, float>);

// concepts
static_assert(    csl::mp::concepts::tuple_like<std::tuple<int, float>>);
static_assert(    csl::mp::concepts::tuple_like<std::pair<int, float>>);
static_assert(    csl::mp::concepts::tuple_like<std::array<int, 3>>);
static_assert(    csl::mp::concepts::pair_like<std::pair<int, float>>);
static_assert(not csl::mp::concepts::pair_like<std::tuple<int, float, bool>>);
static_assert(    csl::mp::concepts::instance<std::tuple<int, float>, std::tuple>);

// type traits
static_assert(    csl::mp::type_traits::is_homogeneous_v<std::array<int, 3>>);
static_assert(not csl::mp::type_traits::is_homogeneous_v<std::tuple<int, float>>);

using all_integral = csl::mp::type_traits::is_constrained_by<std::tuple<int, long>, std::is_integral>;
static_assert(all_integral::value);

// bind_front / negate
using not_integral = csl::mp::type_traits::negate<std::is_integral>;
static_assert(not_integral::type<float>::value);

// --- type-list algorithms ---
using T = csl::mp::tuple<int, float, int, bool>;

// query
static_assert(    csl::mp::type_traits::contains_v<T, int>);
static_assert(not csl::mp::type_traits::contains_v<T, double>);
static_assert(csl::mp::type_traits::index_of_v<T, float>      == 1);
static_assert(csl::mp::type_traits::last_index_of_v<T, int>   == 2);
static_assert(csl::mp::type_traits::count_v<T, int>           == 2);
static_assert(csl::mp::type_traits::count_if_v<T, std::is_integral> == 3); // int, int, bool

// transformation
using filtered     = csl::mp::type_traits::filter_t<T, std::is_integral>;
static_assert(std::is_same_v<filtered, csl::mp::tuple<int, int, bool>>);

using replaced     = csl::mp::type_traits::replace_t<T, float, double>;
static_assert(std::is_same_v<replaced, csl::mp::tuple<int, double, int, bool>>);

using replaced_if  = csl::mp::type_traits::replace_if_t<T, std::is_floating_point, double>;
static_assert(std::is_same_v<replaced_if, csl::mp::tuple<int, double, int, bool>>);

using deduplicated = csl::mp::type_traits::unique_t<T>;
static_assert(std::is_same_v<deduplicated, csl::mp::tuple<int, float, bool>>);

using pushed       = csl::mp::type_traits::push_back_t<csl::mp::tuple<int, float>, bool>;
static_assert(std::is_same_v<pushed, csl::mp::tuple<int, float, bool>>);

using popped       = csl::mp::type_traits::pop_front_t<csl::mp::tuple<int, float, bool>>;
static_assert(std::is_same_v<popped, csl::mp::tuple<float, bool>>);

// set operations
using A = csl::mp::tuple<int, float>;
using B = csl::mp::tuple<float, bool>;

using u = csl::mp::type_traits::set_union_t<A, B>;
static_assert(std::is_same_v<u, csl::mp::tuple<int, float, bool>>);

using i = csl::mp::type_traits::set_intersection_t<A, B>;
static_assert(std::is_same_v<i, csl::mp::tuple<float>>);

using d = csl::mp::type_traits::set_difference_t<A, B>;
static_assert(std::is_same_v<d, csl::mp::tuple<int>>);

auto main(int, char*[]) -> int
{
    // tuple construction and access
    constexpr auto t = csl::mp::functions::make_tuple(42, 3.14f, true);
    static_assert(t.get<0>()                   == 42);
    static_assert(t.get<bool>()                == true);
    static_assert(t[csl::mp::index_t<1>{}]     == 3.14f);
    static_assert(t[std::type_identity<int>{}] == 42);

    // cat
    constexpr auto a = csl::mp::functions::make_tuple(1, 2);
    constexpr auto b = csl::mp::functions::make_tuple(3.0f);
    constexpr auto c = csl::mp::functions::cat(a, b);
    static_assert(csl::mp::size_v<decltype(c)> == 3);
    static_assert(get<2>(c) == 3.0f);

    // for_each
    constexpr auto sum = [] {
        int s = 0;
        csl::mp::functions::for_each(csl::mp::functions::make_tuple(1, 2, 3), [&s](auto v) { s += v; });
        return s;
    }();
    static_assert(sum == 6);

    // for_each_enumerate
    constexpr auto enumerate_ok = [] {
        bool ok = true;
        csl::mp::functions::for_each_enumerate(
            csl::mp::functions::make_tuple(10, 20, 30),
            [&ok](std::size_t i, auto v) { ok = ok and (v == static_cast<int>((i + 1) * 10)); }
        );
        return ok;
    }();
    static_assert(enumerate_ok);

    // fold_left
    static_assert(
        csl::mp::functions::fold_left(csl::mp::functions::make_tuple(1, 2, 3, 4), std::plus<int>{}, 0) == 10
    );

    // all_of / any_of / none_of
    constexpr auto ints = csl::mp::functions::make_tuple(2, 4, 6);
    static_assert(csl::mp::functions::all_of (ints, [](int v) { return v % 2 == 0; }));
    static_assert(csl::mp::functions::any_of (ints, [](int v) { return v == 4; }));
    static_assert(csl::mp::functions::none_of(ints, [](int v) { return v > 10; }));

    // P2445: forward_like
    auto owner = 42;
    static_assert(std::is_same_v<
        decltype(csl::mp::forward_like<int&&>(owner)),
        int&&
    >);
}
