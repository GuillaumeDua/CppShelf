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

// ---  WIP ---

namespace test::ag::size_ {
    template <typename T>
    concept is_well_formed = 
        csl::ag::concepts::aggregate<T>
    and (csl::ag::size_v<T> == std::tuple_size_v<csl::ag::to_tuple_t<T>>)
    and (csl::ag::size_v<T> == std::tuple_size_v<csl::ag::view_t<T>>)
    ;
}

auto main() -> int {

    int i = 42;
    using type = test::ag::type<int>;
    auto value = type{
        i, i, std::move(i), i, i, std::move(i)
    };
    
    static_assert(test::ag::size_::is_well_formed<type>);
}
