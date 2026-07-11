// ODR sanity (fmt-style feature-header contract):
// this TU includes the typeinfo bridge, the sibling TU (odr_mixed_plain.cpp) does not - each formats *different* types.
// The resulting binary must link and behave deterministically.
#include <csl/ag.hpp>
#include <csl/ag/formatting/format.hpp>
#include <csl/ag/formatting/typeinfo.hpp>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace test::ag::odr {
    struct bridged { int i; };
}

TEST_CASE("odr-mixed: bridged TU - typenamed uses csl::typeinfo", "[ag][formatting][odr]") {
    using namespace csl::ag::io;

    // NOTE: int demangling seems stable across compilers
    CHECK(to_string<typenamed>(test::ag::odr::bridged{ .i = 42 }) == "{int: 42}");
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
