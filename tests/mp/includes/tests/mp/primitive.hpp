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
namespace test::concepts::std_array {
    static_assert(csl::mp::concepts::std_array<std::array<int, 1>>);
    static_assert(not csl::mp::concepts::std_array<char[1]>); // NOLINT(*-c-arrays)
}

namespace test::primitives::value_type {
}
namespace test::primitives::bind_front {
    // WIP -> +_t
}
namespace test::primitives::bind_back {

}
