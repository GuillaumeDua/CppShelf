#pragma once
#include <csl/mp.hpp>
#include <cstdint>

// STL tuplelikes
#include <tuple>
#include <array>
#include <utility> // std::pair

// NOLINTBEGIN(*-avoid-magic-numbers)

// concepts
namespace test::tuples::concepts::tuple_like {

    static_assert(csl::mp::concepts::tuple_like<std::array<int, 2>>);
    static_assert(csl::mp::concepts::tuple_like<std::pair<int, float>>);

    static_assert(csl::mp::concepts::tuple_like<std::tuple<>>);
    static_assert(csl::mp::concepts::tuple_like<std::tuple<int> &>);
    static_assert(csl::mp::concepts::tuple_like<std::tuple<int, int> &&>);
    static_assert(csl::mp::concepts::tuple_like<const std::tuple<int, float> &>);

    static_assert(csl::mp::concepts::tuple_like<csl::mp::tuple<>>);
    static_assert(csl::mp::concepts::tuple_like<csl::mp::tuple<int> &>);
    static_assert(csl::mp::concepts::tuple_like<csl::mp::tuple<int, int> &&>);
    static_assert(csl::mp::concepts::tuple_like<const csl::mp::tuple<int, float> &>);
}
namespace test::tuples::concepts::pair_like {

    // std::array
    static_assert(not csl::mp::concepts::pair_like<std::array<int, 1>>);
    static_assert(csl::mp::concepts::pair_like<std::array<int, 2>>);
    static_assert(not csl::mp::concepts::pair_like<std::array<int, 3>>);

    // std::pair
    static_assert(csl::mp::concepts::pair_like<std::pair<int, float>>);

    // std::tuple
    static_assert(not csl::mp::concepts::pair_like<std::tuple<>>);
    static_assert(not csl::mp::concepts::pair_like<std::tuple<int>>);
    static_assert(csl::mp::concepts::pair_like<std::tuple<int, int>>);
    static_assert(csl::mp::concepts::pair_like<std::tuple<int, float>>);

    // tuple
    static_assert(not csl::mp::concepts::pair_like<csl::mp::tuple<>>);
    static_assert(not csl::mp::concepts::pair_like<csl::mp::tuple<int>>);
    static_assert(csl::mp::concepts::pair_like<csl::mp::tuple<int, int>>);
    static_assert(csl::mp::concepts::pair_like<csl::mp::tuple<int, float>>);
}
namespace test::tuples::concepts::sized {

    // empty
    static_assert(csl::mp::concepts::empty<csl::mp::tuple<>>);
    static_assert(csl::mp::concepts::empty<std::tuple<>>);
    static_assert(csl::mp::concepts::empty<std::array<int, 0>>);

    // not_empty
    static_assert(not csl::mp::concepts::empty<std::pair<int, char>>);
    static_assert(csl::mp::concepts::not_empty<std::pair<int, char>>);
    static_assert(not csl::mp::concepts::empty<std::tuple<int>>);
    static_assert(csl::mp::concepts::not_empty<std::tuple<int>>);

    // sized, at_least
    static_assert(csl::mp::concepts::sized<csl::mp::tuple<>, 0>);
    static_assert(csl::mp::concepts::sized<csl::mp::tuple<int>, 1>);
    static_assert(csl::mp::concepts::sized_at_least<csl::mp::tuple<int>, 1>);
    static_assert(not csl::mp::concepts::sized_at_least<csl::mp::tuple<int>, 2>);
    static_assert(csl::mp::concepts::sized<std::pair<int, char>, 2>);
}
namespace test::tuples::concepts::tuple_element {

    // int
    static_assert(not csl::mp::concepts::tuple_element<int, 0>);

    // std::array
    static_assert(csl::mp::concepts::tuple_element<std::array<int, 2>, 0>);
    static_assert(csl::mp::concepts::tuple_element<std::array<int, 2>, 1>);
    static_assert(not csl::mp::concepts::tuple_element<std::array<int, 2>, 2>);

    // std::pair
    static_assert(csl::mp::concepts::tuple_element<std::pair<int, float>, 0>);
    static_assert(not csl::mp::concepts::tuple_element<std::pair<int, float>, 2>);

    // tuple
    using T = csl::mp::tuple<int, float>;
    static_assert(csl::mp::concepts::tuple_element<T, 0>);
    static_assert(csl::mp::concepts::tuple_element<T, 1>);
    static_assert(not csl::mp::concepts::tuple_element<T, 2>);
}

// tuples: details::concepts
namespace test::tuples::concepts::deductible {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    // can_deduce_by_type
    static_assert(csl::mp::details::concepts::can_deduce_by_type<without_duplicates, int>);
    static_assert(not csl::mp::details::concepts::can_deduce_by_type<with_duplicates, int>);

    // can_deduce_by_index
    static_assert(csl::mp::details::concepts::can_deduce_by_index<without_duplicates, 0>);
    static_assert(csl::mp::details::concepts::can_deduce_by_index<with_duplicates, 0>);
}

// tuples: API
namespace test::tuples::size {
    static_assert(0 == csl::mp::tuple<>::size);
    static_assert(1 == csl::mp::tuple<int>::size);
    static_assert(2 == csl::mp::tuple<int, char>::size);
    static_assert(3 == csl::mp::tuple<int, char, bool>::size);
    // duplicates
    static_assert(2 == csl::mp::tuple<int, int>::size);
}
namespace test::tuples::size {
    static_assert(0 == csl::mp::size_v<csl::mp::tuple<>>);
    static_assert(1 == csl::mp::size_v<csl::mp::tuple<int>>);
    static_assert(2 == csl::mp::size_v<csl::mp::tuple<int, char>>);
    static_assert(3 == csl::mp::size_v<csl::mp::tuple<int, char, bool>>);
    // duplicates
    static_assert(2 == csl::mp::size_v<csl::mp::tuple<int, int>>);
}
namespace test::tuples::empty {
    static_assert(csl::mp::empty_v<csl::mp::tuple<>>);
    static_assert(not csl::mp::empty_v<csl::mp::tuple<int>>);

    static_assert(csl::mp::concepts::empty<csl::mp::tuple<>>);
    static_assert(not csl::mp::concepts::empty<csl::mp::tuple<int>>);
}
namespace test::tuples::algorithm::count {
    using t = csl::mp::tuple<int, char, bool, int, double>;
    static_assert(0 == csl::mp::count_v<t, float>);
    static_assert(1 == csl::mp::count_v<t, char>);
    static_assert(2 == csl::mp::count_v<t, int>);

    // empty tuple
    static_assert(0 == csl::mp::count_v<csl::mp::tuple<>, int>);

    static_assert(1 == csl::mp::count_v<std::tuple<int, char>, char>);
}
namespace test::tuples::algorithm::count_if {
    using t = csl::mp::tuple<int, char, bool, double, float>;
    static_assert(3 == csl::mp::count_if_v<t, std::is_integral>);
    static_assert(2 == csl::mp::count_if_v<t, std::is_floating_point>);

    // empty tuple
    using is_int64_t = csl::mp::bind_front<std::is_same, std::int64_t>;
    static_assert(0 == csl::mp::count_if_v<t, is_int64_t::type>);
}
namespace test::tuples::type_gettable {
    static_assert(csl::mp::is_type_gettable_v<csl::mp::tuple<int>, int>);
    static_assert(csl::mp::is_type_gettable_v<csl::mp::tuple<int, char>, int>);
    static_assert(csl::mp::is_type_gettable_v<std::tuple<int>, int>);
    static_assert(csl::mp::is_type_gettable_v<std::tuple<int, char>, int>);
    static_assert(not csl::mp::is_type_gettable_v<csl::mp::tuple<int, int>, int>);
    static_assert(not csl::mp::is_type_gettable_v<std::array<int, 1>, int>);
}
namespace test::tuples::support_get_by_type {

    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(not csl::mp::support_get_by_type_v<with_duplicates>);
    static_assert(csl::mp::support_get_by_type_v<without_duplicates>);
    static_assert(csl::mp::support_get_by_type_v<csl::mp::tuple<int>>);

    static_assert(csl::mp::support_get_by_type_v<std::tuple<>>);
    static_assert(csl::mp::support_get_by_type_v<csl::mp::tuple<>>);
    static_assert(not csl::mp::support_get_by_type_v<std::array<int, 1>>);
    static_assert(not csl::mp::support_get_by_type_v<std::tuple<int, int>>);
    static_assert(csl::mp::support_get_by_type_v<std::tuple<int, char>>);
}
namespace test::tuples::type_gettable {

    static_assert(csl::mp::concepts::index_gettable<csl::mp::tuple<int>, 0>);
    static_assert(not csl::mp::concepts::index_gettable<csl::mp::tuple<int>, 1>);
}
namespace test::tuples::algorithm::uniqued {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(not csl::mp::concepts::uniqued<with_duplicates>);
    static_assert(csl::mp::concepts::uniqued<without_duplicates>);
    static_assert(csl::mp::concepts::uniqued<csl::mp::tuple<>>);
    static_assert(csl::mp::concepts::uniqued<csl::mp::tuple<int>>);
}
namespace test::tuples::algorithm::unfold {
    
    static_assert(std::same_as<
        csl::mp::unfold_t<std::tuple<int, char>, csl::mp::tuple>,
        csl::mp::tuple<int, char>
    >);
    static_assert(std::same_as<
        csl::mp::unfold_t<std::array<int, 2>, csl::mp::tuple>,
        csl::mp::tuple<int, int>
    >);
    static_assert(std::same_as<
        csl::mp::unfold_t<std::array<int, 2>, std::tuple>,
        std::tuple<int, int>
    >);
    static_assert(std::same_as<
        csl::mp::unfold_t<csl::mp::tuple<int, char>, std::tuple>,
        std::tuple<int, char>
    >);
}

// WIP: revert API so it looks like std::ranges
namespace test::tuples::algorithm::contains {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(csl::mp::contains_v<int, with_duplicates>);
    static_assert(csl::mp::contains_v<int, without_duplicates>);

    static_assert(not csl::mp::contains_v<double, with_duplicates>);
    static_assert(not csl::mp::contains_v<double, without_duplicates>);
}
namespace test::tuples::compare::tuple {

    template <
        csl::mp::concepts::tuple_like lhs_t,
        csl::mp::concepts::tuple_like rhs_t
    >
    struct impl {
        constexpr static void common_reference() {

            static_assert(requires{ std::common_reference_t<lhs_t>{}; });
            static_assert(requires{ std::common_reference_t<rhs_t>{}; });

            static_assert(std::same_as<
                csl::mp::tuple_common_reference_t<
                    lhs_t, rhs_t,
                    std::type_identity_t, std::type_identity_t
                >,
                csl::mp::tuple<
                    std::common_reference_t<
                        std::tuple_element_t<0, lhs_t>,
                        std::tuple_element_t<0, rhs_t>
                    >,
                    std::common_reference_t<
                        std::tuple_element_t<1, lhs_t>,
                        std::tuple_element_t<1, rhs_t>
                    >
                >
            >);

            static_assert(
                requires {
                    typename csl::mp::tuple_common_reference_t<
                        lhs_t, rhs_t,
                        std::type_identity_t,
                        std::type_identity_t
                    >;
                }
            );

            static_assert(std::common_reference_with<
                lhs_t, rhs_t
            >);
        }

        constexpr static void equality() {
            static_assert(std::equality_comparable<lhs_t>);
            static_assert(std::equality_comparable<rhs_t>);
            static_assert(requires{
                lhs_t{} == rhs_t{};
            });

            static_assert(lhs_t{} == lhs_t{});
            static_assert(lhs_t{ {}, {} } == lhs_t{ {}, {} });
            static_assert(lhs_t{ {}, 'a' } != lhs_t{ 1.F, 'a'});
            static_assert(lhs_t{ 42, {}  } != lhs_t{ 42.F, 'a'});

            // NOTE: C++23
            static_assert(std::equality_comparable_with<
                lhs_t, rhs_t
            >);

            #if CSL_MP_TUPLE__IMPLICIT_CONVERSION
            [[maybe_unused]] constexpr static auto narrowing = csl::mp::tuple<char>{42};
            static_assert(lhs_t{ 42, 'a' } == lhs_t{ 42, 'a'});
            static_assert(lhs_t{ {}, 'a' } != lhs_t{ 42, 'a'});
            static_assert(lhs_t{ 42, {}  } != lhs_t{ 42, 'a'});
            #endif
        }

        constexpr static void three_way() {
            static_assert(std::three_way_comparable<lhs_t>);
            static_assert(std::three_way_comparable<rhs_t>);
            static_assert(requires{
                lhs_t{} <=> rhs_t{};
            });
            static_assert(std::three_way_comparable_with<
                lhs_t, rhs_t
            >);
            static_assert(lhs_t{ 0.F, {} } < rhs_t{1.F, {} });
            static_assert(lhs_t{ 0.F, {} } < rhs_t{0.F, 1});
        }
    };

    using lhs_t = csl::mp::tuple<float, char>;
    using rhs_t = csl::mp::tuple<double, int>;

    using symetrical_ok  = impl<lhs_t, lhs_t>;
    // #if defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION) \
    //             and CSL_MP_TUPLE__IMPLICIT_CONVERSION
    using asymetrical_ok = impl<lhs_t, lhs_t>;
    // #endif
}
namespace test::tuples::compare::tuplelikes {
    // TODO(Guillaume)
}
namespace test::tuples::fwd_as_tuple {

    constexpr auto c = 'a';
    static_assert(std::is_same_v<
        csl::mp::tuple<int&&, const char&>,
        decltype(csl::mp::forward_as_tuple(42, c))
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<csl::mp::tuple<int>&&>,
        decltype(csl::mp::forward_as_tuple(csl::mp::tuple{42}))
    >);
}
namespace test::tuples::cat {

    // empty
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::cat_result_t<>
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::cat_result_t<csl::mp::tuple<>>
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::cat_result_t<csl::mp::tuple<>, csl::mp::tuple<>>
    >);

    // type
    static_assert(std::is_same_v<
        csl::mp::tuple<int, char, double, float>,
        csl::mp::cat_result_t<
            csl::mp::tuple<int, char>,
            csl::mp::tuple<double, float>
        >
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<int, char, double, float>,
        csl::mp::cat_result_t<
            csl::mp::tuple<>,
            csl::mp::tuple<int>,
            csl::mp::tuple<>,
            csl::mp::tuple<char>,
            csl::mp::tuple<double, float>,
            csl::mp::tuple<>
        >
    >);

    // value
    static_assert(
        csl::mp::tuple<char, int>{'a', 42}
    ==  csl::mp::cat(
            csl::mp::tuple<char>{'a'},
            csl::mp::tuple<int>{42}
        )
    );
}
namespace test::tuples::set_union {
    using T0 = csl::mp::tuple<int, char>;
    using T1 = csl::mp::tuple<int, double>;
    
    static_assert(std::is_same_v<
        csl::mp::set_union_t<T0, T1>,
        csl::mp::tuple<int, char, double>
    >);
}
namespace test::tuples::set_intersection {
    using T0 = csl::mp::tuple<int, char>;
    using T1 = csl::mp::tuple<int, double>;
    
    static_assert(std::is_same_v<
        csl::mp::set_intersection_t<T0, T1>,
        csl::mp::tuple<int>
    >);
}
namespace test::tuples::indexes {
    using invalid_tuple = csl::mp::tuple<char, double, float, int, int>;
    static_assert(csl::mp::index_of_v<invalid_tuple, int> == 3);
    static_assert(csl::mp::last_index_of_v<invalid_tuple, int> == 4);

    using valid_tuple = csl::mp::tuple<char, double, float, int>;
    static_assert(csl::mp::index_of_v<valid_tuple, int> == 3);
    static_assert(csl::mp::last_index_of_v<valid_tuple, int> == 3);
}
// TODO(Guillaume) sort, is_sorted
// TODO(Guillaume) unique/deduplicate
namespace test::tuples::is_uniqued {
    using valid_tuple = csl::mp::tuple<int, char>;
    static_assert(csl::mp::is_uniqued_v<valid_tuple>);

    using invalid_tuple = csl::mp::tuple<int, char, int>;
    static_assert(not csl::mp::is_uniqued_v<invalid_tuple>);
}
namespace test::tuples::filter {
    using T1 = csl::mp::tuple<int, double, char, float>;
    using filtered_integrals = csl::mp::filter_t<T1, std::is_integral>;
    using filtered_floating = csl::mp::filter_t<T1, std::is_floating_point>;

    using expected_integrals = csl::mp::tuple<int, char>;
    using expected_floating = csl::mp::tuple<double, float>;

    static_assert(std::is_same_v<filtered_integrals, expected_integrals>);
    static_assert(std::is_same_v<filtered_floating, expected_floating>);
}
namespace test::tuples::deduplicate {
    using valid_tuple = csl::mp::tuple<int, char>;
    using invalid_tuple = csl::mp::tuple<int, char, int>;
    
    static_assert(std::is_same_v<valid_tuple,
        csl::mp::deduplicate_t<valid_tuple>
    >);
    static_assert(std::is_same_v<
        valid_tuple,
        csl::mp::deduplicate_t<invalid_tuple>
    >);
}

// construction
namespace test::tuples::storage::constructors::default_ {
    using type = csl::mp::tuple<int, char>;
    [[maybe_unused]] constexpr auto value = type{};
}
namespace test::tuples::storage::constructors::value {
    using type = csl::mp::tuple<int, char>;
    [[maybe_unused]] constexpr auto value = type{ 42, 'a' };
}
namespace test::tuples::storage::constructors::copy {
    using type = csl::mp::tuple<int, char>;
    constexpr auto value = type{ 42, 'a' };
    [[maybe_unused]] constexpr auto copy = value;
    static_assert(value == copy);
}

#include <string_view>
namespace test::tuples::storage::constructors::move {
    using type = csl::mp::tuple<int, char, std::string_view>;
    [[maybe_unused]] constexpr auto moved_to = []{
        auto tmp = type{ 42, 'a', std::string_view{} };
        auto value = std::move(tmp); // NOLINT(performance-move-const-arg)
        return value;
    }();
}
namespace test::tuples::storage::constructors::conversion::safe {

    #if CSL_MP_TUPLE__IMPLICIT_CONVERSION >= CSL_MP_TUPLE__IMPLICIT_CONVERSION_SAFE
    [[maybe_unused]] constexpr csl::mp::tuple<std::int32_t>                 d = csl::mp::tuple<std::int8_t>{};
    [[maybe_unused]] constexpr csl::mp::tuple<std::int32_t, std::int64_t>   e = csl::mp::tuple<std::int8_t, std::int8_t>{};
    [[maybe_unused]] constexpr csl::mp::tuple<double>                       f { float{} };
    #endif
}
namespace test::tuples::storage::constructors::conversion::unsafe {
    #if CSL_MP_TUPLE__IMPLICIT_CONVERSION == CSL_MP_TUPLE__IMPLICIT_CONVERSION_UNSAFE
    [[maybe_unused]] constexpr csl::mp::tuple<int, char>    a = csl::mp::tuple<double, int>{ .0 , 0 };
    [[maybe_unused]] constexpr csl::mp::tuple<int>          b { .0F };
    [[maybe_unused]] constexpr csl::mp::tuple<float>        c { .0 };
    #endif
}
namespace test::tuples::storage::of_refs {
    using type = csl::mp::tuple<int&&, const char &>;

    [[maybe_unused]] static void impl() {
        const auto c = 'a';
        auto i = 42;
        [[maybe_unused]] auto value = type{ std::move(i), c }; // NOLINT(performance-move-*)
    }
}

namespace test::tuples::deduction_guide {
    static_assert(std::same_as<
        csl::mp::tuple<>,
        decltype(csl::mp::tuple{})
    >);
    static_assert(std::same_as<
        csl::mp::tuple<int, char>,
        decltype(csl::mp::tuple{42, 'a'})
    >);
}
namespace test::tuples::get {
    using type = csl::mp::tuple<int, char>;
    constexpr auto value = type{ 42, 'a' };

    static_assert(42  == value.template get<0>());
    static_assert('a' == value.template get<1>());
}
namespace test::tuples::get::cvref {

    using tuple_element_t = csl::mp::element_t<0, type>;

    using expecting_lvalue = decltype(std::declval<type &>().get<0>());
    static_assert(std::same_as<tuple_element_t &, expecting_lvalue>);

    using expecting_rvalue = decltype(std::declval<type &&>().get<0>());
    static_assert(std::same_as<tuple_element_t &&, expecting_rvalue>);

    using expecting_const_lvalue = decltype(std::declval<const type &>().get<0>());
    static_assert(std::same_as<const tuple_element_t &, expecting_const_lvalue>);

    using expecting_const_rvalue = decltype(std::declval<const type &&>().get<0>());
    static_assert(std::same_as<const tuple_element_t &&, expecting_const_rvalue>);
}
namespace test::tuples::get::cvref::consistency {

    static_assert(std::same_as<
        decltype(std::declval<type &>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<type &>()))
    >);
    static_assert(std::same_as<
        decltype(std::declval<type &&>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<type &&>()))
    >);
    static_assert(std::same_as<
        decltype(std::declval<const type &>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<const type &>()))
    >);
    static_assert(std::same_as<
        decltype(std::declval<const type &&>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<const type &&>()))
    >);
}

// std::tuple interface/inter-operatiblity
namespace test::tuples::std_interopterability::tuple_size {
    using valid_tuple = csl::mp::tuple<int, char>;
    using invalid_tuple = csl::mp::tuple<int, char, int>;

    static_assert(std::tuple_size_v<valid_tuple> == 2);
    static_assert(std::tuple_size_v<invalid_tuple> == 3);
}
namespace test::tuples::std_interopterability::tuple_element {
    using valid_tuple = csl::mp::tuple<int, char>;
    static_assert(std::is_same_v<int,  std::tuple_element_t<0, valid_tuple>>);
    static_assert(std::is_same_v<char, std::tuple_element_t<1, valid_tuple>>);

    using invalid_tuple = csl::mp::tuple<int, char, int>;
    static_assert(std::is_same_v<int,  std::tuple_element_t<0, invalid_tuple>>);
    static_assert(std::is_same_v<char, std::tuple_element_t<1, invalid_tuple>>);
    static_assert(std::is_same_v<int,  std::tuple_element_t<0, invalid_tuple>>);
}
namespace test::tuples::std_interopterability::get {

    using std::get; // NOTE: std::get is not a customization point

    using type = csl::mp::tuple<int, char>;
    constexpr auto value = type{ 42, 'a' };

    static_assert(42  == get<0>(value));
    static_assert('a' == get<1>(value));
}

// ADL
namespace test::tuples::get::ADL {

    using csl::mp::get;

    using type = csl::mp::tuple<int, char>;
    constexpr auto value = type{ 42, 'a' };

    static_assert(42 == get<0>(value));
    static_assert('a' == get<1>(value));
}

namespace test::tuples::get::square_bracket_op {

    using type = csl::mp::tuple<int, char>;
    constexpr auto value = type{ 42, 'a' };
    
    static_assert(42 == value[csl::mp::index_t<0>{}]);
    static_assert(42 == value[csl::mp::index<0>]);

    namespace literals {
        using namespace csl::mp::literals;
        static_assert(42 == value[0_index]);
    }
}

// structured binding
namespace test::tuples::structured_binding {

    using type = csl::mp::tuple<int, char>;

    constexpr void lvalue(){
        constexpr auto value = type{};
        [[maybe_unused]] auto & [i, c] = value; // NOLINT(*-qualified-auto)
    }
    constexpr void const_lvalue(){
        constexpr auto value = type{};
        [[maybe_unused]] const auto & [i, c] = value;
    }
    constexpr void rvalue(){
        [[maybe_unused]] auto && [i, c] = type{};
    }
    constexpr void const_rvalue(){
        [[maybe_unused]] const auto && [i, c] = type{};
    }
}

// NOLINTEND(*-avoid-magic-numbers)
