// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# define CSL_AG__ENABLE_FMTLIB_SUPPORT true // force fmtlib support. Prefer using CMake cache instead CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_FMTLIB_SUPPORT=ON"
#endif

namespace tests::ag::types {
    // TODO(Guillaume): handle empty ?
    struct one_field{ int i; };
    struct two_fields{ int i; char c; };
    struct nested {
        int i;
        one_field field_1;
        two_fields field_2;
    };
    struct nested_std_tuplelike{
        bool b;
        std::string_view sv;
        std::tuple<int, char, std::string_view> tu;
        std::array<char, 3> a;
        std::pair<int, int> p;
    };

    // BUG: struct two_fields_inheritance : two_fields { }; // decomposes into 2 elements, but only 1 name was provided
}

namespace tests::concepts {
    namespace types = tests::ag::types;

    static_assert(csl::ag::concepts::produced<fmt::formatter<types::one_field>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::two_fields>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::nested>>);
    static_assert(csl::ag::concepts::produced<fmt::formatter<types::nested_std_tuplelike>>);

    // not impacting <fmt/format.h>
    static_assert(not csl::ag::concepts::produced<fmt::formatter<int>>);
    // not impacting <fmt/ranges.h>
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::string>>);
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::vector<int>>>);
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::vector<types::one_field>>>);
    // not impacting <fmt/ranges.h> - tuplelikes
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::tuple<int>>>);
    static_assert(not csl::ag::concepts::produced<fmt::formatter<std::array<int, 3>>>);
}

// WIP: check possible clash with user-defined formatters -> complete, partial/generics, etc.

/*  // WIP
    What to test ?

    - dispatched to the right underlying formatter (ex: char -> ['\x00'] vs. [], ranges -> [{a,b,c}] vs. [a,b,c], etc.)

    - default formatter
    - default formatter with :n parse
    - indented
*/

#include <gtest/gtest.h>

using test_models = ::testing::Types<
    tests::ag::types::one_field,
    tests::ag::types::two_fields,
    tests::ag::types::nested,
    tests::ag::types::nested_std_tuplelike
>;
TYPED_TEST_SUITE(csl_test_ag_FmtFormatAggregate, test_models);

template <typename T>
struct csl_test_ag_FmtFormatAggregate : public testing::Test {};
#pragma region csl_test_ag_FmtFormatAggregate specializations
template <>
struct csl_test_ag_FmtFormatAggregate<tests::ag::types::one_field> : public testing::Test {
    constexpr static tests::ag::types::one_field value{ .i = 42 };
    constexpr static std::string_view default_formatter_expected_result = "{42}";
    constexpr static std::string_view default_formatter_n_expected_result = "42";
    constexpr static std::string_view indented_formatter_expected_result =
R"({
    42
})";
};

template <>
struct csl_test_ag_FmtFormatAggregate<tests::ag::types::two_fields> : public testing::Test {
    constexpr static tests::ag::types::two_fields value{ .i = 42, .c = 'A' };
    constexpr static std::string_view default_formatter_expected_result = "{42, 'A'}";
    constexpr static std::string_view default_formatter_n_expected_result = "42'A'";
    constexpr static std::string_view indented_formatter_expected_result =
R"({
    42,
    'A'
})";
};
template <>
struct csl_test_ag_FmtFormatAggregate<tests::ag::types::nested> : public testing::Test {
    constexpr static tests::ag::types::nested value{
        .i = 1,
        .field_1 = { .i = 12 },
        .field_2 = { .i = 123, .c = 'A'}
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
struct csl_test_ag_FmtFormatAggregate<tests::ag::types::nested_std_tuplelike> : public testing::Test {
    constexpr static tests::ag::types::nested_std_tuplelike value{
        .b = true,
        .sv = "hello",
        .tu = { 2, 'b', "str"},
        .a = {'a', 'b', 'c'},
        .p = { 42, 43 }, // NOLINT
    };
    constexpr static std::string_view default_formatter_expected_result = R"({true, "hello", (2, 'b', "str"), ['a', 'b', 'c'], (42, 43)})";
    constexpr static std::string_view default_formatter_n_expected_result = R"(true"hello"(2, 'b', "str")['a', 'b', 'c'](42, 43))";
    constexpr static std::string_view indented_formatter_expected_result =
R"({
    true,
    "hello",
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
    using namespace tests::ag;
    namespace types = tests::ag::types;

    constexpr auto values = std::tuple{
        std::type_identity<types::one_field>{},
        std::type_identity<types::two_fields>{},
        std::type_identity<types::nested>{},
        std::type_identity<types::nested_std_tuplelike>{},
        // std::type_identity<types::two_fields_inheritance>{},
    };

    // WIP
    const auto value = tests::ag::types::nested{
        .i = 1,
        .field_1 = {
            11,
        },
        .field_2 = {
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

    // fmt::println(fmt::runtime("indented (n) : [{:n}]"), tests::ag::types::two_fields{} | indented);
    
    // fmt::println("\ntwo_fields:");
    // fmt::println("default      : [{}]",   tests::ag::types::two_fields{});
    // fmt::println("indented     : [{}]",   tests::ag::types::two_fields{} | indented);
    // fmt::println("default  (n) : [{:n}]", tests::ag::types::two_fields{});
    // fmt::println("indented (n) : [{:n}]", tests::ag::types::two_fields{} | indented);

    // fmt::println("\nnested_std_tuplelike:");
    // fmt::println("default      : [{}]",   tests::ag::types::nested_std_tuplelike{});
    // fmt::println("indented     : [{}]",   tests::ag::types::nested_std_tuplelike{} | indented);
    // fmt::println("default  (n) : [{:n}]", tests::ag::types::nested_std_tuplelike{});
    // fmt::println("indented (n) : [{:n}]", tests::ag::types::nested_std_tuplelike{} | indented);
    

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
