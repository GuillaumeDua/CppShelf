// ODR sanity (fmt-style feature-header contract):
// this TU does NOT include the typeinfo bridge - see odr_mixed_bridge.cpp for the rationale and the contract being verified.
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

namespace test::ag::odr {
    struct plain { char c; };
}

TEST_CASE("odr-mixed: plain TU - typenamed uses the <typeindex> fallback", "[ag][formatting][odr]") {
    using namespace csl::ag::formatting;
    const auto expected = std::format("{{{}: 'A'}}", std::type_index(typeid(char)).name());
    CHECK(std::format("{}", test::ag::odr::plain{ .c = 'A' } | typenamed) == expected);
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
