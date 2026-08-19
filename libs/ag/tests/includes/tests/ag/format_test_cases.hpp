#pragma once

// Backend-agnostic csl::ag formatting test cases: the includer supplies the backend.
//
// Include at file scope, with `namespace types = test::ag::types;` in scope
// and `implementation` already defined in the unnamed namespace, providing:
// - `name`
// - `format(fmt_str, value)`
// - `formatter<T, Char>`

#include <tests/ag/format_fixtures.hpp>

#include <array>
#include <coroutine>
#include <format>
#include <functional>
#include <string>
#include <tuple>
#include <typeindex>
#include <variant>
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

#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
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

    // formatting is a per-type opt-in: a type its owner never opted in must not be claimed.
    // std-owned aggregates matter most here - claiming them would violate [namespace.std]/2.
    static_assert(not csl::ag::concepts::produced<implementation::formatter<types::without_formatter>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::monostate>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::identity>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::plus<>>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::less<>>>);
    static_assert(not csl::ag::concepts::produced<implementation::formatter<std::suspend_always>>);

    // ... whereas the view form needs no opt-in, for any of them
    namespace decorators = csl::ag::formatting::details::decorators;
    static_assert(csl::ag::concepts::produced<implementation::formatter<decorators::formatted_view_t<types::without_formatter>>>);
    static_assert(csl::ag::concepts::produced<implementation::formatter<decorators::formatted_view_t<std::monostate>>>);
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

TEST_CASE("view form requires no opt-in" + implementation::name_suffix() + "", implementation::tags()) {

    constexpr auto value = types::without_formatter{ .i = 42 };

    CHECK(implementation::format("{}", value | csl::ag::formatting::indented) == "{\n    42\n}");
    // format_options::none reproduces the default output, without any opt-in
    CHECK(implementation::format("{}", value | csl::ag::formatting::format_options::none) == "{42}");
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
    CHECK(implementation::format("{:n}", f::value | csl::ag::formatting::no_braces) == f::no_braces_expected);
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::no_braces) == f::no_braces_expected);
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
    CHECK(implementation::format("{:i}", f::value | csl::ag::formatting::indented) == f::indented_expected);
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::indented) == f::indented_expected);
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
    CHECK(implementation::format("{:x}", f::value | csl::ag::formatting::indexed) == f::indexed_expected);
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::indexed) == f::indexed_expected);
    CHECK(implementation::format("{:x}", f::value) == f::indexed_expected);
}

#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
TEMPLATE_TEST_CASE(":t" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{:t}", f::value | csl::ag::formatting::typenamed) == f::typenamed_expected);
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::typenamed) == f::typenamed_expected);
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
        implementation::format("{:ixt}", f::value | csl::ag::formatting::indented | csl::ag::formatting::indexed | csl::ag::formatting::typenamed)
        == f::indented_indexed_typenamed_expected
    );
    CHECK(
        implementation::format("{}", f::value | csl::ag::formatting::indented | csl::ag::formatting::indexed | csl::ag::formatting::typenamed)
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
        implementation::format("{:z}", f::value | csl::ag::formatting::indented),
        "csl::ag::formatting: unrecognized format-spec letter (expected one of: n, i, x, t)"
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
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::indented) == f::indented_expected);
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
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::indexed) == f::indexed_expected);
}

#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
TEMPLATE_TEST_CASE("typenamed" + implementation::name_suffix() + "", implementation::tags(),
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::typenamed) == f::typenamed_expected);
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
        implementation::format("{}", f::value | csl::ag::formatting::indented | csl::ag::formatting::indexed | csl::ag::formatting::typenamed)
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
        constexpr auto view = csl::ag::formatting::indented | csl::ag::formatting::indexed | csl::ag::formatting::typenamed;
        CHECK(implementation::format("{}", f::value | view) == f::indented_indexed_typenamed_expected);
    }

    {
        constexpr auto view = csl::ag::formatting::indexed | csl::ag::formatting::typenamed | csl::ag::formatting::indented;
        CHECK(
            implementation::format("{}", f::value |  view)
            == f::indented_indexed_typenamed_expected
        );
    }

    {
        constexpr auto partial_view = csl::ag::formatting::indexed | csl::ag::formatting::typenamed;
        CHECK(
            implementation::format("{}", f::value | csl::ag::formatting::indented | partial_view)
            == f::indented_indexed_typenamed_expected
        );
    }

    {
        constexpr auto partial_view = csl::ag::formatting::indexed | csl::ag::formatting::typenamed;
        CHECK(
            implementation::format("{}", f::value | partial_view | csl::ag::formatting::indented)
            == f::indented_indexed_typenamed_expected
        );
    }
}
#endif

#if not (defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO)
// Fallback contract: without the typeinfo bridge, type_name is std::type_index(typeid(T)).name()
// (implementation-defined, possibly mangled - but deterministic).
TEMPLATE_TEST_CASE("typenamed: <typeindex> runtime fallback" + implementation::name_suffix() + "", implementation::tags(),
    types::field_2
) {
    using f = fixture<TestType>;
    const auto expected = std::format("{{{}: 123, {}: 'A'}}",
        std::type_index(typeid(int)).name(),
        std::type_index(typeid(char)).name()
    );
    CHECK(implementation::format("{}", f::value | csl::ag::formatting::typenamed) == expected);
    CHECK(implementation::format("{:t}", f::value) == expected);
}
#endif

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*cert-err58-cpp)
// NOLINTEND(*-use-anonymous-namespace)
// NOLINTEND(*-avoid-do-while)
