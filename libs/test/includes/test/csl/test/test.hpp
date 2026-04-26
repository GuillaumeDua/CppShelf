#pragma once

// Minimalistic C++17 testing framework: https://godbolt.org/z/sTb6aza87,
//  supports <source_location> if available.
//  For instance, not when using C++17 with libc++
//
// Prefer Catch2 or GoogleTest when available.

#if __cplusplus < 201703L
 #error "csl/mp.hpp requires C++17 or greater"
#endif

#if __has_include(<version>)
# include <version>
#endif

#if defined(__cpp_lib_source_location) and __cpp_lib_source_location >= 201907L
# include <source_location>
namespace csl::test::details {
using source_location = std::source_location;
}
#elif __has_include(<experimental/source_location>)
# pragma message("csl::test: using <experimental/source_location>. Prefer <source_location> if available.")
# include <experimental/source_location>
namespace csl::test::details {
using source_location = std::experimental::source_location;
}
#else
namespace csl::test::details {
using source_location = void;
}
#endif

#include <stdexcept>

namespace csl::test {
    struct failure : std::runtime_error {

        // NOLINTBEGIN(modernize-use-constraints)
        template <
            typename source_location = csl::test::details::source_location,
            std::enable_if_t<not std::is_void_v<source_location>, int> = 0
        >
        explicit failure(source_location location = source_location::current())
        : std::runtime_error(location.function_name())
        {}

        template <
            typename source_location = csl::test::details::source_location,
            std::enable_if_t<std::is_void_v<source_location>, int> = 0
        >
        explicit failure()
        : std::runtime_error("<source_location> not supported")
        {}
        // NOLINTEND(modernize-use-constraints)

        using std::runtime_error::runtime_error;
        using std::runtime_error::what;
    };
}

#define csl_test_expect(expr) if (not (expr)) throw csl::test::failure{}; // NOLINT(*-macro-usage)
