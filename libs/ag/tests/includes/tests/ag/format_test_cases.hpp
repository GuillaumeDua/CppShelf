#pragma once

#include <array>
#include <format>
#include <string>
#include <tuple>
#include <vector>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-use-anonymous-namespace)
// NOLINTBEGIN(*cert-err58-cpp)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace {
    namespace implementation {

#if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT) and CSL_AG__ENABLE_BITFIELDS_SUPPORT
        constexpr bool bitfields_enabled = true;
#else
        constexpr bool bitfields_enabled = false;
#endif

#if __has_include(<csl/typeinfo.hpp>)
        constexpr bool typeinfo_linked = true;
#else
        constexpr bool typeinfo_linked = false;
#endif

        [[nodiscard]] inline auto const & name_suffix() {
            static const std::string value = std::format(
                " [FORMATTING={}] [BITFIELDS={}] [TYPEINFO={}]",
                name,
                bitfields_enabled ? "ON" : "OFF",
                typeinfo_linked ? "ON" : "OFF"
            );
            return value;
        }

        [[nodiscard]] inline auto const & tags() {
            static const std::string value = std::format("[ag][{}]", name);
            return value;
        }
    }
} // namespace

namespace tests::concepts::produced {

    static_assert(csl::ag::concepts::produced<implementation::formatter<types::field_1>>);
    static_assert(csl::ag::concepts::produced<implementation::formatter<types::field_2>>);
    static_assert(csl::ag::concepts::produced<implementation::formatter<types::field_3_nested>>);
    static_assert(csl::ag::concepts::produced<implementation::formatter<types::field_3_nested_tuplelike>>);
    static_assert(csl::ag::concepts::produced<implementation::formatter<types::field_4_nested_range>>);
    static_assert(csl::ag::concepts::produced<implementation::formatter<types::field_everything>>);

    // not impacting builtin/third-party formatters
    static_assert(not csl::ag::concepts::produced<implementation::formatter<int>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::string>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::vector<int>>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::vector<types::field_1>>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::tuple<int>>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::array<int, 3>>>);
}

TEMPLATE_TEST_CASE("default" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{}", f::value) == f::default_expected);
}

TEMPLATE_TEST_CASE(":n" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{:n}", f::value) == f::no_braces_expected);
}

TEMPLATE_TEST_CASE(":i" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{:i}", f::value | csl::ag::io::indented) == f::indented_expected);
    CHECK(implementation::format("{:i}", f::value) == f::indented_expected);
}

TEMPLATE_TEST_CASE(":x" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{:x}", f::value | csl::ag::io::indexed) == f::indexed_expected);
    CHECK(implementation::format("{:x}", f::value) == f::indexed_expected);
}

#if __has_include(<csl/typeinfo.hpp>)
TEMPLATE_TEST_CASE(":t" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{:t}", f::value | csl::ag::io::typenamed) == f::typenamed_expected);
    CHECK(implementation::format("{:t}", f::value) == f::typenamed_expected);
}

TEMPLATE_TEST_CASE(":ixt" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;

    CHECK(
        implementation::format("{:ixt}", f::value | csl::ag::io::indented | csl::ag::io::indexed | csl::ag::io::typenamed)
        == f::indented_indexed_typenamed_expected
    );
    CHECK(implementation::format("{:ixt}", f::value) == f::indented_indexed_typenamed_expected);
}
#endif

TEMPLATE_TEST_CASE(":z (unrecognized spec letter) throws" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK_THROWS_WITH(
        implementation::format("{:z}", f::value | csl::ag::io::indented),
        "csl::ag::io: unrecognized format-spec letter (expected one of: n, i, x, t)"
    );
}

TEMPLATE_TEST_CASE("indented" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{}", f::value | csl::ag::io::indented) == f::indented_expected);
}

TEMPLATE_TEST_CASE("indexed" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{}", f::value | csl::ag::io::indexed) == f::indexed_expected);
}

#if __has_include(<csl/typeinfo.hpp>)
TEMPLATE_TEST_CASE("typenamed" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{}", f::value | csl::ag::io::typenamed) == f::typenamed_expected);
}

TEMPLATE_TEST_CASE("indented+indexed+typenamed" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(
        implementation::format("{}", f::value | csl::ag::io::indented | csl::ag::io::indexed | csl::ag::io::typenamed)
        == f::indented_indexed_typenamed_expected
    );
}

TEMPLATE_TEST_CASE("view composition" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;

    {
        constexpr auto view = csl::ag::io::indented | csl::ag::io::indexed | csl::ag::io::typenamed;
        CHECK(implementation::format("{}", f::value | view) == f::indented_indexed_typenamed_expected);
    }

    {
        constexpr auto view = csl::ag::io::indexed | csl::ag::io::typenamed;
        CHECK(
            implementation::format("{}", f::value | view)
            == f::indented_indexed_typenamed_expected
        );
    }
}
#endif

TEMPLATE_TEST_CASE("csl::ag::io::to_string default output" + implementation::name_suffix() + "", implementation::tags(),
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

#if __has_include(<csl/typeinfo.hpp>)
TEMPLATE_TEST_CASE("csl::ag::io::to_string composed view output" + implementation::name_suffix() + "", implementation::tags(),
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

TEMPLATE_TEST_CASE("csl::ag::io::to_string composed NTTP output" + implementation::name_suffix() + "", implementation::tags(),
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
#endif

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*cert-err58-cpp)
// NOLINTEND(*-use-anonymous-namespace)
// NOLINTEND(*-avoid-do-while)
