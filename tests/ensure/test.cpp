#include <csl/ensure.hpp>

namespace test::strong_types::comparisons {
    using meters = csl::ensure::strong_types<int, struct meters_tag>;
    static_assert(42 == meters{ 42 });
    static_assert(meters{ 42 } == 42);
    static_assert(meters{ 42 } == meters{ 42 });
}

auto main() -> int {
    
}