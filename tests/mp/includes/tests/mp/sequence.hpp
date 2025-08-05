#pragma once
#include <csl/mp.hpp>

// NOLINTBEGIN(*-avoid-magic-numbers)

namespace test::seq::is_sequence_ {
    using type = std::make_index_sequence<5>;

    static_assert(csl::mp::seq::is_sequence_v<type>);

    static_assert(csl::mp::seq::concepts::sequence<type>);
    static_assert(csl::mp::seq::concepts::sequence<type &>);
    static_assert(csl::mp::seq::concepts::sequence<type &&>);
    static_assert(csl::mp::seq::concepts::sequence<const type &>);
}

namespace test::seq::size {
    using type = std::make_index_sequence<5>;

    static_assert(csl::mp::seq::size_v<type> == 5);
    static_assert(csl::mp::seq::size_v<type> == type::size());
    
    static_assert(csl::mp::seq::concepts::empty<std::index_sequence<>>);
    static_assert(not csl::mp::seq::concepts::empty<type>);
    static_assert(csl::mp::seq::concepts::not_empty<type>);
    static_assert(csl::mp::seq::concepts::sized<type, 5>);
    static_assert(csl::mp::seq::concepts::sized_at_least<type, 4>);
    static_assert(csl::mp::seq::concepts::sized_at_least<type, 5>);
}

namespace test::seq::reverse_::ordered_ {
    
    using namespace csl::mp::seq;

    using type = std::make_index_sequence<5>;
    using reversed_type = csl::mp::seq::make_reverse_index_sequence<5>;
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
namespace test::seq::reverse_::unordered_ {
    using namespace csl::mp::seq;

    using type = std::index_sequence<4,1,0,3,2>;
    static_assert(std::same_as<
        reverse_t<type>,
        std::index_sequence<2,3,0,1,4>
    >);
}

namespace test::seq::to_tuplelike_ {
    using type = std::make_index_sequence<2>;

    static_assert(std::same_as<
        std::array<std::size_t, 2>,
        csl::mp::seq::to_tuplelike_t<type>
    >);
}

namespace test::seq::at_ {

}
namespace test::seq::get_ {

}

// NOLINTEND(*-avoid-magic-numbers)
