#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT true
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "ON"
#else
#  define CSL_AG_BITFIELDS_STR "OFF"
#endif

#include <csl/ag.hpp>
#include <tests/types.hpp>

#include <format>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-use-anonymous-namespace)
// NOLINTBEGIN(*cert-err58-cpp)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace types = test::ag::types;

namespace tests::concepts::produced {

    static_assert(csl::ag::concepts::produced<std::formatter<types::field_1>>);
    static_assert(csl::ag::concepts::produced<std::formatter<types::field_2>>);
    static_assert(csl::ag::concepts::produced<std::formatter<types::field_3_nested>>);
    static_assert(csl::ag::concepts::produced<std::formatter<types::field_3_nested_tuplelike>>);
    static_assert(csl::ag::concepts::produced<std::formatter<types::field_4_nested_range>>);
    static_assert(csl::ag::concepts::produced<std::formatter<types::field_everything>>);

    // not impacting <format> built-in formatters
    static_assert(not csl::ag::concepts::produced<std::formatter<int>>);
    static_assert(not csl::ag::concepts::produced<std::formatter<std::string>>);
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
        constexpr static std::string_view default_expected = "{42}";
        constexpr static std::string_view no_braces_expected = "42";
        constexpr static std::string_view indented_expected =
R"({
    42
})";
        constexpr static std::string_view indexed_expected = "{[0] 42}";
        constexpr static std::string_view typenamed_expected = "{int: 42}";
        constexpr static std::string_view indented_indexed_typenamed_expected =
R"({
    [0] int: 42
})";
    };

    template <>
    struct fixture<types::field_2> {
        constexpr static types::field_2 value{ .i = 123, .c = 'A' };
        constexpr static std::string_view default_expected = "{123, 'A'}";
        constexpr static std::string_view no_braces_expected = "123'A'";
        constexpr static std::string_view indented_expected =
R"({
    123,
    'A'
})";
        constexpr static std::string_view indexed_expected = "{[0] 123, [1] 'A'}";
        constexpr static std::string_view typenamed_expected = "{int: 123, char: 'A'}";
        constexpr static std::string_view indented_indexed_typenamed_expected =
R"({
    [0] int: 123,
    [1] char: 'A'
})";
    };

    template <>
    struct fixture<types::field_3_nested> {
        constexpr static types::field_3_nested value{
            .i  = 1,
            .f1 = fixture<types::field_1>::value,
            .f2 = fixture<types::field_2>::value
        };
        constexpr static std::string_view default_expected = "{1, {42}, {123, 'A'}}";
        constexpr static std::string_view no_braces_expected = "1{42}{123, 'A'}";
        constexpr static std::string_view indented_expected =
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
        constexpr static std::string_view indexed_expected =
            "{[0] 1, [1] {[0] 42}, [2] {[0] 123, [1] 'A'}}";
        constexpr static std::string_view typenamed_expected =
            "{int: 1, test::ag::types::field_1: {int: 42}, test::ag::types::field_2: {int: 123, char: 'A'}}";
        constexpr static std::string_view indented_indexed_typenamed_expected =
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
            .a  = {'a', 'b', 'c'},
            .p  = { 42, 43 }, // NOLINT
        };
        constexpr static std::string_view default_expected = R"({(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)})";
        constexpr static std::string_view no_braces_expected = R"((2, 'b', "str")['a', 'b', 'c'](42, 43))";
        constexpr static std::string_view indented_expected =
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
        constexpr static std::string_view indexed_expected =
            R"({[0] ([0] 2, [1] 'b', [2] "str"), [1] [[0] 'a', [1] 'b', [2] 'c'], [2] ([0] 42, [1] 43)})";
        constexpr static std::string_view typenamed_expected =
            R"({std::tuple<int, char, std::basic_string_view<char, std::char_traits<char> > >: (int: 2, char: 'b', std::basic_string_view<char>: "str"), std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::pair<int, int>: (int: 42, int: 43)})";
        constexpr static std::string_view indented_indexed_typenamed_expected =
R"({
    [0] std::tuple<int, char, std::basic_string_view<char, std::char_traits<char> > >: (
        [0] int: 2,
        [1] char: 'b',
        [2] std::basic_string_view<char>: "str"
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
            .sv   = "hello",
            .a_c  = { 'a', 'b', 'c' },
            .a_i  = { 42, 43, 44 },
            .a_sv = { "a", "b", "c" },
        };
        constexpr static std::string_view default_expected = R"({"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]})";
        constexpr static std::string_view no_braces_expected = R"("hello"['a', 'b', 'c'][42, 43, 44]["a", "b", "c"])";
        constexpr static std::string_view indented_expected =
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
        constexpr static std::string_view indexed_expected =
            R"({[0] "hello", [1] [[0] 'a', [1] 'b', [2] 'c'], [2] [[0] 42, [1] 43, [2] 44], [3] [[0] "a", [1] "b", [2] "c"]})";
        constexpr static std::string_view typenamed_expected =
            R"({std::basic_string_view<char>: "hello", std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::array<int, 3>: [int: 42, int: 43, int: 44], std::array<std::basic_string_view<char>, 3>: [std::basic_string_view<char>: "a", std::basic_string_view<char>: "b", std::basic_string_view<char>: "c"]})";
        constexpr static std::string_view indented_indexed_typenamed_expected =
R"({
    [0] std::basic_string_view<char>: "hello",
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
    [3] std::array<std::basic_string_view<char>, 3>: [
        [0] std::basic_string_view<char>: "a",
        [1] std::basic_string_view<char>: "b",
        [2] std::basic_string_view<char>: "c"
    ]
})";
    };

    template <>
    struct fixture<types::field_everything> {
        constexpr static types::field_everything value{
            .b  = true,
            .f1 = fixture<types::field_3_nested>::value,
            .f2 = fixture<types::field_3_nested_tuplelike>::value,
            .f3 = fixture<types::field_4_nested_range>::value,
        };
        constexpr static std::string_view default_expected = R"({true, {1, {42}, {123, 'A'}}, {(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)}, {"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]}})";
        constexpr static std::string_view no_braces_expected = R"(true{1, {42}, {123, 'A'}}{(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)}{"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]})";
        constexpr static std::string_view indented_expected =
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
        constexpr static std::string_view indexed_expected =
            R"({[0] true, [1] {[0] 1, [1] {[0] 42}, [2] {[0] 123, [1] 'A'}}, [2] {[0] ([0] 2, [1] 'b', [2] "str"), [1] [[0] 'a', [1] 'b', [2] 'c'], [2] ([0] 42, [1] 43)}, [3] {[0] "hello", [1] [[0] 'a', [1] 'b', [2] 'c'], [2] [[0] 42, [1] 43, [2] 44], [3] [[0] "a", [1] "b", [2] "c"]}})";
        constexpr static std::string_view typenamed_expected =
            R"({bool: true, test::ag::types::field_3_nested: {int: 1, test::ag::types::field_1: {int: 42}, test::ag::types::field_2: {int: 123, char: 'A'}}, test::ag::types::field_3_nested_tuplelike: {std::tuple<int, char, std::basic_string_view<char, std::char_traits<char> > >: (int: 2, char: 'b', std::basic_string_view<char>: "str"), std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::pair<int, int>: (int: 42, int: 43)}, test::ag::types::field_4_nested_range: {std::basic_string_view<char>: "hello", std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::array<int, 3>: [int: 42, int: 43, int: 44], std::array<std::basic_string_view<char>, 3>: [std::basic_string_view<char>: "a", std::basic_string_view<char>: "b", std::basic_string_view<char>: "c"]}})";
        constexpr static std::string_view indented_indexed_typenamed_expected =
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
        [0] std::tuple<int, char, std::basic_string_view<char, std::char_traits<char> > >: (
            [0] int: 2,
            [1] char: 'b',
            [2] std::basic_string_view<char>: "str"
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
        [0] std::basic_string_view<char>: "hello",
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
        [3] std::array<std::basic_string_view<char>, 3>: [
            [0] std::basic_string_view<char>: "a",
            [1] std::basic_string_view<char>: "b",
            [2] std::basic_string_view<char>: "c"
        ]
    }
})";
    };
#pragma endregion
} // namespace

TEMPLATE_TEST_CASE("csl::ag::std::format default [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][format]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(std::format("{}", f::value) == f::default_expected);
}

TEMPLATE_TEST_CASE("csl::ag::std::format :n [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][format]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(std::format("{:n}", f::value) == f::no_braces_expected);
}

TEMPLATE_TEST_CASE("csl::ag::std::format indented [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][format]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(std::format("{}", f::value | csl::ag::io::indented) == f::indented_expected);
}

TEMPLATE_TEST_CASE("csl::ag::std::format indexed [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][format]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(std::format("{}", f::value | csl::ag::io::indexed) == f::indexed_expected);
}

TEMPLATE_TEST_CASE("csl::ag::std::format typenamed [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][format]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(std::format("{}", f::value | csl::ag::io::typenamed) == f::typenamed_expected);
}

TEMPLATE_TEST_CASE("csl::ag::std::format indented+indexed+typenamed [BITFIELDS=" CSL_AG_BITFIELDS_STR "]", "[ag][format]",
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range,
    types::field_everything
) {
    using f = fixture<TestType>;
    CHECK(
        std::format("{}", f::value | csl::ag::io::indented | csl::ag::io::indexed | csl::ag::io::typenamed)
        == f::indented_indexed_typenamed_expected
    );
}

#undef CSL_AG_BITFIELDS_STR

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*cert-err58-cpp)
// NOLINTEND(*-use-anonymous-namespace)
// NOLINTEND(*-avoid-do-while)
