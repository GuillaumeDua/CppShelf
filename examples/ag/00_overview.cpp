// @awesome-doc-code-sections::CE={
//  "language"            : "c++",
//  "includes_transformation" : [
//        [ "csl/",       "https://raw.githubusercontent.com/GuillaumeDua/CppShelf/main/includes/ag/csl/" ]
//  ],
//  "add_in_doc_execution" : true
//  }

#include "../includes/ag/csl/ag.hpp"

struct S { char c; int i; };

static_assert(
    csl::ag::concepts::aggregate<S> and
    csl::ag::size_v<S> == 2
);
static_assert(std::same_as<char,  csl::ag::element_t<0, S>>);
static_assert(std::same_as<int,   csl::ag::element_t<1, S>>);

auto main() -> int {

    S value{ 'A', 41 };
    ++std::get<1>(value);

    using namespace csl::ag::io;
    std::cout << "value: " << value << '\n';
    // (wip) compatibility with `fmt` and `std::print` will be available soon
}

// @awesome-doc-code-sections:expected_output position=right

// value: S& : {
//    [0] char : A
//    [1] int : 42
// }
