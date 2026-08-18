// ODR sanity (fmt-style feature-header contract):
// this TU includes the typeinfo bridge, the sibling TU (odr_mixed_plain.cpp) does not - each formats *different* types.
// The resulting binary must link and behave deterministically.
#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>
#include <csl/ag/formatting/typeinfo.hpp>

#include <format>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace test::ag::odr {
    struct bridged { int i; };
}

TEST_CASE("odr-mixed: bridged TU - typenamed uses csl::typeinfo", "[ag][formatting][odr]") {
    using namespace csl::ag::formatting;

    // NOTE: int demangling seems stable across compilers
    CHECK(std::format("{}", test::ag::odr::bridged{ .i = 42 } | typenamed) == "{int: 42}");
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
