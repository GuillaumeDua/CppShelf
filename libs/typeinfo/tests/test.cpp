#include <csl/typeinfo.hpp>
#include <catch2/catch_test_macros.hpp>

namespace test::value_name::enums {
    enum global_ns_colors : int { red, blue, yellow, orange, green, purple };
}

TEST_CASE("typeinfo::type_name", "[typeinfo][compile_time]") {
    STATIC_REQUIRE(csl::typeinfo::type_name_v<decltype(int{ 42 })> == "int"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
}

TEST_CASE("typeinfo::value_name", "[typeinfo][compile_time]") {
#if defined(__GNUC__) or defined(__clang__)
    STATIC_REQUIRE(csl::typeinfo::value_name_v<int{ 42 }> == "42"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
#else // MSVC
    STATIC_REQUIRE(csl::typeinfo::value_name_v<int{ 42 }> == "0x2a");
#endif
}

TEST_CASE("typeinfo::value_name::enums", "[typeinfo][compile_time]") {
    STATIC_REQUIRE(csl::typeinfo::type_name_v<decltype(test::value_name::enums::global_ns_colors::red)> == "test::value_name::enums::global_ns_colors");
    STATIC_REQUIRE(csl::typeinfo::value_name_v<test::value_name::enums::global_ns_colors::red> == "test::value_name::enums::red");
}

TEST_CASE("typeinfo::type_name_as", "[typeinfo][compile_time]") {
    STATIC_REQUIRE(csl::typeinfo::type_name_as_v<int, char>     ==  "int");
    STATIC_REQUIRE(csl::typeinfo::type_name_as_v<int, wchar_t>  == L"int");
    STATIC_REQUIRE(csl::typeinfo::type_name_as_v<int, char16_t> == u"int");
    STATIC_REQUIRE(csl::typeinfo::type_name_as_v<int, char32_t> == U"int");

    using colors = test::value_name::enums::global_ns_colors;
    STATIC_REQUIRE(csl::typeinfo::type_name_as_v<colors, wchar_t> == L"test::value_name::enums::global_ns_colors");
    STATIC_REQUIRE(csl::typeinfo::type_name_as_v<colors, wchar_t>.size() == csl::typeinfo::type_name_v<colors>.size());
}

TEST_CASE("typeinfo::value_name_as", "[typeinfo][compile_time]") {
#if defined(__GNUC__) or defined(__clang__)
    STATIC_REQUIRE(csl::typeinfo::value_name_as_v<int{ 42 }, wchar_t> == L"42"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
#else // MSVC
    STATIC_REQUIRE(csl::typeinfo::value_name_as_v<int{ 42 }, wchar_t> == L"0x2a"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
#endif
    STATIC_REQUIRE(
        csl::typeinfo::value_name_as_v<test::value_name::enums::global_ns_colors::red, wchar_t>
        == L"test::value_name::enums::red"
    );
}
