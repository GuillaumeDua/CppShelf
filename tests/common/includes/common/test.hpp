#pragma once

// mini C++17 testing framework: https://godbolt.org/z/sTb6aza87,
// which may support <source_location> if available.
// For instance, not when using C++17 with libc++

#if defined(__cpp_lib_source_location) and __cpp_lib_source_location <=	201907L
# include <source_location>
namespace test::details {
using source_location = std::source_location;
}
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
namespace test::details {
using source_location = std::experimental::source_location;
}
#else
namespace test::details {
    using source_location = void;
}
#endif

#include <stdexcept>

namespace test {
    struct failure : std::runtime_error {

        template <
            typename source_location = test::details::source_location,
            std::enable_if_t<not std::is_void_v<source_location>, int> = 0
        >
        explicit failure(source_location location = source_location::current())
        : std::runtime_error(location.function_name())
        {}

        template <
            typename source_location = test::details::source_location,
            std::enable_if_t<std::is_void_v<source_location>, int> = 0
        >
        explicit failure()
        : std::runtime_error("location not supported")
        {}

        using std::runtime_error::runtime_error;
        using std::runtime_error::what;
    };
}

#define csl_test_expect(expr) if (not (expr)) throw test::failure{}; // NOLINT(*-macro-usage)
