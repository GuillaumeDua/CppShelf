#include <csl/ag.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

struct type_0{ int i = 0; char c = 'a'; };
struct type_1{ char c = 'c'; type_0 t; };
struct type_2{ bool b = true; type_1 t; std::tuple<int, char> tu = { 2, 'b'}; std::array<char, 3> a = {'a', 'b', 'c'}; std::pair<int, int> p = { 42, 43 }; }; // NOLINT
struct type_3{ int i = 0; char c = 'a'; };
struct type_4{ const int i = 0; char & c; };

auto & operator<<(std::ostream & os, type_3) { return os << "type_3 : user-defined operator<<(std::ostream&, const T &)\n"; }

auto main() -> int {

    using namespace csl::ag::io;

    char c = 'a';

    std::cout
        << type_0{} << "\n-----\n"
        << type_1{} << "\n-----\n"
        << type_2{} << "\n-----\n"
        << type_3{} << "\n-----\n"
        << type_4{ 42, c } << "\n-----\n"
        // << std::tuple{42, 'a'} << "-----\n"
    ;

    // {
    //     auto value = toto{ 42, 'a' };
    //     [[maybe_unused]] auto && [ v0, v1 ] = value;
    //     print(value);
    // }

    {
        auto value = type_0{ 42, 'A' }; // NOLINT
        /*constexpr*/ auto as_tuple = csl::ag::as_tuple(value); // WTF not a constant expression ???
    }

    // static_assert(std::same_as<
    //     int&,
    //     std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
    // >);
    // static_assert(std::same_as<
    //     char&,
    //     std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
    // >);

    // // /*static_*/assert(csl::ag::get<0>(value) == 42);
    // // /*static_*/assert(csl::ag::get<1>(value) == 'A');

    // assert(
    //     std::addressof(std::get<0>(as_tuple)) ==
    //     std::addressof(value.i)
    // );
    // assert(
    //     std::addressof(std::get<1>(as_tuple)) ==
    //     std::addressof(value.c)
    // );

    // // // print(value);

    // struct qwe{ int i; char & c; bool && b; };
    // // static_assert(std::tuple_size<qwe>::value == 3);
    // static_assert(std::same_as<
    //     int,
    //     csl::ag::element<0, qwe>::type
    // >);
    // static_assert(std::same_as<
    //     char &,
    //     csl::ag::element<1, qwe>::type
    // >);
    // static_assert(std::same_as<
    //     bool &&,
    //     csl::ag::element<2, qwe>::type
    // >);
}
