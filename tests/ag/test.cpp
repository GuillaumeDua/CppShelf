#include <csl/ag.hpp>

#include <iostream>
#include <cassert>
#include <memory>

void print(csl::ag::concepts::aggregate auto && value) {
    // TODO : recursivity or std::formatter

    // std::cout << std::setw(10) << gcl::cx::type_name_v<decltype(value)>;

    auto as_tuple = csl::ag::as_tuple(std::forward<decltype(value)>(value));
    using as_tuple_t = std::remove_cvref_t<decltype(as_tuple)>;
    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((
            std::cout
                << '[' << indexes << "] : "
                << std::get<indexes>(std::forward<decltype(as_tuple)>(as_tuple))
                << '\n'
            // gcl::cx::type_name_v<std::tuple_element_t<indexes, as_tuple_t>>,
        ), ...);  
    }(std::make_index_sequence<std::tuple_size_v<as_tuple_t>>{});
}

struct toto{ int i; char c; };

auto main() -> int {
    using namespace csl::ag;

    // TODO : wrap that in a test function

    auto value = toto{ 42, 'A' }; // NOLINT
    auto as_tuple = csl::ag::as_tuple(value);

    static_assert(std::same_as<
        int&,
        std::tuple_element_t<0, std::remove_cvref_t<decltype(as_tuple)>>
    >);
    static_assert(std::same_as<
        char&,
        std::tuple_element_t<1, std::remove_cvref_t<decltype(as_tuple)>>
    >);

    assert(
        std::addressof(std::get<0>(as_tuple)) ==
        std::addressof(value.i)
    );
    assert(
        std::addressof(std::get<1>(as_tuple)) ==
        std::addressof(value.c)
    );

    print(value);
}
