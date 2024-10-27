// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# define CSL_AG__ENABLE_FMTLIB_SUPPORT true // force fmtlib support. Prefer using CMake cache instead CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_FMTLIB_SUPPORT=ON"
#endif

namespace test::ag::types::owning {
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
    namespace types = test::ag::types::owning;

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

#include <cassert> // TODO(Guillaume): GoogleTest or Catch2 test-suite -> one test per type
#include <fmt/core.h>

namespace test::ag::io {
    template <typename T>
    struct piece;

    template <typename T>
    /*constexpr*/ static void check(piece<T>){
        //fmt::print(FMT_COMPILE("{}\n{}\n\n"), piece<T>::value, piece<T>::expected_result);
        assert(fmt::format("{}", piece<T>::value) == piece<T>::expected_result); // NOTE: not compile-time for now.
    }

    template <>
    struct piece<test::ag::types::owning::one_field>{
        constexpr static test::ag::types::owning::one_field value{ .i = 42 };
        constexpr static std::string_view expected_result = "{42}";
        // TODO(Guillaume): expected_result_compact
        // TODO(Guillaume): expected_result_default = expected_result_compact
        // TODO(Guillaume): expected_result_pretty
    };
    template <>
    struct piece<test::ag::types::owning::two_fields>{
        constexpr static test::ag::types::owning::two_fields value{ .i = 42, .c = 'A' };
        constexpr static std::string_view expected_result = "{42, A}";
    };
    template <>
    struct piece<test::ag::types::owning::nested>{
        constexpr static test::ag::types::owning::nested value{
            .i = 1,
            .field_1 = { .i = 12 },
            .field_2 = { .i = 123, .c = 'A'}
        };
        constexpr static std::string_view expected_result = "{1, {12}, {123, A}}";
    };
    template <>
    struct piece<test::ag::types::owning::nested_std_tuplelike>{
        constexpr static test::ag::types::owning::nested_std_tuplelike value{
            .b = true,
            .sv = "hello",
            .tu = { 2, 'b', "str"},
            .a = {'a', 'b', 'c'},
            .p = { 42, 43 }, // NOLINT
        };
        constexpr static inline std::string_view expected_result = "{true, hello, (2, b, str), [a, b, c], (42, 43)}";
    };
}

#include <tuple>
#include <type_traits>

/*
    What to test ?

    - dispatched to the right underlying formatter (ex: char -> ['\x00'] vs. [], ranges -> [{a,b,c}] vs. [a,b,c], etc.)

    - default formatter
    - default formatter with :n parse
    - indented
*/

auto main() -> int {
    using namespace test::ag;
    namespace types = test::ag::types::owning;

    constexpr auto to_test = std::tuple{
        std::type_identity<types::one_field>{},
        std::type_identity<types::two_fields>{},
        std::type_identity<types::nested>{},
        std::type_identity<types::nested_std_tuplelike>{},
        // std::type_identity<types::two_fields_inheritance>{},
    };

    // []<csl::ag::io::concepts::formattable ... Ts>(std::tuple<std::type_identity<Ts>...>){
    //     ((io::check(io::piece<Ts>{})), ...);
    // }(to_test);


    // WIP
    const auto value = test::ag::types::owning::nested{
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
    fmt::println("{}, {}", std::tuple{char{}}, std::tuple{ 'a'});
    fmt::println("{}, {}", std::vector{char{}}, std::vector{'a'});

    using namespace csl::ag::io;
    fmt::println(
        "Chars: {}, {}",
        csl::ag::io::details::decorators::depthen_view_t{char{}},
        csl::ag::io::details::decorators::depthen_view_t{'a'}
    );
    fmt::println("default    : [{}]",  test::ag::types::owning::two_fields{});
    fmt::println("indented   : [{}]", test::ag::types::owning::two_fields{} | indented);

    // fmt::println("default    : [{}]", value);
    // fmt::println("indented   : [{}]", value | indented);

    // fmt::println("compact    : [{:c}]", value);
    // // WIP: https://godbolt.org/z/ovv8eoqq8
    // fmt::println("pretty     : [\n{:i}\n]", value);
    // fmt::println("pretty(I)  : [\n{:i,I}\n]", value);
    // fmt::println("pretty(T)  : [\n{:i,T}\n]", value);
    // fmt::println("pretty(IT) : [\n{:i,IT}\n]", value);
}
