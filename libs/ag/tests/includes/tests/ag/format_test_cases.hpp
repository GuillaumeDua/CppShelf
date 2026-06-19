#pragma once

// Shared csl::ag::concepts::produced static_asserts and TEMPLATE_TEST_CASE bodies
// for csl::ag formatting tests (fmtlib_support.cpp, std_format_support.cpp).
//
// Must be included after `namespace types = test::ag::types;` and, inside an anonymous
// namespace, the `fixture<T>` specializations from <tests/ag/format_fixtures.hpp> and a
// `namespace under_test` defining:
//   formatter - alias to fmt::formatter / std::formatter under test
//   tag       - catch2 tag, e.g. "fmt" / "format"
//   format    - proxy calling fmt::format / std::format under test
// and CSL_AG_BITFIELDS_STR, CSL_AG_FORMATTING_STR (the enabled CSL_AG__ENABLE_*_SUPPORT macro name).

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-use-anonymous-namespace)
// NOLINTBEGIN(*cert-err58-cpp)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace tests::concepts::produced {

    static_assert(csl::ag::concepts::produced<under_test::formatter<types::field_1>>);
    static_assert(csl::ag::concepts::produced<under_test::formatter<types::field_2>>);
    static_assert(csl::ag::concepts::produced<under_test::formatter<types::field_3_nested>>);
    static_assert(csl::ag::concepts::produced<under_test::formatter<types::field_3_nested_tuplelike>>);
    static_assert(csl::ag::concepts::produced<under_test::formatter<types::field_4_nested_range>>);
    static_assert(csl::ag::concepts::produced<under_test::formatter<types::field_everything>>);

    // not impacting builtin/third-party formatters
    static_assert(not csl::ag::concepts::produced<under_test::formatter<int>>);
    static_assert(not csl::ag::concepts::produced<under_test::formatter<std::string>>);
    static_assert(not csl::ag::concepts::produced<under_test::formatter<std::vector<int>>>);
    static_assert(not csl::ag::concepts::produced<under_test::formatter<std::vector<types::field_1>>>);
    static_assert(not csl::ag::concepts::produced<under_test::formatter<std::tuple<int>>>);
    static_assert(not csl::ag::concepts::produced<under_test::formatter<std::array<int, 3>>>);
}

TEMPLATE_TEST_CASE("default [FORMATTING=" CSL_AG_FORMATTING_STR "] [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(under_test::format("{}", f::value) == f::default_expected);
}

TEMPLATE_TEST_CASE(":n [FORMATTING=" CSL_AG_FORMATTING_STR "] [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(under_test::format("{:n}", f::value) == f::no_braces_expected);
}

TEMPLATE_TEST_CASE("indented [FORMATTING=" CSL_AG_FORMATTING_STR "] [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(under_test::format("{}", f::value | csl::ag::io::indented) == f::indented_expected);
}

TEMPLATE_TEST_CASE("indexed [FORMATTING=" CSL_AG_FORMATTING_STR "] [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(under_test::format("{}", f::value | csl::ag::io::indexed) == f::indexed_expected);
}

TEMPLATE_TEST_CASE("typenamed [FORMATTING=" CSL_AG_FORMATTING_STR "] [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(under_test::format("{}", f::value | csl::ag::io::typenamed) == f::typenamed_expected);
}

TEMPLATE_TEST_CASE("indented+indexed+typenamed [FORMATTING=" CSL_AG_FORMATTING_STR "] [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(
        under_test::format("{}", f::value | csl::ag::io::indented | csl::ag::io::indexed | csl::ag::io::typenamed)
        == f::indented_indexed_typenamed_expected
    );
}

TEMPLATE_TEST_CASE("csl::ag::io::to_string default output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(csl::ag::io::to_string(f::value) == f::default_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io::to_string composed view output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(
        to_string(f::value | indented | indexed | typenamed)
        == f::indented_indexed_typenamed_expected
    );
}

TEMPLATE_TEST_CASE("csl::ag::io::to_string composed NTTP output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][" + std::string(under_test::tag) + "]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(
        to_string<indented | indexed | typenamed>(f::value)
        == f::indented_indexed_typenamed_expected
    );
}

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*cert-err58-cpp)
// NOLINTEND(*-use-anonymous-namespace)
// NOLINTEND(*-avoid-do-while)
