#pragma once

#include <csl/mp.hpp>

// WIP

namespace test::type_traits::type_identity {
    static_assert(std::same_as<int, csl::mp::type_identity_t<int>>);
}
namespace test::type_traits::unwrap_reference {
    static_assert(std::same_as<int, csl::mp::unwrap_reference_t<int>>);
    static_assert(std::same_as<int&, csl::mp::unwrap_reference_t<std::reference_wrapper<int>>>);
    static_assert(not std::same_as<int, csl::mp::unwrap_reference_t<int&>>);

    static_assert(std::same_as<int, csl::mp::unwrap_ref_decay_t<int>>);
    static_assert(std::same_as<int&, csl::mp::unwrap_ref_decay_t<std::reference_wrapper<int>>>);
    static_assert(std::same_as<int, csl::mp::unwrap_ref_decay_t<int&>>);
}
namespace test::concepts::fwd_ref {
    static_assert(csl::mp::concepts::fwd_ref<int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<int&&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&&, int>);

    static_assert(not csl::mp::concepts::fwd_ref<int&, int&>);
}
#include <string>
namespace test::concepts::instance {
    static_assert(csl::mp::concepts::instance<std::vector<int>, std::vector>);
    static_assert(csl::mp::concepts::instance<std::string, std::basic_string>);
    static_assert(csl::mp::concepts::instance<const std::string &, std::basic_string>);
}

namespace test::primitives::value_type {
    static_assert(std::same_as<char, csl::mp::value_type_t<std::string>>);
    static_assert(std::same_as<char, csl::mp::value_type_t<std::array<char, 1>>>);
    static_assert(std::same_as<char, csl::mp::value_type_t<char[1]>>); // NOLINT(*-c-arrays)
    static_assert(std::same_as<bool, csl::mp::value_type_t<std::is_same<int, char>>>);
}
namespace test::primitives::bind_front {
    using same_as_int = csl::mp::bind_front<std::is_same, int>;
    static_assert(same_as_int::value<int>);

    using tuple_starting_with_int = csl::mp::bind_front<csl::mp::tuple, int>;
    static_assert(std::same_as<
        tuple_starting_with_int::type<char>,
        csl::mp::tuple<int, char>
    >);
}
namespace test::primitives::bind_back {
    using same_as_int = csl::mp::bind_back<std::is_same, int>;
    static_assert(same_as_int::value<int>);

    using tuple_ending_with_int = csl::mp::bind_back<csl::mp::tuple, int>;
    static_assert(std::same_as<
        tuple_ending_with_int::type<char>,
        csl::mp::tuple<char, int>
    >);
}
