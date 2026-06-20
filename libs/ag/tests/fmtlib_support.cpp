#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#define CSL_AG__ENABLE_FMTLIB_SUPPORT true

#include <csl/ag.hpp>
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>

// TODO: check possible clash with user-defined formatters -> complete, partial/generics, etc.

namespace types = test::ag::types;

namespace tests::concepts::fmt_formattable {

    namespace concepts = csl::ag::io::details::concepts;

    static_assert(concepts::fmt_formattable<types::field_1, char>);
    static_assert(concepts::fmt_formattable<types::field_2, char>);
    static_assert(concepts::fmt_formattable<types::field_3_nested, char>);
    static_assert(concepts::fmt_formattable<types::field_3_nested_tuplelike, char>);
    static_assert(concepts::fmt_formattable<types::field_4_nested_range, char>);
    static_assert(concepts::fmt_formattable<types::field_everything, char>);

    static_assert(concepts::fmt_formattable<int, char>);
    static_assert(concepts::fmt_formattable<std::string, char>);
    static_assert(concepts::fmt_formattable<std::vector<int>, char>);
    static_assert(concepts::fmt_formattable<std::vector<types::field_1>, char>);
    static_assert(concepts::fmt_formattable<std::tuple<int>, char>);
    static_assert(concepts::fmt_formattable<std::array<int, 3>, char>);
}

namespace {
#include <tests/ag/format_fixtures.hpp>

    namespace implementation {
        template <typename T, typename Char = char>
        using formatter = fmt::formatter<T, Char>;

        constexpr std::string_view name = "fmt";

        template <typename T>
        auto format(std::string_view fmt_str, T const & value) {
            return fmt::vformat(fmt_str, fmt::make_format_args(value));
        }
    }
} // namespace

#include <tests/ag/format_test_cases.hpp>
