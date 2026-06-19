#pragma once

// Shared fixture data for csl::ag formatting tests (fmtlib_support.cpp, std_format_support.cpp).
// Must be included inside an anonymous namespace, after `namespace types = test::ag::types;`.

// NOLINTBEGIN(*-avoid-magic-numbers)

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
        R"({std::tuple<int, char, std::string_view>: (int: 2, char: 'b', std::string_view: "str"), std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::pair<int, int>: (int: 42, int: 43)})";
    constexpr static std::string_view indented_indexed_typenamed_expected =
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
        R"({std::string_view: "hello", std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::array<int, 3>: [int: 42, int: 43, int: 44], std::array<std::string_view, 3>: [std::string_view: "a", std::string_view: "b", std::string_view: "c"]})";
    constexpr static std::string_view indented_indexed_typenamed_expected =
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
        R"({bool: true, test::ag::types::field_3_nested: {int: 1, test::ag::types::field_1: {int: 42}, test::ag::types::field_2: {int: 123, char: 'A'}}, test::ag::types::field_3_nested_tuplelike: {std::tuple<int, char, std::string_view>: (int: 2, char: 'b', std::string_view: "str"), std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::pair<int, int>: (int: 42, int: 43)}, test::ag::types::field_4_nested_range: {std::string_view: "hello", std::array<char, 3>: [char: 'a', char: 'b', char: 'c'], std::array<int, 3>: [int: 42, int: 43, int: 44], std::array<std::string_view, 3>: [std::string_view: "a", std::string_view: "b", std::string_view: "c"]}})";
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
};
#pragma endregion

// NOLINTEND(*-avoid-magic-numbers)
