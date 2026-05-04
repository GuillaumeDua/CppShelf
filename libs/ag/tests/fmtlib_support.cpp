// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# define CSL_AG__ENABLE_FMTLIB_SUPPORT true // force fmtlib support. Prefer using CMake cache instead CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_FMTLIB_SUPPORT=ON"
#endif

#include <tests/types.hpp>

// TODO: test if bitfield support is ON
// WIP: check possible clash with user-defined formatters -> complete, partial/generics, etc.

namespace types = test::ag::types;

namespace tests::concepts::produced {

    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_1>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_2>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_3_nested>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_3_nested_tuplelike>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_4_nested_range>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_everything>>);

    // not impacting <fmt/format.h>
    static_assert(not csl::ag::concepts::produced<fmt::formatter<int>>);
    // not impacting <fmt/ranges.h>
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::string>>);
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::vector<int>>>);
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::vector<types::field_1>>>);
    // not impacting <fmt/ranges.h> - tuplelikes
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::tuple<int>>>);
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::array<int, 3>>>);
}

namespace tests::concepts::fmt_formattable {

    namespace concepts = csl::ag::io::details::concepts;

    static_assert(concepts::fmt_formattable<types::field_1, char>);
    static_assert(concepts::fmt_formattable<types::field_2, char>);
    static_assert(concepts::fmt_formattable<types::field_3_nested, char>);
    static_assert(concepts::fmt_formattable<types::field_3_nested_tuplelike, char>);
    static_assert(concepts::fmt_formattable<types::field_4_nested_range, char>);
    static_assert(concepts::fmt_formattable<types::field_everything, char>);

    static_assert(concepts::fmt_formattable<int, char>);
    static_assert(concepts::fmt_formattable<std::string, char>);
    static_assert(concepts::fmt_formattable<std::vector<int>, char>);
    static_assert(concepts::fmt_formattable<std::vector<types::field_1>, char>);
    static_assert(concepts::fmt_formattable<std::tuple<int>, char>);
    static_assert(concepts::fmt_formattable<std::array<int, 3>, char>);
}

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {
    template <typename T>
    struct fixture;

#pragma region fixture specializations
    template <>
    struct fixture<types::field_1> {
        constexpr static types::field_1 value{ .i = 42 };
        constexpr static std::string_view default_formatter_expected_result = "{42}";
        constexpr static std::string_view default_formatter_n_expected_result = "42";
        constexpr static std::string_view indented_formatter_expected_result =
R"({
    42
})";
    };

    template <>
    struct fixture<types::field_2> {
        constexpr static types::field_2 value{ .i = 123, .c = 'A' };
        constexpr static std::string_view default_formatter_expected_result = "{123, 'A'}";
        constexpr static std::string_view default_formatter_n_expected_result = "123'A'";
        constexpr static std::string_view indented_formatter_expected_result =
R"({
    123,
    'A'
})";
    };

    template <>
    struct fixture<types::field_3_nested> {
        constexpr static types::field_3_nested value{
            .i = 1,
            .f1 = fixture<types::field_1>::value,
            .f2 = fixture<types::field_2>::value
        };
        constexpr static std::string_view default_formatter_expected_result = "{1, {42}, {123, 'A'}}";
        constexpr static std::string_view default_formatter_n_expected_result = "1{42}{123, 'A'}";
        constexpr static std::string_view indented_formatter_expected_result =
R"({
    1,
    {
        42
    },
    {
        123,
        'A'
    }
})";
    };

    template <>
    struct fixture<types::field_3_nested_tuplelike> {
        constexpr static types::field_3_nested_tuplelike value{
            .tu = { 2, 'b', "str"},
            .a = {'a', 'b', 'c'},
            .p = { 42, 43 }, // NOLINT
        };
        constexpr static std::string_view default_formatter_expected_result = R"({(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)})";
        constexpr static std::string_view default_formatter_n_expected_result = R"((2, 'b', "str")['a', 'b', 'c'](42, 43))";
        constexpr static std::string_view indented_formatter_expected_result =
R"({
    (
        2,
        'b',
        "str"
    ),
    [
        'a',
        'b',
        'c'
    ],
    (
        42,
        43
    )
})";
    };

    template <>
    struct fixture<types::field_4_nested_range> {
        constexpr static types::field_4_nested_range value{
            .sv = "hello",
            .a_c = { 'a', 'b', 'c' },
            .a_i = { 42, 43, 44 },
            .a_sv = { "a", "b", "c" },
        };
        constexpr static std::string_view default_formatter_expected_result = R"({"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]})";
        constexpr static std::string_view default_formatter_n_expected_result = R"("hello"['a', 'b', 'c'][42, 43, 44]["a", "b", "c"])";
        constexpr static std::string_view indented_formatter_expected_result =
R"({
    "hello",
    [
        'a',
        'b',
        'c'
    ],
    [
        42,
        43,
        44
    ],
    [
        "a",
        "b",
        "c"
    ]
})";
    };

    template <>
    struct fixture<types::field_everything> {
        constexpr static types::field_everything value{
            .b = true,
            .f1 = fixture<types::field_3_nested>::value,
            .f2 = fixture<types::field_3_nested_tuplelike>::value,
            .f3 = fixture<types::field_4_nested_range>::value,
        };
        constexpr static std::string_view default_formatter_expected_result = R"({true, {1, {42}, {123, 'A'}}, {(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)}, {"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]}})";
        constexpr static std::string_view default_formatter_n_expected_result = R"(true{1, {42}, {123, 'A'}}{(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)}{"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]})";
        constexpr static std::string_view indented_formatter_expected_result =
R"({
    true,
    {
        1,
        {
            42
        },
        {
            123,
            'A'
        }
    },
    {
        (
            2,
            'b',
            "str"
        ),
        [
            'a',
            'b',
            'c'
        ],
        (
            42,
            43
        )
    },
    {
        "hello",
        [
            'a',
            'b',
            'c'
        ],
        [
            42,
            43,
            44
        ],
        [
            "a",
            "b",
            "c"
        ]
    }
})";
    };
#pragma endregion
} // namespace

TEMPLATE_TEST_CASE("csl::ag::fmt default formatter", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(fmt::format("{}", f::value) == f::default_formatter_expected_result);
}

TEMPLATE_TEST_CASE("csl::ag::fmt default formatter :n", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(fmt::format("{:n}", f::value) == f::default_formatter_n_expected_result);
}

TEMPLATE_TEST_CASE("csl::ag::fmt indented formatter", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(fmt::format("{}", f::value | csl::ag::io::indented) == f::indented_formatter_expected_result);
    CHECK(
        fmt::format("{}", csl::ag::io::details::decorators::depthen_view_t{ f::value })
        == f::indented_formatter_expected_result
    );
}
