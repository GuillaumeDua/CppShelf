#include "../../mp/mp.hpp" // temporary, create real test later ...

#include <iostream> // debug only

auto main() -> int {

    using namespace csl::mp;

    std::cout << "4 == " << index_of_v<bool,
        char, char, int, char, bool, int, bool> << '\n';
    std::cout << "1 == " << rindex_of_v<int,
        char, char, int, bool> << '\n';

    std::cout << "2 == " << index_of_v<int,
        char, char, int, int, bool> << '\n';
    std::cout << "1 == " << rindex_of_v<int,
        char, char, int, int, bool> << '\n';

    std::cout << "-------\n";

    std::cout << "2 == " << index_of_v<int,
        std::tuple<char, char, int, bool>> << '\n';
    std::cout << "1 == " << rindex_of_v<int,
        std::tuple<char, char, int, bool>> << '\n';

    std::cout << "2 == " << index_of_v<int,
        std::tuple<char, char, int, int, bool>> << '\n';
    std::cout << "1 == " << rindex_of_v<int,
        std::tuple<char, char, int, int, bool>> << '\n';
}
