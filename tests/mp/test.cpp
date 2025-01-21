#include <csl/mp.hpp>

// parts
#include <tests/mp/tuple.hpp>
#include <tests/mp/sequence.hpp>
// #include <tests/mp/flat.hpp>

// pack: legacy algos to refactor/move
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

// #include <iostream> // debug only
auto main() -> int {

    // using namespace csl::mp;
    // std::cout << "1 == " << last_index_of_v<int,
    //     char, char, int, bool> << '\n';

    
}
