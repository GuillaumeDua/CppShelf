#pragma once
#include <csl/mp.hpp>
#include <cstdint>

// NOLINTBEGIN(*-avoid-magic-numbers)

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
    static_assert(0 == csl::mp::tuple_size_v<csl::mp::tuple<>>);
    static_assert(1 == csl::mp::tuple_size_v<csl::mp::tuple<int>>);
    static_assert(2 == csl::mp::tuple_size_v<csl::mp::tuple<int, char>>);
    static_assert(3 == csl::mp::tuple_size_v<csl::mp::tuple<int, char, bool>>);
    // duplicates
    static_assert(2 == csl::mp::tuple_size_v<csl::mp::tuple<int, int>>);
}
namespace test::tuples::empty {
    static_assert(csl::mp::empty_v<csl::mp::tuple<>>);
    static_assert(not csl::mp::empty_v<csl::mp::tuple<int>>);

    static_assert(csl::mp::concepts::empty_tuple<csl::mp::tuple<>>);
    static_assert(not csl::mp::concepts::empty_tuple<csl::mp::tuple<int>>);
}
namespace test::tuples::count {
    using t = csl::mp::tuple<int, char, bool, int, double>;
    static_assert(0 == csl::mp::count_v<float, t>);
    static_assert(1 == csl::mp::count_v<char, t>);
    static_assert(2 == csl::mp::count_v<int, t>);

    // empty tuple
    static_assert(0 == csl::mp::count_v<int, csl::mp::tuple<>>);
}
namespace test::tuples::count_if {
    using t = csl::mp::tuple<int, char, bool, double, float>;
    static_assert(3 == csl::mp::count_if_v<std::is_integral, t>);
    static_assert(2 == csl::mp::count_if_v<std::is_floating_point, t>);

    // empty tuple
    using is_int64_t = csl::mp::bind_front<std::is_same, std::int64_t>;
    static_assert(0 == csl::mp::count_if_v<is_int64_t::type, t>);
}
namespace test::tuples::contains {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(csl::mp::contains_v<int, with_duplicates>);
    static_assert(csl::mp::contains_v<int, without_duplicates>);

    static_assert(not csl::mp::contains_v<double, with_duplicates>);
    static_assert(not csl::mp::contains_v<double, without_duplicates>);
}
namespace test::tuples::has_duplicates {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(csl::mp::has_duplicates_v<with_duplicates>);
    static_assert(not csl::mp::has_duplicates_v<without_duplicates>);
    static_assert(not csl::mp::has_duplicates_v<csl::mp::tuple<>>);
    static_assert(not csl::mp::has_duplicates_v<csl::mp::tuple<int>>);
}
namespace testi::tuples::is_valid_tuple {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(not csl::mp::is_valid_tuple_v<with_duplicates>);
    static_assert(csl::mp::is_valid_tuple_v<without_duplicates>);
    static_assert(csl::mp::is_valid_tuple_v<csl::mp::tuple<>>);
    static_assert(csl::mp::is_valid_tuple_v<csl::mp::tuple<int>>);
}
namespace test::tuples::compare {

    // tuple_element_storage
    // static_assert(std::three_way_comparable<
    //     csl::mp::details::tuple_element_storage<0, int>
    // >);
    // static_assert(std::three_way_comparable<
    //     csl::mp::details::tuple_element_storage<1, float>
    // >);
    // static_assert(std::three_way_comparable_with<
    //     csl::mp::details::tuple_element_storage<0, int>,
    //     csl::mp::details::tuple_element_storage<1, float>
    // >);

    using lhs_t = csl::mp::tuple<int, char>;
    using rhs_t = csl::mp::tuple<double, int>;
    
    constexpr auto qwe = std::tuple<int, char>{ {}, {} };

    // equality
    static_assert(std::equality_comparable<lhs_t>);
    static_assert(std::equality_comparable<rhs_t>);
    static_assert(requires{
        lhs_t{} == rhs_t{};
    });
    static_assert(lhs_t{} == lhs_t{});
    static_assert(lhs_t{ 42, 'a' } == lhs_t{ 42, 'a'});
    // WIP
    static_assert(lhs_t{ {}, 'a' } != lhs_t{ 42, 'a'});
    static_assert(lhs_t{ 42, {}  } != lhs_t{ 42, 'a'});
    // static_assert(std::equality_comparable_with<
    //     lhs_t, rhs_t
    // >);

    // spaceship
    static_assert(std::three_way_comparable<lhs_t>);
    static_assert(std::three_way_comparable<rhs_t>);
    static_assert(requires{
        lhs_t{} <=> rhs_t{};
    });
    // static_assert(std::three_way_comparable_with<
    //     lhs_t, rhs_t
    // >);
}
namespace test::tuples::tuple_cat {

    // empty
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::tuple_cat_result_t<>
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::tuple_cat_result_t<csl::mp::tuple<>>
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::tuple_cat_result_t<csl::mp::tuple<>, csl::mp::tuple<>>
    >);

    // type
    static_assert(std::is_same_v<
        csl::mp::tuple<int, char, double, float>,
        csl::mp::tuple_cat_result_t<
            csl::mp::tuple<int, char>,
            csl::mp::tuple<double, float>
        >
    >);

    // value
    static_assert(
        csl::mp::tuple<char, int>{'a', 42}
    ==  csl::mp::tuple_cat(
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
    static_assert(csl::mp::first_index_of_v<int, invalid_tuple> == 3);
    static_assert(csl::mp::rfirst_index_of_v<int, invalid_tuple> == 1);
    static_assert(csl::mp::last_index_of_v<int, invalid_tuple> == 4);

    using valid_tuple = csl::mp::tuple<char, double, float, int>;
    static_assert(csl::mp::index_of_v<int, valid_tuple> == 3);
    static_assert(csl::mp::first_index_of_v<int, valid_tuple> == 3);
    static_assert(csl::mp::rfirst_index_of_v<int, valid_tuple> == 0);
    static_assert(csl::mp::last_index_of_v<int, valid_tuple> == 3);
}
namespace test::tuples::is_unique {
    using valid_tuple = csl::mp::tuple<int, char>;
    static_assert(csl::mp::is_unique_v<int, valid_tuple>);
    static_assert(csl::mp::is_unique_v<char, valid_tuple>);

    using invalid_tuple = csl::mp::tuple<int, char, int>;
    static_assert(not csl::mp::is_unique_v<int, invalid_tuple>);
    static_assert(csl::mp::is_unique_v<char, invalid_tuple>);
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
namespace test::tuples::storage::constructors::move {
    using type = csl::mp::tuple<int, char, std::string_view>;
    [[maybe_unused]] constexpr auto moved_to = []{
        auto tmp = type{ 42, 'a', std::string_view{} };
        auto value = std::move(tmp); // NOLINT(performance-move-const-arg)
        return value;
    }();
}
namespace test::tuples::storage::constructors::convertion {
    #if defined(CSL_MP_TUPLE__ALLOW_CONVERSION) and CSL_MP_TUPLE__ALLOW_CONVERSION
    [[maybe_unused]] constexpr csl::mp::tuple<int, char>    a = csl::mp::tuple<double, int>{ .42 , 42 };
    [[maybe_unused]] constexpr csl::mp::tuple<int>          b { .42f };
    #endif
    [[maybe_unused]] constexpr csl::mp::tuple<std::int32_t>                 c = csl::mp::tuple<std::int8_t>{};
    [[maybe_unused]] constexpr csl::mp::tuple<std::int32_t, std::int64_t>   d = csl::mp::tuple<std::int8_t, std::int8_t>{};
    [[maybe_unused]] constexpr csl::mp::tuple<double>                       e { float{} };
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

// std::tuple interface/inter-operatiblity
#include <tuple>
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
    // NOTE: std::get is not a customization point
    using std::get;

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
