// Force CSL_AG__ENABLE_FMTLIB_SUPPORT=ON
#include "fmt/base.h"
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
    // struct two_fields_inheritance : two_fields { }; //  decomposes into 2 elements, but only 1 name was provided
}

// QUESTION: formattable type_traits + concept ?
// QUESTION: concept to check if csl::ag formatter or another
// WIP: check no clash with user-defined formatters -> complete, partial/generics, etc.

namespace test::ag::io {
    template <typename T>
    struct result {
        using type = T;
        std::string value;
    };
}

#include <tuple>

auto main() -> int {
    using namespace test::ag;
    namespace types = test::ag::types::owning;

    auto expected_results = std::tuple{
        io::result<types::simple>{""},
        io::result<types::two_fields>{""},
        // io::result<types::two_fields_inheritance>{""},
    };

    []<typename ... Ts>(std::tuple<io::result<Ts>...>){
        ((fmt::println("{}", Ts{})), ...);
    }(expected_results);
}
