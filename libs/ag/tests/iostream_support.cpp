#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1

#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "ON"
#else
#  define CSL_AG_BITFIELDS_STR "OFF"
#endif

#include <csl/ag.hpp>
#include <tests/types.hpp>

#include <sstream>
#include <string>
#include <string_view>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace types = test::ag::types;

// Test-local type with user-defined operator<<.
// Must be at file scope (not anonymous namespace) so ADL finds it from header templates.
struct printable_t { int value; };
auto & operator<<(std::ostream & os, printable_t v) { return os << "printable:" << v.value; }

struct with_printable_field {
    printable_t p;
    int         x;
};

// --- helpers ---

template <typename T>
auto capture_default(const T & value) -> std::string {
    std::ostringstream ss;
    using namespace csl::ag::io;
    ss << value;
    return ss.str();
}

template <typename T>
auto capture_no_braces(const T & value) -> std::string {
    std::ostringstream ss;
    using namespace csl::ag::io;
    ss << no_braces << value;
    return ss.str();
}

template <typename T>
auto capture_indented(const T & value) -> std::string {
    std::ostringstream ss;
    using namespace csl::ag::io;
    ss << indented << value;
    return ss.str();
}

// --- compile-time checks ---

namespace tests::compile_time {

    using namespace csl::ag::io::details;

    // Fundamental types and common ostream-printable types
    static_assert(ostream_formattable<int>);
    static_assert(ostream_formattable<char>);
    static_assert(ostream_formattable<float>);
    static_assert(ostream_formattable<bool>);
    static_assert(ostream_formattable<std::string>);
    static_assert(ostream_formattable<std::string_view>);

    // Plain aggregates without user operator<< are NOT ostream_formattable
    static_assert(not ostream_formattable<types::field_1>);
    static_assert(not ostream_formattable<types::field_2>);
    static_assert(not ostream_formattable<types::field_3_nested>);
    static_assert(not ostream_formattable<types::empty>);

    // printable_t has a user-defined operator<< so it IS ostream_formattable
    static_assert(ostream_formattable<printable_t>);
}

// --- fixtures ---
//
// Differences from fmtlib_support.cpp fixtures:
//   - char values are unquoted   ('A' → A)
//   - string_view values are unquoted  ("str" → str)
//   - bool values are 0/1 (no std::boolalpha)

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
    };

    template <>
    struct fixture<types::field_2> {
        constexpr static types::field_2 value{ .i = 123, .c = 'A' };
        constexpr static std::string_view default_expected     = "{123, A}";
        constexpr static std::string_view no_braces_expected   = "123A";
        constexpr static std::string_view indented_expected =
R"({
    123,
    A
})";
    };

    template <>
    struct fixture<types::field_3_nested> {
        constexpr static types::field_3_nested value{
            .i  = 1,
            .f1 = fixture<types::field_1>::value,
            .f2 = fixture<types::field_2>::value
        };
        constexpr static std::string_view default_expected     = "{1, {42}, {123, A}}";
        constexpr static std::string_view no_braces_expected   = "1{42}{123, A}";
        constexpr static std::string_view indented_expected =
R"({
    1,
    {
        42
    },
    {
        123,
        A
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
        // std::tuple → (), std::array → [], std::pair → ()
        // chars and string_view unquoted via iostream
        constexpr static std::string_view default_expected     = "{(2, b, str), [a, b, c], (42, 43)}";
        constexpr static std::string_view no_braces_expected   = "(2, b, str)[a, b, c](42, 43)";
        constexpr static std::string_view indented_expected =
R"({
    (
        2,
        b,
        str
    ),
    [
        a,
        b,
        c
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
            .sv   = "hello",
            .a_c  = { 'a', 'b', 'c' },
            .a_i  = { 42, 43, 44 },
            .a_sv = { "a", "b", "c" },
        };
        // sv: string_view — ostream_formattable, unquoted
        // arrays: range-like tuple → []
        constexpr static std::string_view default_expected     = "{hello, [a, b, c], [42, 43, 44], [a, b, c]}";
        constexpr static std::string_view no_braces_expected   = "hello[a, b, c][42, 43, 44][a, b, c]";
        constexpr static std::string_view indented_expected =
R"({
    hello,
    [
        a,
        b,
        c
    ],
    [
        42,
        43,
        44
    ],
    [
        a,
        b,
        c
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
        // bool: 0/1 via iostream (no boolalpha)
        constexpr static std::string_view default_expected =
            "{1, {1, {42}, {123, A}}, {(2, b, str), [a, b, c], (42, 43)}, {hello, [a, b, c], [42, 43, 44], [a, b, c]}}";
        constexpr static std::string_view no_braces_expected =
            "1{1, {42}, {123, A}}{(2, b, str), [a, b, c], (42, 43)}{hello, [a, b, c], [42, 43, 44], [a, b, c]}";
        constexpr static std::string_view indented_expected =
R"({
    1,
    {
        1,
        {
            42
        },
        {
            123,
            A
        }
    },
    {
        (
            2,
            b,
            str
        ),
        [
            a,
            b,
            c
        ],
        (
            42,
            43
        )
    },
    {
        hello,
        [
            a,
            b,
            c
        ],
        [
            42,
            43,
            44
        ],
        [
            a,
            b,
            c
        ]
    }
})";
    };

} // namespace

// --- tests ---

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
    // After printing with no_braces, the next print should revert to braced.
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
    using f = fixture<TestType>;
    CHECK(capture_default(f::value) == f::default_expected);
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
    using f = fixture<TestType>;
    CHECK(capture_no_braces(f::value) == f::no_braces_expected);
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
    using f = fixture<TestType>;
    CHECK(capture_indented(f::value) == f::indented_expected);
}

TEST_CASE("csl::ag::io user operator<< preferred for ostream_formattable types [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    // printable_t IS structured_bindable (aggregate) AND has user operator<<.
    // The user's exact-match overload wins over our constrained template.
    CHECK(capture_default(printable_t{7}) == "printable:7");
}

TEST_CASE("csl::ag::io ostream_formattable field: user operator<< used directly [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    // with_printable_field: { printable_t p; int x; }
    // Field p is ostream_formattable — user's operator<< is used (not recursive print).
    auto out = capture_default(with_printable_field{.p = {42}, .x = 7});
    CHECK(out == "{printable:42, 7}");
}

TEST_CASE("csl::ag::io empty aggregate [BITFIELDS=" CSL_AG_BITFIELDS_STR "]",
          "[ag][iostream]")
{
    CHECK(capture_default(types::empty{})   == "{}");
    CHECK(capture_no_braces(types::empty{}) == "");
}

#undef CSL_AG_BITFIELDS_STR
