// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# define CSL_AG__ENABLE_FMTLIB_SUPPORT true // force fmtlib support. Prefer using CMake cache instead CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_FMTLIB_SUPPORT=ON"
#endif

namespace test::ag::types::owning {
    struct simple{ int i; };
    struct two_fields{ int i; char c; };
    struct nested {
        int i;
        simple field_1;
        two_fields field_2;
    };
    struct nested_std_tuplelike{
        bool b;
        std::tuple<int, char> tu;
        std::array<char, 3> a;
        std::pair<int, int> p;
    };

    // struct two_fields_inheritance : two_fields { }; // decomposes into 2 elements, but only 1 name was provided
}

namespace tests::concepts {
    namespace types = test::ag::types::owning;
    static_assert(csl::ag::io::concepts::fmt_formatter_is_csl_product<types::simple>);
    static_assert(not csl::ag::io::concepts::fmt_formatter_is_csl_product<int>);
    static_assert(not csl::ag::io::concepts::fmt_formatter_is_csl_product<std::tuple<int>>);
    static_assert(not csl::ag::io::concepts::fmt_formatter_is_csl_product<std::array<int, 3>>);
}

// WIP: check no clash with user-defined formatters -> complete, partial/generics, etc.

#include <cassert> // TODO(Guillaume): GoogleTest or Catch2 test-suite -> one test per type
#include <fmt/core.h>
#include <fmt/compile.h>
namespace test::ag::io {
    template <typename T>
    struct piece;

    template <typename T>
    constexpr void check(piece<T>){
        assert(fmt::format(FMT_COMPILE("{}"), piece<T>::value) == piece<T>::expected_result); // NOTE: not compile-time for now.
    }

    template <>
    struct piece<test::ag::types::owning::simple>{
        constexpr static inline test::ag::types::owning::simple value{ .i = 42 };
        constexpr static inline std::string_view expected_result = "{42}";
        // TODO(Guillaume): expected_result_compact
        // TODO(Guillaume): expected_result_pretty
    };
    template <>
    struct piece<test::ag::types::owning::two_fields>{
        constexpr static inline test::ag::types::owning::two_fields value{ .i = 42, .c = 'A' };
        constexpr static inline std::string_view expected_result = "{42, A}";
    };
    template <>
    struct piece<test::ag::types::owning::nested>{
        constexpr static inline test::ag::types::owning::nested value{
            .i = 1,
            .field_1 = { .i = 12 },
            .field_2 = { .i = 123, .c = 'A'}
        };
        constexpr static inline std::string_view expected_result = "{1, {12}, {123, A}}";
    };
    template <>
    struct piece<test::ag::types::owning::nested_std_tuplelike>{
        constexpr static inline test::ag::types::owning::nested_std_tuplelike value{
            .b = true,
            .tu = { 2, 'b'},
            .a = {'a', 'b', 'c'},
            .p = { 42, 43 }, // NOLINT
        };
        constexpr static inline std::string_view expected_result = "{true, (2, 'b'), ['a', 'b', 'c'], (42, 43)}";
    };
}

#include <tuple>
#include <type_traits>

auto main() -> int {
    using namespace test::ag;
    namespace types = test::ag::types::owning;

    constexpr auto to_test = std::tuple{
        std::type_identity<types::simple>{},
        std::type_identity<types::two_fields>{},
        std::type_identity<types::nested>{},
        std::type_identity<types::nested_std_tuplelike>{},
        // std::type_identity<types::two_fields_inheritance>{},
    };

    []<csl::ag::io::concepts::formattable ... Ts>(std::tuple<std::type_identity<Ts>...>){
        ((io::check(io::piece<Ts>{})), ...);
    }(to_test);
}
