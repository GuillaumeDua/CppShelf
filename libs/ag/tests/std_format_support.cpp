#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#define CSL_AG__ENABLE_STD_FORMAT_SUPPORT true
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG_BITFIELDS_STR "ON"
#else
#  define CSL_AG_BITFIELDS_STR "OFF"
#endif
#define CSL_AG_FORMATTING_STR "CSL_AG__ENABLE_STD_FORMAT_SUPPORT"

#include <csl/ag.hpp>
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>

#include <format>

namespace types = test::ag::types;

// csl::ag::io::details::concepts::std_formattable: see csl/ag.hpp.
//
// NOTE: std::formatter<tuple-like> is standardized in C++23 (P2286),
// but since some compiler's libstdc++ doesn't implement it yet,
// coverage cannot mirror tests::concepts::fmt_formattable (fmtlib_support.cpp) one-for-one here.
namespace tests::concepts::std_formattable {

    namespace concepts = csl::ag::io::details::concepts;

    static_assert(concepts::std_formattable<types::field_1, char>);
    static_assert(concepts::std_formattable<types::field_2, char>);
    static_assert(concepts::std_formattable<types::field_3_nested, char>);

    static_assert(concepts::std_formattable<int, char>);
    static_assert(concepts::std_formattable<std::string, char>);
}

namespace {
#include <tests/ag/format_fixtures.hpp>

    namespace under_test {
        template <typename T, typename Char = char>
        using formatter = std::formatter<T, Char>;

        constexpr std::string_view tag = "format";

        template <typename T>
        auto format(std::string_view fmt_str, T const & value) {
            return std::vformat(fmt_str, std::make_format_args(value));
        }
    }
} // namespace

#include <tests/ag/format_test_cases.hpp>

#undef CSL_AG_FORMATTING_STR
#undef CSL_AG_BITFIELDS_STR
