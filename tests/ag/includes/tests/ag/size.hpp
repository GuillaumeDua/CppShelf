#pragma once

#include <csl/ag.hpp>

namespace test::ag::field_count::trivial_cases {
    using expect_1 = struct { int i; };
    using expect_2 = struct { int i; char c; };

    static_assert(csl::ag::size_v<expect_1> == 1);
    static_assert(csl::ag::size_v<expect_2> == 2);
}
namespace test::ag::field_count::non_trivial_cases {
    // NOLINTBEGIN
    using expect_1 = struct { int & i; };
    using expect_2 = struct { int & i; char && c; };
    using expect_3 = struct { int & i; char && c; char & cc; };
    // NOLINTEND

    static_assert(csl::ag::size_v<expect_1> == 1);
    static_assert(csl::ag::size_v<expect_2> == 2);
    static_assert(csl::ag::size_v<expect_3> == 3);
}
