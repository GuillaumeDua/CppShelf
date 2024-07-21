// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#include <type_traits>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# define CSL_AG__ENABLE_FMTLIB_SUPPORT true // force fmtlib support. Prefer using CMake cache instead CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#endif
#include <csl/ag.hpp>
#if not defined(CSL_AG__ENABLE_FMTLIB_SUPPORT)
# error "[Test] csl::ag : expect CSL_AG__ENABLE_FMTLIB_SUPPORT=OFF"
#endif

namespace test::ag::types::owning {
    struct simple{ int i; };
    struct two_fields{ int i; char c; };
    // struct two_fields_inheritance : two_fields { }; // decomposes into 2 elements, but only 1 name was provided
}

// QUESTION: formattable type_traits + concept ?
// QUESTION: concept to check if csl::ag formatter or another
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
    };
    template <>
    struct piece<test::ag::types::owning::two_fields>{
        constexpr static inline test::ag::types::owning::two_fields value{ .i = 42, .c = 'A' };
        constexpr static inline std::string_view expected_result = "{42, A}";
    };
}

#include <tuple>

auto main() -> int {
    using namespace test::ag;
    namespace types = test::ag::types::owning;

    constexpr auto to_test = std::tuple{
        std::type_identity<types::simple>{},
        std::type_identity<types::two_fields>{},
        // std::type_identity<types::two_fields_inheritance>{},
    };

    []<typename ... Ts>(std::tuple<std::type_identity<Ts>...>){
        ((io::check(io::piece<Ts>{})), ...);
    }(to_test);
}
