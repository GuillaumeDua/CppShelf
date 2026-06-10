#include <csl/ag.hpp>
#include <iostream>
#include <utility>

auto main() -> int {
    struct A { int & i; float && f; };
    int i = 42; float f = .13f;
    auto value = csl::ag::to_tuple(A{ .i = i, .f = std::move(f) });

    [&value]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        ((std::cout << std::get<indexes>(value) << ' '), ...);
    }(std::make_index_sequence<csl::ag::size_v<A>>{});

    static_assert(std::same_as<
        int&,
        std::tuple_element_t<0, std::remove_cvref_t<decltype(value)>>
    >);
    static_assert(std::same_as<
        float&&,
        std::tuple_element_t<1, std::remove_cvref_t<decltype(value)>>
    >);
}
