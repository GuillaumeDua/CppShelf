#include "./flat.hpp"

#include <csl/mp.hpp>

#include <type_traits>
#include <concepts>
#include <iostream> // debug only

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

    using seq_type = std::make_index_sequence<5>;
    using reversed_seq_type = csl::mp::seq::make_reverse_index_sequence<5>;
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
