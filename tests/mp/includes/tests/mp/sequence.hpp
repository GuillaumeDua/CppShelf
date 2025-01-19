#pragma once
#include <csl/mp.hpp>

// sequences
namespace test::reverse_integer_sequence {
    
    using namespace csl::mp::seq;

    using seq_type = std::make_index_sequence<5>;                           // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    using reversed_seq_type = csl::mp::seq::make_reverse_index_sequence<5>; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    static_assert(std::same_as<
        reversed_seq_type,
        std::index_sequence<4,3,2,1,0>
    >);
    static_assert(std::same_as<
        reverse_sequence<reversed_seq_type>,
        std::index_sequence<0,1,2,3,4>
    >);
    static_assert(std::same_as<
        reversed_seq_type,
        reverse_sequence<std::index_sequence<0,1,2,3,4>>
    >);
}
