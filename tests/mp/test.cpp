// #include "./flat.hpp"

#include <bits/utility.h>
#include <csl/mp.hpp>

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <iostream> // debug only

// tuples: details
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
namespace test::tuples::tuple_cat {
    using T0 = csl::mp::tuple<int, char>;
    using T1 = csl::mp::tuple<double, float>;

    using T2_expected = csl::mp::tuple<int, char, double, float>;
    using T2 = csl::mp::tuple_cat_result_t<T0, T1>;
    static_assert(std::is_same_v<T2, T2_expected>);

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

// std::tuple interface/inter-operatiblity
namespace test::tuples::std_interopterability {
    using valid_tuple = csl::mp::tuple<int, char>;
    using invalid_tuple = csl::mp::tuple<int, char, int>;

    static_assert(std::tuple_size_v<valid_tuple> == 2);
    static_assert(std::tuple_size_v<invalid_tuple> == 3);
}

// sequences
namespace test::reverse_integer_sequence {
    
    using namespace csl::mp::seq;

    using seq_type = std::make_index_sequence<5>;                           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    using reversed_seq_type = csl::mp::seq::make_reverse_index_sequence<5>; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static_assert(std::same_as<
        reversed_seq_type,
        std::index_sequence<4,3,2,1,0>
    >);
    static_assert(std::same_as<
        reverse_sequence<reversed_seq_type>,
        std::index_sequence<0,1,2,3,4>
    >);
    static_assert(std::same_as<
        reversed_seq_type,
        reverse_sequence<std::index_sequence<0,1,2,3,4>>
    >);
}
#if false
namespace test::index_of {

    using namespace csl::mp;

    using pack_type = pack<double, char, int, float, int, bool>;
    // static_assert(2 ==       index_of_v<int, pack_type>);
    // static_assert(2 == first_index_of_v<int, pack_type>);
    // static_assert(1 ==      rindex_of_v<int, pack_type>);
    // static_assert(1 ==  last_index_of_v<int, pack_type>);
}

namespace test::nth {
    using pack_type = csl::mp::pack<int, char, double, bool, float>;
    static_assert(std::is_same_v<
        csl::mp::nth_t<1, pack_type>,
        char
    >);
    static_assert(
        csl::mp::index_of_v<char, pack_type> == 1
    );
}
#endif
// namespace test::index {
//     using namespace csl::mp;

//     static_assert(4 == index_of_v<bool,
//         char, char, int, char, bool, int, bool>);
//     static_assert(1 == last_index_of_v<int,
//         char, char, int, bool>);

//     static_assert(2 == index_of_v<int,
//         char, char, int, int, bool>);
//     static_assert(1 == last_index_of_v<int,
//         char, char, int, int, bool>);

//     static_assert(2 == index_of_v<int,
//         pack<char, char, int, bool>>);
//     static_assert(1 == last_index_of_v<int,
//         pack<char, char, int, bool>>);

//     static_assert(2 == index_of_v<int,
//         pack<char, char, int, int, bool>>);
//     static_assert(1 == last_index_of_v<int,
//         pack<char, char, int, int, bool>>);
// }

auto main() -> int {

    // using namespace csl::mp;
    // std::cout << "1 == " << last_index_of_v<int,
    //     char, char, int, bool> << '\n';

    
}
