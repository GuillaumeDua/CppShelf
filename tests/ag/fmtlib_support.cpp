// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# define CSL_AG__ENABLE_FMTLIB_SUPPORT true // force fmtlib support. Prefer using CMake cache instead CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_FMTLIB_SUPPORT=ON"
#endif

#include <tests/types.hpp>

// WIP: check possible clash with user-defined formatters -> complete, partial/generics, etc.

namespace types = test::ag::types;

namespace tests::concepts::produced {

    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_1>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_2>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_3_nested>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::field_3_nested_tuplelike>>);

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

#include <gtest/gtest.h>

using test_models = ::testing::Types<
    types::field_1,
    types::field_2,
    types::field_3_nested,
    types::field_3_nested_tuplelike,
    types::field_4_nested_range
>;
TYPED_TEST_SUITE(csl_test_ag_FmtFormatAggregate, test_models);

template <typename T>
struct csl_test_ag_FmtFormatAggregate : public testing::Test {};
#pragma region csl_test_ag_FmtFormatAggregate specializations
template <>
struct csl_test_ag_FmtFormatAggregate<types::field_1> : public testing::Test {
    constexpr static types::field_1 value{ .i = 42 };
    constexpr static std::string_view default_formatter_expected_result = "{42}";
    constexpr static std::string_view default_formatter_n_expected_result = "42";
    constexpr static std::string_view indented_formatter_expected_result =
R"({
    42
})";
};

template <>
struct csl_test_ag_FmtFormatAggregate<types::field_2> : public testing::Test {
    constexpr static types::field_2 value{ .i = 42, .c = 'A' };
    constexpr static std::string_view default_formatter_expected_result = "{42, 'A'}";
    constexpr static std::string_view default_formatter_n_expected_result = "42'A'";
    constexpr static std::string_view indented_formatter_expected_result =
R"({
    42,
    'A'
})";
};
template <>
struct csl_test_ag_FmtFormatAggregate<types::field_3_nested> : public testing::Test {
    constexpr static types::field_3_nested value{
        .i = 1,
        .f1 = { .i = 12 },
        .f2 = { .i = 123, .c = 'A'}
    };
    constexpr static std::string_view default_formatter_expected_result = "{1, {12}, {123, 'A'}}";
    constexpr static std::string_view default_formatter_n_expected_result = "1{12}{123, 'A'}";
    constexpr static std::string_view indented_formatter_expected_result =
R"({
    1,
    {
        12
    },
    {
        123,
        'A'
    }
})";
};
template <>
struct csl_test_ag_FmtFormatAggregate<types::field_3_nested_tuplelike> : public testing::Test {
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
struct csl_test_ag_FmtFormatAggregate<types::field_4_nested_range> : public testing::Test {
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

#pragma endregion

TYPED_TEST(csl_test_ag_FmtFormatAggregate, DefaultFormatter) {

    EXPECT_EQ(
        fmt::format("{}", TestFixture::value),
        TestFixture::default_formatter_expected_result
    );
}
TYPED_TEST(csl_test_ag_FmtFormatAggregate, DefaultFormatterParseN) {

    EXPECT_EQ(
        fmt::format("{:n}", TestFixture::value),
        TestFixture::default_formatter_n_expected_result
    );
}
TYPED_TEST(csl_test_ag_FmtFormatAggregate, IndentedFormatter) {

    EXPECT_EQ(
        fmt::format("{}", TestFixture::value | csl::ag::io::indented),
        TestFixture::indented_formatter_expected_result
    );
    EXPECT_EQ(
        fmt::format("{}", csl::ag::io::details::decorators::depthen_view_t{ TestFixture::value }),
        TestFixture::indented_formatter_expected_result
    );
}


auto main(int argc, char *argv[]) -> int {

    // WIP
    const auto value = types::field_3_nested{
        .i = 1,
        .f1 = {
            11,
        },
        .f2 = {
            .i = 12,
            .c = 'c'
        }
    };
    static_assert(csl::ag::concepts::aggregate<std::remove_cvref_t<decltype(value)>>);
    static_assert(csl::ag::concepts::structured_bindable<std::remove_cvref_t<decltype(value)>>);

    fmt::println("{}, {}", char{}, 'a');
    fmt::println("{}, {}", std::tuple{char{}}, std::tuple{ 'a' });
    fmt::println("{:n}, {:n}", std::tuple{char{}, 42 }, std::tuple{ 'a', 42 });
    fmt::println("{}, {}", std::vector{char{}}, std::vector{'a'});

    using namespace csl::ag::io;
    fmt::println(
        "Chars: {}, {}",
        csl::ag::io::details::decorators::depthen_view_t{char{}},
        csl::ag::io::details::decorators::depthen_view_t{'a'}
    );

    // fmt::println(fmt::runtime("indented (n) : [{:n}]"), types::field_2{} | indented);
    
    // fmt::println("\nfield_2:");
    // fmt::println("default      : [{}]",   types::field_2{});
    // fmt::println("indented     : [{}]",   types::field_2{} | indented);
    // fmt::println("default  (n) : [{:n}]", types::field_2{});
    // fmt::println("indented (n) : [{:n}]", types::field_2{} | indented);

    // fmt::println("\nfield_3_nested_tuplelike:");
    // fmt::println("default      : [{}]",   types::field_3_nested_tuplelike{});
    // fmt::println("indented     : [{}]",   types::field_3_nested_tuplelike{} | indented);
    // fmt::println("default  (n) : [{:n}]", types::field_3_nested_tuplelike{});
    // fmt::println("indented (n) : [{:n}]", types::field_3_nested_tuplelike{} | indented);
    

    // fmt::println("default    : [{}]", value);
    // fmt::println("indented   : [{}]", value | indented);

    // fmt::println("compact    : [{:c}]", value);
    // // WIP: https://godbolt.org/z/ovv8eoqq8
    // fmt::println("pretty     : [\n{:i}\n]", value);
    // fmt::println("pretty(I)  : [\n{:i,I}\n]", value);
    // fmt::println("pretty(T)  : [\n{:i,T}\n]", value);
    // fmt::println("pretty(IT) : [\n{:i,IT}\n]", value);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
