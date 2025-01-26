#pragma once
#include <csl/mp.hpp>

// sequences
namespace test::reverse_::ordered_ {
    
    using namespace csl::mp::seq;

    using type = std::make_index_sequence<5>;                             // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    using reversed_type = csl::mp::seq::make_reverse_index_sequence<5>; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static_assert(std::same_as<
        reversed_type,
        std::index_sequence<4,3,2,1,0>
    >);
    static_assert(std::same_as<
        reverse_t<reversed_type>,
        std::index_sequence<0,1,2,3,4>
    >);
    static_assert(std::same_as<
        reversed_type,
        reverse_t<std::index_sequence<0,1,2,3,4>>
    >);
}
namespace test::reverse_::unordered_ {
    using namespace csl::mp::seq;

    using type = std::index_sequence<4,1,0,3,2>;                           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static_assert(std::same_as<
        reverse_t<type>,
        std::index_sequence<2,3,0,1,4>
    >);
}

namespace test::at_ {

}
namespace test::get_ {

}