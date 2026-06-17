#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#define CSL_AG__ENABLE_FMTLIB_SUPPORT true
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "ON"
#else
#  define CSL_AG_BITFIELDS_STR "OFF"
#endif

#include <csl/ag.hpp>
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>

// TODO: check possible clash with user-defined formatters -> complete, partial/generics, etc.

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
        constexpr static std::string_view indexed_expected_result = "{[0] 42}";
        constexpr static std::string_view typenamed_expected_result = "{int: 42}";
        constexpr static std::string_view indented_indexed_typenamed_expected_result =
R"({
    [0] int: 42
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
        constexpr static std::string_view indexed_expected_result = "{[0] 123, [1] 'A'}";
        constexpr static std::string_view typenamed_expected_result = "{int: 123, char: 'A'}";
        constexpr static std::string_view indented_indexed_typenamed_expected_result =
R"({
    [0] int: 123,
    [1] char: 'A'
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
        constexpr static std::string_view indexed_expected_result =
            "{[0] 1, [1] {[0] 42}, [2] {[0] 123, [1] 'A'}}";
        constexpr static std::string_view typenamed_expected_result =
            "{int: 1, test::ag::types::field_1: {int: 42}, test::ag::types::field_2: {int: 123, char: 'A'}}";
        constexpr static std::string_view indented_indexed_typenamed_expected_result =
R"({
    [0] int: 1,
    [1] test::ag::types::field_1: {
        [0] int: 42
    },
    [2] test::ag::types::field_2: {
        [0] int: 123,
        [1] char: 'A'
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
        constexpr static std::string_view indexed_expected_result =
            R"({[0] ([0] 2, [1] 'b', [2] "str"), [1] [[0] 'a', [1] 'b', [2] 'c'], [2] ([0] 42, [1] 43)})";
        constexpr static std::string_view typenamed_expected_result =
            R"({std::tuple<int, char, std::string_view>: (int: 2, char: 'b', std::string_view: "str"), std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::pair<int, int>: (int: 42, int: 43)})";
        constexpr static std::string_view indented_indexed_typenamed_expected_result =
R"({
    [0] std::tuple<int, char, std::string_view>: (
        [0] int: 2,
        [1] char: 'b',
        [2] std::string_view: "str"
    ),
    [1] std::array<char, 3>: [
        [0] char: 'a',
        [1] char: 'b',
        [2] char: 'c'
    ],
    [2] std::pair<int, int>: (
        [0] int: 42,
        [1] int: 43
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
        constexpr static std::string_view indexed_expected_result =
            R"({[0] "hello", [1] [[0] 'a', [1] 'b', [2] 'c'], [2] [[0] 42, [1] 43, [2] 44], [3] [[0] "a", [1] "b", [2] "c"]})";
        constexpr static std::string_view typenamed_expected_result =
            R"({std::string_view: "hello", std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::array<int, 3>: [int: 42, int: 43, int: 44], std::array<std::string_view, 3>: [std::string_view: "a", std::string_view: "b", std::string_view: "c"]})";
        constexpr static std::string_view indented_indexed_typenamed_expected_result =
R"({
    [0] std::string_view: "hello",
    [1] std::array<char, 3>: [
        [0] char: 'a',
        [1] char: 'b',
        [2] char: 'c'
    ],
    [2] std::array<int, 3>: [
        [0] int: 42,
        [1] int: 43,
        [2] int: 44
    ],
    [3] std::array<std::string_view, 3>: [
        [0] std::string_view: "a",
        [1] std::string_view: "b",
        [2] std::string_view: "c"
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
        constexpr static std::string_view indexed_expected_result =
            R"({[0] true, [1] {[0] 1, [1] {[0] 42}, [2] {[0] 123, [1] 'A'}}, [2] {[0] ([0] 2, [1] 'b', [2] "str"), [1] [[0] 'a', [1] 'b', [2] 'c'], [2] ([0] 42, [1] 43)}, [3] {[0] "hello", [1] [[0] 'a', [1] 'b', [2] 'c'], [2] [[0] 42, [1] 43, [2] 44], [3] [[0] "a", [1] "b", [2] "c"]}})";
        constexpr static std::string_view typenamed_expected_result =
            R"({bool: true, test::ag::types::field_3_nested: {int: 1, test::ag::types::field_1: {int: 42}, test::ag::types::field_2: {int: 123, char: 'A'}}, test::ag::types::field_3_nested_tuplelike: {std::tuple<int, char, std::string_view>: (int: 2, char: 'b', std::string_view: "str"), std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::pair<int, int>: (int: 42, int: 43)}, test::ag::types::field_4_nested_range: {std::string_view: "hello", std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::array<int, 3>: [int: 42, int: 43, int: 44], std::array<std::string_view, 3>: [std::string_view: "a", std::string_view: "b", std::string_view: "c"]}})";
        constexpr static std::string_view indented_indexed_typenamed_expected_result =
R"({
    [0] bool: true,
    [1] test::ag::types::field_3_nested: {
        [0] int: 1,
        [1] test::ag::types::field_1: {
            [0] int: 42
        },
        [2] test::ag::types::field_2: {
            [0] int: 123,
            [1] char: 'A'
        }
    },
    [2] test::ag::types::field_3_nested_tuplelike: {
        [0] std::tuple<int, char, std::string_view>: (
            [0] int: 2,
            [1] char: 'b',
            [2] std::string_view: "str"
        ),
        [1] std::array<char, 3>: [
            [0] char: 'a',
            [1] char: 'b',
            [2] char: 'c'
        ],
        [2] std::pair<int, int>: (
            [0] int: 42,
            [1] int: 43
        )
    },
    [3] test::ag::types::field_4_nested_range: {
        [0] std::string_view: "hello",
        [1] std::array<char, 3>: [
            [0] char: 'a',
            [1] char: 'b',
            [2] char: 'c'
        ],
        [2] std::array<int, 3>: [
            [0] int: 42,
            [1] int: 43,
            [2] int: 44
        ],
        [3] std::array<std::string_view, 3>: [
            [0] std::string_view: "a",
            [1] std::string_view: "b",
            [2] std::string_view: "c"
        ]
    }
})";
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

TEMPLATE_TEST_CASE("csl::ag::fmt default formatter [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
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

TEMPLATE_TEST_CASE("csl::ag::fmt default formatter :n [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
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

TEMPLATE_TEST_CASE("csl::ag::fmt indented formatter [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(fmt::format("{}", f::value | csl::ag::io::indented) == f::indented_formatter_expected_result);
}

TEMPLATE_TEST_CASE("csl::ag::fmt indexed formatter [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(fmt::format("{}", f::value | csl::ag::io::indexed) == f::indexed_expected_result);
}

TEMPLATE_TEST_CASE("csl::ag::fmt typenamed formatter [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(fmt::format("{}", f::value | csl::ag::io::typenamed) == f::typenamed_expected_result);
}

TEMPLATE_TEST_CASE("csl::ag::fmt indented+indexed+typenamed formatter [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(
        fmt::format("{}", f::value | csl::ag::io::indented | csl::ag::io::indexed | csl::ag::io::typenamed)
        == f::indented_indexed_typenamed_expected_result
    );
}

TEMPLATE_TEST_CASE("csl::ag::io::to_string default output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(csl::ag::io::to_string(f::value) == f::default_formatter_expected_result);
}

TEMPLATE_TEST_CASE("csl::ag::io::to_string composed view output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][fmt]",
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
        == f::indented_indexed_typenamed_expected_result
    );
}

#undef CSL_AG_BITFIELDS_STR
