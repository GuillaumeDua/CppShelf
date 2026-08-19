// Cross-TU feature-header contract (fmt-style):
// this TU does NOT include the typeinfo bridge - see typeinfo_bridge_enabled.cpp for the rationale and the contract being verified.
#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>

#include <format>
#include <typeindex>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-avoid-magic-numbers)

#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
# error "CSL_AG_TEST__WITH_TYPEINFO must not be defined for this test"
#endif

namespace test::ag::cross_tu {
    struct plain { char c; };
}

TEST_CASE("cross-TU: typeinfo bridge disabled - typenamed uses the <typeindex> fallback", "[ag][formatting][cross_tu]") {
    using namespace csl::ag::formatting;
    const auto expected = std::format("{{{}: 'A'}}", std::type_index(typeid(char)).name());
    CHECK(std::format("{}", test::ag::cross_tu::plain{ .c = 'A' } | typenamed) == expected);
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
