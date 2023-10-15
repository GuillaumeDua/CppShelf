#include <type_traits>
#if defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# undef CSL_AG_ENABLE_BITFIELDS_SUPPORT
#endif
#include <csl/ag.hpp>
#if defined(CSL_AG_ENABLE_BITFIELDS_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG_ENABLE_BITFIELDS_SUPPORT=OFF"
#endif

#include "includes/tests/ag/details/mp.hpp"
#include "includes/tests/ag/size.hpp"
#include "includes/tests/ag/conversion/to_tuple.hpp"
#include "includes/tests/ag/conversion/to_tuple_view.hpp"

#include <iostream>
#include <iomanip>
#include <cassert>


void print(csl::ag::concepts::aggregate auto && value){

    using type = std::remove_cvref_t<decltype(value)>;

    std::cout
        << "--------------------------------------------\n"
        << "| " << gcl::cx::type_name_v<decltype(value)> << '\n'
        << "--------------------------------------------\n"
    ;

    [&]<
        typename ... Ts,
        std::size_t ... indexes
    >(
        std::type_identity<std::tuple<Ts...>>,
        std::index_sequence<indexes...>
    ){
        ((
            std::cout
                << std::left
                << std::setw(12) << gcl::cx::type_name_v<csl::ag::element_t<indexes, type>>
                << " -> "
                << std::setw(12) << gcl::cx::type_name_v<decltype(csl::ag::get<indexes>(csl_fwd(value)))>
                << " -> "
                << std::setw(12) << gcl::cx::type_name_v<decltype(csl::ag::get<Ts>(csl_fwd(value)))>
                << "\n"
        ), ...);
    }(
        std::type_identity<csl::ag::to_tuple_t<type>>{},
        std::make_index_sequence<csl::ag::size_v<type>>{}
    );
}

auto main() -> int {}
