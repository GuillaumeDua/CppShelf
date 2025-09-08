#pragma once

#include <csl/mp.hpp>

// TODO(Guillaume)

namespace test::primitives::type_identity {
    static_assert(std::same_as<int, csl::mp::type_identity_t<int>>);
}
namespace test::concepts::fwd_ref {
    static_assert(csl::mp::concepts::fwd_ref<int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<int&&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&&, int>);

    static_assert(not csl::mp::concepts::fwd_ref<int&, int&>);
}
namespace test::concepts::unwrap_reference {
    static_assert(std::same_as<int, csl::mp::unwrap_reference_t<int>>);
    static_assert(std::same_as<int&, csl::mp::unwrap_reference_t<std::reference_wrapper<int>>>);
    static_assert(not std::same_as<int, csl::mp::unwrap_reference_t<int&>>);

    static_assert(std::same_as<int, csl::mp::unwrap_ref_decay_t<int>>);
    static_assert(std::same_as<int&, csl::mp::unwrap_ref_decay_t<std::reference_wrapper<int>>>);
    static_assert(std::same_as<int, csl::mp::unwrap_ref_decay_t<int&>>);
}

namespace test::primitives::value_type {
}
namespace test::primitives::bind_front {
    
}
namespace test::primitives::bind_back {

}
