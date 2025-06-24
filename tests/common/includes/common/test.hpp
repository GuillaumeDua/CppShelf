#pragma once

#if defined(__cpp_lib_source_location) and __cpp_lib_source_location <=	201907L
# include <source_location>
using source_location_t = std::source_location;
#elif __has_include(<experimental/source_location>)
#include <experimental/source_location>
using source_location_t = std::experimental::source_location;
#else
# error "missing <source_location>"
#endif

#include <stdexcept>

namespace test {
    struct failure : std::runtime_error {
        explicit failure(source_location_t location = source_location_t::current())
        : std::runtime_error(location.function_name())
        {}
        using std::runtime_error::runtime_error;
        using std::runtime_error::what;
    };
}

#define csl_test_expect(expr) if (not (expr)) throw failure{}; // NOLINT(*-macro-usage)
