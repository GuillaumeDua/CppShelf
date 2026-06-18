#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1
#define CSL_AG__ENABLE_FMTLIB_SUPPORT 0
#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT 0

#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "ON"
#else
#  define CSL_AG_BITFIELDS_STR "OFF"
#endif

#include <csl/ag.hpp>
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>

#include <sstream>
#include <string>
#include <string_view>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(*-avoid-do-while)
// NOLINTBEGIN(*-use-anonymous-namespace)
// NOLINTBEGIN(*cert-err58-cpp)
// NOLINTBEGIN(*-avoid-magic-numbers)

namespace types = test::ag::types;

struct printable_t { int value; };
// user-defined overload, at file scope (not anonymous namespace) so ADL finds it
static auto & operator<<(std::ostream & os, printable_t v) { return os << "printable:" << v.value; }

struct with_printable_field {
    printable_t p;
    int         x;
};

namespace tests::compile_time {

    using namespace csl::ag::io::details;

    static_assert(ostream_formattable<int>);
    static_assert(ostream_formattable<char>);
    static_assert(ostream_formattable<float>);
    static_assert(ostream_formattable<bool>);
    static_assert(ostream_formattable<std::string>);
    static_assert(ostream_formattable<std::string_view>);

    static_assert(not ostream_formattable<types::field_1>);
    static_assert(not ostream_formattable<types::field_2>);
    static_assert(not ostream_formattable<types::field_3_nested>);
    static_assert(not ostream_formattable<types::empty>);

    static_assert(ostream_formattable<printable_t>);
}

namespace {

    template <typename T>
    struct fixture;

    template <>
    struct fixture<types::field_1> {
        constexpr static types::field_1 value{ .i = 42 };
        constexpr static std::string_view default_expected     = "{42}";
        constexpr static std::string_view no_braces_expected   = "42";
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
        constexpr static std::string_view default_expected     = "{123, 'A'}";
        constexpr static std::string_view no_braces_expected   = "123'A'";
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
        constexpr static std::string_view default_expected     = "{1, {42}, {123, 'A'}}";
        constexpr static std::string_view no_braces_expected   = "1{42}{123, 'A'}";
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
        constexpr static std::string_view default_expected     = R"({(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)})";
        constexpr static std::string_view no_braces_expected   = R"((2, 'b', "str")['a', 'b', 'c'](42, 43))";
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
        constexpr static std::string_view default_expected     = R"({"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]})";
        constexpr static std::string_view no_braces_expected   = R"("hello"['a', 'b', 'c'][42, 43, 44]["a", "b", "c"])";
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
        constexpr static std::string_view default_expected =
            R"({true, {1, {42}, {123, 'A'}}, {(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)}, {"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]}})";
        constexpr static std::string_view no_braces_expected =
            R"(true{1, {42}, {123, 'A'}}{(2, 'b', "str"), ['a', 'b', 'c'], (42, 43)}{"hello", ['a', 'b', 'c'], [42, 43, 44], ["a", "b", "c"]})";
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

}

TEST_CASE("csl::ag::io operator<< returns std::ostream & [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    std::ostringstream ss;
    using namespace csl::ag::io;
    std::ostream & result = (ss << types::field_1{42});
    CHECK(&result == static_cast<std::ostream *>(&ss));
}

TEST_CASE("csl::ag::io operator<< supports chaining [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    std::ostringstream ss;
    using namespace csl::ag::io;
    constexpr auto v = fixture<types::field_1>::value;
    ss << v << '|' << v;
    auto out = ss.str();
    auto sep = out.find('|');
    REQUIRE(sep != std::string::npos);
    CHECK(out.substr(0, sep)  == fixture<types::field_1>::default_expected);
    CHECK(out.substr(sep + 1) == fixture<types::field_1>::default_expected);
}

TEST_CASE("csl::ag::io no_braces manipulator is one-shot [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    // After printing with non-default (here, no_braces), the next print should revert to default (braced).
    std::ostringstream ss;
    using namespace csl::ag::io;

    constexpr auto v = fixture<types::field_1>::value;
    ss << no_braces << v << '|' << v;
    auto out = ss.str();
    auto sep = out.find('|');

    REQUIRE(sep != std::string::npos);
    CHECK(out.substr(0, sep)  == fixture<types::field_1>::no_braces_expected);
    CHECK(out.substr(sep + 1) == fixture<types::field_1>::default_expected);
}

TEST_CASE("csl::ag::io indented manipulator is one-shot [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    std::ostringstream ss;
    using namespace csl::ag::io;
    
    constexpr auto v = fixture<types::field_1>::value;
    ss << indented << v << '|' << v;
    auto out = ss.str();
    auto sep = out.find('|');

    REQUIRE(sep != std::string::npos);
    CHECK(out.substr(0, sep)  == fixture<types::field_1>::indented_expected);
    CHECK(out.substr(sep + 1) == fixture<types::field_1>::default_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io default (braced) output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string(f::value) == f::default_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io no_braces output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string<no_braces>(f::value) == f::no_braces_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io indented output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string<indented>(f::value) == f::indented_expected);
}

TEST_CASE("csl::ag::io user operator<< preferred for ostream_formattable types [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    using namespace csl::ag::io;
    // printable_t: structured_bindable AND has user-defined operator<<.
    // - exact-match overload wins over our constrained template.
    CHECK(to_string(printable_t{42}) == "printable:42");
}

TEST_CASE("csl::ag::io ostream_formattable field: user operator<< used directly [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    using namespace csl::ag::io;
    // with_printable_field: { printable_t p; int x; }
    // Field p is ostream_formattable - user's operator<< is used (not recursive print).
    auto out = to_string(with_printable_field{.p = {42}, .x = 7}); // NOLINT(*-magic-numbers)
    CHECK(out == "{printable:42, 7}");
}

TEST_CASE("csl::ag::io empty aggregate [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    using namespace csl::ag::io;
    CHECK(to_string(types::empty{})            == "{}");
    CHECK(to_string<no_braces>(types::empty{}) == "");
}

TEMPLATE_TEST_CASE("csl::ag::io indexed output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string<indexed>(f::value) == f::indexed_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io typenamed output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string<typenamed>(f::value) == f::typenamed_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io indented+indexed+typenamed output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string<indented | indexed | typenamed>(f::value) == f::indented_indexed_typenamed_expected);
}

TEMPLATE_TEST_CASE("csl::ag::io::to_string composed view output [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::field_3_nested_tuplelike,
                   types::field_4_nested_range,
                   types::field_everything)
{
    using namespace csl::ag::io;
    using f = fixture<TestType>;
    CHECK(to_string(f::value | indented | indexed | typenamed) == f::indented_indexed_typenamed_expected);
}

#undef CSL_AG_BITFIELDS_STR

// NOLINTEND(*-avoid-magic-numbers)
// NOLINTEND(*cert-err58-cpp)
// NOLINTEND(*-use-anonymous-namespace)
// NOLINTEND(*-avoid-do-while)
