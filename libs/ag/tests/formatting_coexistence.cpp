#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

// Every formatting feature header in one TU: backends must coexist
#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>
#include <csl/ag/formatting/backend/fmt.hpp>
#include <csl/ag/formatting/backend/ostream.hpp>
#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
#   include <csl/ag/formatting/typeinfo.hpp>
#endif
#include <tests/types.hpp>

#include <sstream>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace types = test::ag::types;

TEST_CASE("all formatting backends coexist in one TU", "[ag][formatting][coexistence]") {

    constexpr auto value = types::field_2{ .i = 123, .c = 'A' };
    constexpr std::string_view expected = "{123, 'A'}";

    CHECK(std::format("{}", value) == expected);
    CHECK(fmt::format("{}", value) == expected);
    {
        std::ostringstream ss;
        using namespace csl::ag::io;
        ss << value;
        CHECK(ss.str() == expected);
    }
}

TEST_CASE("all formatting backends coexist in one TU - composed views", "[ag][formatting][coexistence]") {

    using namespace csl::ag::io;
    constexpr auto value = types::field_2{ .i = 123, .c = 'A' };
    constexpr std::string_view expected = "{[0] int: 123, [1] char: 'A'}";

    CHECK(std::format("{}", value | indexed | typenamed) == expected);
    CHECK(fmt::format("{}", value | indexed | typenamed) == expected);
    {
        std::ostringstream ss;
        ss << (value | indexed | typenamed);
        CHECK(ss.str() == expected);
    }
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*-avoid-do-while)
