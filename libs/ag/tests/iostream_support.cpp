#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#if not defined(FORCE_CSL_AG__ENABLE_FORMAT_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_FORMAT_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_FORMAT_SUPPORT
#if FORCE_CSL_AG__ENABLE_FORMAT_SUPPORT
#  define CSL_AG__ENABLE_FORMAT_SUPPORT 1
#endif

#define CSL_AG__ENABLE_IOSTREAM_SUPPORT 1

#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "BITFIELDS=ON"
#else
#  define CSL_AG_BITFIELDS_STR "BITFIELDS=OFF"
#endif
#if FORCE_CSL_AG__ENABLE_FORMAT_SUPPORT
#  define CSL_AG_FORMAT_STR ",FORMAT=ON"
#else
#  define CSL_AG_FORMAT_STR ",FORMAT=OFF"
#endif

#include <csl/ag.hpp>
#include <tests/types.hpp>

#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <array>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace types      = test::ag::types;
namespace io_details  = csl::ag::io::details;

// Test-local type with user-defined operator<<.
// Must be in the enclosing namespace (not anonymous) so ADL finds it from header templates.
struct printable_t { int value; };
auto & operator<<(std::ostream & os, printable_t v) { return os << "printable:" << v.value; }

// Aggregate whose first field has a user-defined operator<< (printable_t is structured_bindable)
struct with_printable_field {
    printable_t p;
    int         x;
};

// --- std::format backend test types ---
// Only compiled when FORMAT_SUPPORT is ON: write_value's std::format branch is then active.
#if defined(CSL_AG__ENABLE_FORMAT_SUPPORT) and CSL_AG__ENABLE_FORMAT_SUPPORT
#  include <format>

// Non-aggregate (user-declared ctor) with std::formatter but no operator<<.
// field_type: not structured_bindable, not ostream_formattable, IS std::formattable
// → print_field takes the write_value path → std::format("{}", v) is used.
struct format_only_t {
    int value;
    explicit format_only_t(int v) noexcept : value{v} {}
};
template <>
struct std::formatter<format_only_t> {
    constexpr auto parse(std::format_parse_context & ctx) { return ctx.begin(); }
    template <typename FormatContext>
    auto format(format_only_t v, FormatContext & ctx) const {
        return std::format_to(ctx.out(), "{}", v.value);
    }
};

// Aggregate wrapper so operator<<(indented_ostream, structured_bindable &&) recurses into it
struct with_format_only_field {
    format_only_t f{0};
};
#endif // CSL_AG__ENABLE_FORMAT_SUPPORT

// --- helpers ---

template <typename T>
auto capture(const T & value) -> std::string {
    std::ostringstream ss;
    using namespace csl::ag::io;
    ss << value;
    return ss.str();
}

template <typename T>
auto tn() -> std::string { return std::string{io_details::type_name_v<T>}; }

auto ind(std::size_t depth) -> std::string { return std::string(depth * 4, ' '); }

// --- compile-time checks ---

namespace tests::compile_time {

    using namespace io_details;

    // Fundamental types and common ostream-printable types
    static_assert(ostream_formattable<int>);
    static_assert(ostream_formattable<char>);
    static_assert(ostream_formattable<float>);
    static_assert(ostream_formattable<bool>);
    static_assert(ostream_formattable<std::string>);
    static_assert(ostream_formattable<std::string_view>);

    // Plain aggregates without user operator<< are NOT ostream_formattable
    static_assert(not ostream_formattable<types::field_1>);
    static_assert(not ostream_formattable<types::field_2>);
    static_assert(not ostream_formattable<types::field_3_nested>);
    static_assert(not ostream_formattable<types::empty>);

    // std::tuple, std::array, std::pair have no standard operator<<
    static_assert(not ostream_formattable<std::tuple<int>>);
    static_assert(not ostream_formattable<std::array<int, 3>>);
    static_assert(not ostream_formattable<std::pair<int, int>>);

    // printable_t has a user-defined operator<<, so it IS ostream_formattable
    static_assert(ostream_formattable<printable_t>);

#if defined(CSL_AG__ENABLE_FORMAT_SUPPORT) and CSL_AG__ENABLE_FORMAT_SUPPORT
    // format_only_t: user ctor → NOT aggregate/structured_bindable; no op<< → NOT ostream_formattable
    static_assert(not csl::ag::concepts::structured_bindable<format_only_t>);
    static_assert(not ostream_formattable<format_only_t>);
    static_assert(std::formattable<format_only_t, char>);
    // with_format_only_field IS structured_bindable (aggregate wrapper)
    static_assert(csl::ag::concepts::structured_bindable<with_format_only_field>);
#endif
}

// --- fixtures ---

namespace {

    template <typename T>
    struct fixture;

    template <>
    struct fixture<types::field_1> {
        constexpr static types::field_1 value{.i = 42};
        static auto expected() -> std::string {
            return tn<types::field_1>() + " {\n"
                 + "    [0] " + tn<int>() + " : 42\n"
                 + "}";
        }
    };

    template <>
    struct fixture<types::field_2> {
        constexpr static types::field_2 value{.i = 123, .c = 'A'};
        static auto expected() -> std::string {
            return tn<types::field_2>() + " {\n"
                 + "    [0] " + tn<int>()  + " : 123\n"
                 + "    [1] " + tn<char>() + " : A\n"
                 + "}";
        }
    };

    template <>
    struct fixture<types::field_3_nested> {
        constexpr static types::field_3_nested value{.i = 1, .f1 = {42}, .f2 = {123, 'A'}};
        static auto expected() -> std::string {
            return tn<types::field_3_nested>() + " {\n"
                 + "    [0] " + tn<int>() + " : 1\n"
                 + "    [1] " + tn<types::field_1>() + " {\n"
                 + "        [0] " + tn<int>() + " : 42\n"
                 + "    }\n"
                 + "    [2] " + tn<types::field_2>() + " {\n"
                 + "        [0] " + tn<int>()  + " : 123\n"
                 + "        [1] " + tn<char>() + " : A\n"
                 + "    }\n"
                 + "}";
        }
    };

    template <>
    struct fixture<types::empty> {
        constexpr static types::empty value{};
        static auto expected() -> std::string {
            return tn<types::empty>() + " {\n}";
        }
    };
}

// --- tests ---

TEST_CASE("csl::ag::io operator<< returns std::ostream & [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    std::ostringstream ss;
    using namespace csl::ag::io;
    std::ostream & result = (ss << types::field_1{42});
    CHECK(&result == static_cast<std::ostream *>(&ss));
}

TEST_CASE("csl::ag::io operator<< supports chaining [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    std::ostringstream ss;
    using namespace csl::ag::io;
    constexpr auto v = fixture<types::field_1>::value;
    ss << v << '|' << v;
    auto out = ss.str();
    auto sep = out.find('|');
    REQUIRE(sep != std::string::npos);
    auto expected = fixture<types::field_1>::expected();
    CHECK(out.substr(0, sep)  == expected);
    CHECK(out.substr(sep + 1) == expected);
}

TEMPLATE_TEST_CASE("csl::ag::io exact output [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
                   "[ag][iostream]",
                   types::field_1,
                   types::field_2,
                   types::field_3_nested,
                   types::empty)
{
    using f = fixture<TestType>;
    CHECK(capture(f::value) == f::expected());
}

TEST_CASE("csl::ag::io user operator<< preferred over recursive print [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    // printable_t IS structured_bindable (aggregate), but HAS a user operator<<.
    // When used as a standalone value, the user operator<< is used.
    CHECK(capture(printable_t{7}) == "printable:7");
}

TEST_CASE("csl::ag::io user operator<< field: no type-name prefix [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    // with_printable_field: { printable_t p; int x; }
    // Field [0] is printable_t — structured_bindable AND ostream_formattable.
    // Expected: type name prefix is suppressed; user's op<< output is used directly.
    auto out = capture(with_printable_field{.p = {42}, .x = 7});

    auto field0_prefix = ind(1) + "[0] ";
    auto pos = out.find(field0_prefix);
    REQUIRE(pos != std::string::npos);
    auto after = out.substr(pos + field0_prefix.size());
    auto line0  = after.substr(0, after.find('\n'));
    CHECK(line0 == "printable:42");

    // Field [1] is int — non-structured, ostream_formattable: type name is shown
    auto field1_prefix = ind(1) + "[1] " + tn<int>() + " : ";
    CHECK(out.find(field1_prefix + "7") != std::string::npos);
}

TEST_CASE("csl::ag::io tuple-like fields recurse with indentation [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    using T = types::field_3_nested_tuplelike;
    constexpr T value{
        .tu = {2, 'b', "str"},
        .a  = {'a', 'b', 'c'},
        .p  = {42, 43},
    };
    auto out = capture(value);

    // Outer structure
    CHECK(out.starts_with(tn<T>() + " {\n"));
    CHECK(out.back() == '}');

    // std::tuple field (index 0): should recurse, showing sub-indices
    CHECK(out.find(ind(1) + "[0] " + tn<std::tuple<int, char, std::string_view>>() + " {\n") != std::string::npos);

    // std::array field (index 1): should recurse
    CHECK(out.find(ind(1) + "[1] " + tn<std::array<char, 3>>() + " {\n") != std::string::npos);

    // std::pair field (index 2): should recurse
    CHECK(out.find(ind(1) + "[2] " + tn<std::pair<int, int>>() + " {\n") != std::string::npos);

    // Sub-element of the tuple: index 0 value = 2 (int)
    CHECK(out.find(ind(2) + "[0] " + tn<int>() + " : 2") != std::string::npos);
}

TEST_CASE("csl::ag::io ostream_formattable leaf includes type name and value [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    // field_4_nested_range: first field is std::string_view — ostream_formattable, non-structured.
    using T = types::field_4_nested_range;
    constexpr T value{
        .sv   = "hello",
        .a_c  = {'a', 'b', 'c'},
        .a_i  = {1, 2, 3},
        .a_sv = {"x", "y", "z"},
    };
    auto out = capture(value);

    // sv field [0]: ostream_formattable, not structured_bindable → "type_name : value"
    CHECK(out.find(ind(1) + "[0] " + tn<std::string_view>() + " : hello") != std::string::npos);

    // a_c field [1]: std::array — recurse; first char element at depth 2
    CHECK(out.find(ind(2) + "[0] " + tn<char>() + " : a") != std::string::npos);
}

TEST_CASE("csl::ag::io indentation depth is 4 spaces per level [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream]")
{
    auto out = capture(types::field_3_nested{.i = 0, .f1 = {0}, .f2 = {0, '?'}});

    // Depth-1 fields use 4 spaces
    CHECK(out.find("    [0] ") != std::string::npos);
    CHECK(out.find("    [1] ") != std::string::npos);
    CHECK(out.find("    [2] ") != std::string::npos);

    // Depth-2 fields use 8 spaces
    CHECK(out.find("        [0] ") != std::string::npos);

    // Closing brace of nested struct at depth 1 uses 4-space indent
    CHECK(out.find("    }") != std::string::npos);
}

#if defined(CSL_AG__ENABLE_FORMAT_SUPPORT) and CSL_AG__ENABLE_FORMAT_SUPPORT
TEST_CASE("csl::ag::io std::format backend used for non-ostream, non-aggregate fields [" CSL_AG_BITFIELDS_STR CSL_AG_FORMAT_STR "]",
          "[ag][iostream][format]")
{
    // format_only_t: NOT structured_bindable (user ctor), NOT ostream_formattable (no op<<),
    // IS std::formattable → write_value must choose the std::format branch.
    auto out = capture(with_format_only_field{format_only_t{42}});
    // Field [0]: type name + " : " + std::format("{}", format_only_t{42}) = "42"
    CHECK(out.find(ind(1) + "[0] " + tn<format_only_t>() + " : 42") != std::string::npos);
}
#endif // CSL_AG__ENABLE_FORMAT_SUPPORT

#undef CSL_AG_BITFIELDS_STR
#undef CSL_AG_FORMAT_STR
