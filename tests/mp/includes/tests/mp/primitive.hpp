#pragma once

#include <csl/mp.hpp>

// TODO(Guillaume)

namespace test::primitives::type_identity {
    static_assert(std::same_as<int, csl::mp::type_identity_t<int>>);
}
namespace test::primitives::value_type {
    static_assert(csl::mp::concepts::fwd_ref<int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<int&&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&, int>);
    static_assert(csl::mp::concepts::fwd_ref<const int&&, int>);
}
namespace test::primitives::bind_front {

}
namespace test::primitives::bind_back {

}
