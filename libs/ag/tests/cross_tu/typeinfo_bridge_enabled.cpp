// Cross-TU feature-header contract (fmt-style):
// this TU includes the typeinfo bridge, the sibling TU (typeinfo_bridge_disabled.cpp) does not - each formats *different* types.
// The resulting binary must link and behave deterministically.
#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>
#include <csl/ag/formatting/typeinfo.hpp>

#include <format>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace test::ag::cross_tu {
    struct bridged { int i; };
}

TEST_CASE("cross-TU: typeinfo bridge enabled - typenamed uses csl::typeinfo", "[ag][formatting][cross_tu]") {
    using namespace csl::ag::formatting;

    // NOTE: int demangling seems stable across compilers
    CHECK(std::format("{}", test::ag::cross_tu::bridged{ .i = 42 } | typenamed) == "{int: 42}");
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
