// #include "./flat.hpp"

#include <csl/mp.hpp>

#include <cstdint>
#include <type_traits>
#include <concepts>
#include <iostream> // debug only

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
namespace testi::tuples::is_valid {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates = csl::mp::tuple<int, char, int>;

    static_assert(not csl::mp::is_valid_v<with_duplicates>);
    static_assert(csl::mp::is_valid_v<without_duplicates>);
    static_assert(csl::mp::is_valid_v<csl::mp::tuple<>>);
    static_assert(csl::mp::is_valid_v<csl::mp::tuple<int>>);
}
namespace test::tuples::tuple_cat {
    using t1 = csl::mp::tuple<int, char>;
    using t2 = csl::mp::tuple<double, float>;

    using t3_expected = csl::mp::tuple<int, char, double, float>;
    using t3 = csl::mp::tuple_cat_result_t<t1, t2>;
    static_assert(std::is_same_v<t3, t3_expected>);

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
    using T1 = csl::mp::tuple<int, char>;
    using T2 = csl::mp::tuple<int, double>;
    
    static_assert(std::is_same_v<
        csl::mp::set_union_t<T1, T2>,
        csl::mp::tuple<int, char, double>
    >);
}
namespace test::tuples::set_intersection {
    using T1 = csl::mp::tuple<int, char>;
    using T2 = csl::mp::tuple<int, double>;
    
    static_assert(std::is_same_v<
        csl::mp::set_intersection_t<T1, T2>,
        csl::mp::tuple<int>
    >);
}


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
