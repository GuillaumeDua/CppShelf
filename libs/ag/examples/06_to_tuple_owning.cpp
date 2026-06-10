#include <csl/ag.hpp>
#include <iostream>
#include <utility>

struct A { int i; float f; };

auto main() -> int {
    constexpr auto value = A{ .i = 42, .f = 0.13f };
    constexpr auto value_as_tuple = csl::ag::to_tuple(std::move(value));

    [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        static_assert((std::same_as<
            csl::ag::element_t<indexes, A>,
            std::tuple_element_t<indexes, std::remove_cvref_t<decltype(value_as_tuple)>>
        > and ...));
        ((std::cout << std::get<indexes>(value_as_tuple) << ' '), ...);
    }(std::make_index_sequence<csl::ag::size_v<A>>{});
}
