#include <csl/typeinfo.hpp>

namespace test::type_name {
    static_assert(csl::typeinfo::type_name<int{ 42 }>() == "int"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}
namespace test::value_name {
#if defined(__GNUC__) or defined(__clang__)
    static_assert(csl::typeinfo::value_name<int{ 42 }>() == "42"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
#else // MSVC
    static_assert(csl::typeinfo::value_name<int(42)>() == "0x2a");
#endif
}
namespace test::value_name::enums {

    enum global_ns_colors : int {
        red,
        blue,
        yellow,
        orange,
        green,
        purple
    };

    static_assert(csl::typeinfo::type_name<global_ns_colors::red>() == "test::value_name::enums::global_ns_colors");
    static_assert(csl::typeinfo::value_name<global_ns_colors::red>() == "test::value_name::enums::red");
}

auto main() -> int {}
