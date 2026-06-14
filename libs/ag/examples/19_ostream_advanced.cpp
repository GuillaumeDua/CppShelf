#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1

#include <csl/ag.hpp>
#include <array>
#include <iostream>
#include <string>
#include <tuple>

struct A { int i; float f; };
struct B {};

// Note: must be in the same namespace as B (global here) so ADL finds it
auto & operator<<(std::ostream & os, B) {
    return os << "user-defined operator<<(std::ostream&, const B &)";
}

struct C {
    A a;
    B b;
    int & i;
    const std::string str;
    char && c;
    std::tuple<bool, int> t{ true, 2 };
    std::array<char, 3> arr{ 'a', 'b', 'c' };
};

auto main() -> int {
    using namespace csl::ag::io;

    int i = 42;
    char c = 'c';
    auto value = C{
        .a = A{ .i=13, .f=.12F },
        .b = B{},
        .i = i, .str = "str", .c = std::move(c)
    };
    std::cout << value;
}
