#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>
#include <csl/ag/formatting/backend/fmt.hpp>
#include <csl/ag/formatting/backend/ostream.hpp>
#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
#   include <csl/ag/formatting/typeinfo.hpp>
#endif
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>
#include <tests/ag/format_fixtures.hpp>
#include <tests/ag/format_optins_std.hpp>
#include <tests/ag/format_optins_fmt.hpp>

// NOTE: fmtlib exposes wide formatting (fmt::format(L"..."), fmt::wformat_context) through this header only.
#include <fmt/xchar.h>

#include <array>
#include <format>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

namespace types = test::ag::types;

// The wide-formattable subset of the shared test types: every leaf is formattable as wchar_t.
// Types carrying std::string_view / std::string fields are narrow-only by construction -
// the standard provides no std::formatter<std::string_view, wchar_t>.

// NOLINTBEGIN(cert-dcl58-cpp) - std::formatter is a CPO
template <>
struct std::formatter<types::field_1, wchar_t>
    : csl::ag::formatting::std_formatter<types::field_1, wchar_t>{};
template <>
struct std::formatter<types::field_2, wchar_t>
    : csl::ag::formatting::std_formatter<types::field_2, wchar_t>{};
template <>
struct std::formatter<types::field_3_nested, wchar_t>
    : csl::ag::formatting::std_formatter<types::field_3_nested, wchar_t>{};
// NOLINTEND(cert-dcl58-cpp)

template <>
struct fmt::formatter<types::field_1, wchar_t>
    : csl::ag::formatting::fmt_formatter<types::field_1, wchar_t>{};
template <>
struct fmt::formatter<types::field_2, wchar_t>
    : csl::ag::formatting::fmt_formatter<types::field_2, wchar_t>{};
template <>
struct fmt::formatter<types::field_3_nested, wchar_t>
    : csl::ag::formatting::fmt_formatter<types::field_3_nested, wchar_t>{};

namespace tests::concepts::supported_char_type {

    namespace formatting = csl::ag::formatting;

    static_assert(formatting::concepts::supported_char_type<char>);
    static_assert(formatting::concepts::supported_char_type<wchar_t>);

    // Tracks [format.formatter.spec], which provides standard formatter specializations for char and wchar_t only.
    static_assert(not formatting::concepts::supported_char_type<char8_t>);
    static_assert(not formatting::concepts::supported_char_type<char16_t>);
    static_assert(not formatting::concepts::supported_char_type<char32_t>);
    static_assert(not formatting::concepts::supported_char_type<int>);

    using view = formatting::details::decorators::formatted_view_t<types::field_2>;

    static_assert(formatting::details::concepts::std_formattable<types::field_3_nested, wchar_t>);
    static_assert(formatting::details::concepts::fmt_formattable<types::field_3_nested, wchar_t>);

    static_assert(std::formattable<view, char>);
    static_assert(std::formattable<view, wchar_t>);
    static_assert(fmt::formattable<view, wchar_t>);

    // Unsupported character types must report as not-formattable, never fail to compile.
    static_assert(not std::formattable<view, char8_t>);
    static_assert(not std::formattable<view, char32_t>);

    // Narrow-only leaves stay narrow-only.
    static_assert(formatting::details::concepts::std_formattable<types::field_4_nested_range, char>);
    static_assert(not formatting::details::concepts::std_formattable<types::field_4_nested_range, wchar_t>);
}

namespace {

    [[nodiscard]] auto widen(std::string_view value) -> std::wstring {
        return { value.begin(), value.end() };
    }

#if defined(CSL_AG__ENABLE_BITFIELDS_SUPPORT) and CSL_AG__ENABLE_BITFIELDS_SUPPORT
    constexpr bool bitfields_enabled = true;
#else
    constexpr bool bitfields_enabled = false;
#endif

#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
    constexpr bool typeinfo_linked = true;
#else
    constexpr bool typeinfo_linked = false;
#endif

    [[nodiscard]] auto const & name_suffix() {
        static const std::string value = std::format(
            " [FORMATTING=wchar_t] [BITFIELDS={}] [TYPEINFO={}]",
            bitfields_enabled ? "ON" : "OFF",
            typeinfo_linked ? "ON" : "OFF"
        );
        return value;
    }
} // namespace

#define WIDE_TEST_TYPES types::field_1, types::field_2, types::field_3_nested

TEMPLATE_TEST_CASE("std::format - wchar_t" + name_suffix() + "", "[ag][wchar_t][std]", WIDE_TEST_TYPES) {
    using f = fixture<TestType>;

    CHECK(std::format(L"{}",   f::value) == widen(f::default_expected));
    CHECK(std::format(L"{:i}", f::value) == widen(f::indented_expected));
    CHECK(std::format(L"{:x}", f::value) == widen(f::indexed_expected));
    CHECK(std::format(L"{:n}", f::value) == widen(f::no_braces_expected));

    CHECK(std::format(L"{}", f::value | csl::ag::formatting::indented) == widen(f::indented_expected));
    CHECK(std::format(L"{}", f::value | csl::ag::formatting::indexed)  == widen(f::indexed_expected));
}

TEMPLATE_TEST_CASE("fmt::format - wchar_t" + name_suffix() + "", "[ag][wchar_t][fmt]", WIDE_TEST_TYPES) {
    using f = fixture<TestType>;

    CHECK(fmt::format(L"{}",   f::value) == widen(f::default_expected));
    CHECK(fmt::format(L"{:i}", f::value) == widen(f::indented_expected));
    CHECK(fmt::format(L"{:x}", f::value) == widen(f::indexed_expected));
    CHECK(fmt::format(L"{:n}", f::value) == widen(f::no_braces_expected));
}

TEMPLATE_TEST_CASE("std::wostream" + name_suffix() + "", "[ag][wchar_t][ostream]", WIDE_TEST_TYPES) {
    using f = fixture<TestType>;
    using namespace csl::ag::formatting;

    const auto printed = [](auto options) {
        std::wostringstream os;
        os << options << f::value;
        return os.str();
    };

    CHECK(printed(format_options::none)     == widen(f::default_expected));
    CHECK(printed(format_options::indented) == widen(f::indented_expected));
    CHECK(printed(format_options::indexed)  == widen(f::indexed_expected));
    CHECK(printed(format_options::no_braces) == widen(f::no_braces_expected));

    // formatted_view_t bypasses iword
    std::wostringstream view_os;
    view_os << (f::value | indented);
    CHECK(view_os.str() == widen(f::indented_expected));
}

// Narrow and wide output must agree, character for character.
// Unlike the fixture-based cases above this is ungated, so the `:t` spec exercises the type-name
// widening in BOTH modes: the csl::typeinfo bridge (constexpr names) and the <typeindex> runtime
// fallback, whose std::string_view has no compile-time length to widen into storage.
TEMPLATE_TEST_CASE("narrow/wide parity" + name_suffix() + "", "[ag][wchar_t]", WIDE_TEST_TYPES) {
    auto value = fixture<TestType>::value;

    constexpr auto specs = std::array{
        std::pair{std::string_view{"{}"},     std::wstring_view{L"{}"}},
        std::pair{std::string_view{"{:i}"},   std::wstring_view{L"{:i}"}},
        std::pair{std::string_view{"{:x}"},   std::wstring_view{L"{:x}"}},
        std::pair{std::string_view{"{:t}"},   std::wstring_view{L"{:t}"}},
        std::pair{std::string_view{"{:n}"},   std::wstring_view{L"{:n}"}},
        std::pair{std::string_view{"{:ixt}"}, std::wstring_view{L"{:ixt}"}}
    };

    for (const auto & [narrow_spec, wide_spec] : specs) {
        INFO("spec: " << narrow_spec);
        CHECK(
            std::vformat(wide_spec, std::make_wformat_args(value))
            == widen(std::vformat(narrow_spec, std::make_format_args(value)))
        );
        CHECK(
            fmt::vformat(fmt::wstring_view{wide_spec}, fmt::make_wformat_args(value))
            == widen(fmt::vformat(narrow_spec, fmt::make_format_args(value)))
        );
    }
}

// Narrow and wide streams share a single ios_base::xalloc() slot: a manipulator set on one
// must not be observed by the other, and both must decode the same option bits.
TEST_CASE("std::wostream - iword slot is shared with narrow streams" + name_suffix(), "[ag][wchar_t][ostream]") {
    using f = fixture<types::field_2>;
    using namespace csl::ag::formatting;

    std::ostringstream  narrow_os;
    std::wostringstream wide_os;

    narrow_os << indented;
    wide_os << f::value;
    CHECK(wide_os.str() == widen(f::default_expected));

    narrow_os << f::value;
    CHECK(narrow_os.str() == f::indented_expected);
}

#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
TEMPLATE_TEST_CASE("typenamed - wchar_t" + name_suffix() + "", "[ag][wchar_t][typenamed]", WIDE_TEST_TYPES) {
    using f = fixture<TestType>;
    using namespace csl::ag::formatting;

    // The narrow->wide widening of type names is the chokepoint #389 is about:
    // every type-name source (csl::typeinfo, std::type_index::name(), user specializations) is narrow.
    CHECK(std::format(L"{:t}",   f::value) == widen(f::typenamed_expected));
    CHECK(fmt::format(L"{:t}",   f::value) == widen(f::typenamed_expected));
    CHECK(std::format(L"{:ixt}", f::value) == widen(f::indented_indexed_typenamed_expected));
    CHECK(fmt::format(L"{:ixt}", f::value) == widen(f::indented_indexed_typenamed_expected));

    std::wostringstream os;
    os << typenamed << f::value;
    CHECK(os.str() == widen(f::typenamed_expected));
}
#endif

#undef WIDE_TEST_TYPES
