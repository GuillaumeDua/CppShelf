#include <csl/colors.hpp>
#include <iostream>

auto main() -> int {

    using namespace csl::colors;

    std::cout
        << foreground<color::red>{} << "toto" << modifier::reset << " ahahah"
        << '\n'
    ;

    std::cout
        << scoped_theme<foreground<color::green>, background<color::black>>{}
        << "hello, world\n"
    ;

}