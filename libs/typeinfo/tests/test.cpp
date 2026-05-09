#include <csl/typeinfo.hpp>
#include <catch2/catch_test_macros.hpp>

namespace test::value_name::enums {
    enum global_ns_colors : int { red, blue, yellow, orange, green, purple };
}

TEST_CASE("typeinfo::type_name", "[typeinfo][compile_time]") {
    STATIC_REQUIRE(csl::typeinfo::type_name<int{ 42 }>() == "int"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

TEST_CASE("typeinfo::value_name", "[typeinfo][compile_time]") {
#if defined(__GNUC__) or defined(__clang__)
    STATIC_REQUIRE(csl::typeinfo::value_name<int{ 42 }>() == "42"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
#else // MSVC
    STATIC_REQUIRE(csl::typeinfo::value_name<int(42)>() == "0x2a");
#endif
}

TEST_CASE("typeinfo::value_name::enums", "[typeinfo][compile_time]") {
    STATIC_REQUIRE(csl::typeinfo::type_name<test::value_name::enums::global_ns_colors::red>() == "test::value_name::enums::global_ns_colors");
    STATIC_REQUIRE(csl::typeinfo::value_name<test::value_name::enums::global_ns_colors::red>() == "test::value_name::enums::red");
}
