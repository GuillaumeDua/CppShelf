#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#include <csl/ag.hpp>
#include <csl/ag/formatting/backend/std_format.hpp>
#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
#   include <csl/ag/formatting/typeinfo.hpp>
#endif
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>
#include <tests/ag/format_optins_std.hpp>

#include <format>

namespace types = test::ag::types;

// csl::ag::formatting::details::concepts::std_formattable: see csl/ag/formatting/backend/std_format.hpp.
namespace tests::concepts::std_formattable {

    namespace concepts = csl::ag::formatting::details::concepts;

    static_assert(concepts::std_formattable<types::field_1, char>);
    static_assert(concepts::std_formattable<types::field_2, char>);
    static_assert(concepts::std_formattable<types::field_3_nested, char>);
    // NOTE(P2286-independent) this librarys formats tuple-like/range elements itself,
    // so these hold even when the STL lacks std::formatter<tuple-like> (e.g. libstdc++-13)
    static_assert(concepts::std_formattable<types::field_3_nested_tuplelike, char>);
    static_assert(concepts::std_formattable<types::field_4_nested_range, char>);
    static_assert(concepts::std_formattable<types::field_everything, char>);

    static_assert(concepts::std_formattable<int, char>);
    static_assert(concepts::std_formattable<std::string, char>);

    // empty types are valid aggregates
    struct empty{};
    struct nested_empty{ empty e; int i; };
    static_assert(concepts::std_formattable<empty, char>);
    static_assert(concepts::std_formattable<nested_empty, char>);
    static_assert(std::formattable<csl::ag::formatting::details::decorators::formatted_view_t<empty>, char>);
    static_assert(std::formattable<csl::ag::formatting::details::decorators::formatted_view_t<nested_empty>, char>);

    // formattability of the fields is independent from opting the type in:
    // not_opted_in has no std::formatter, yet its field is std-formattable (hence its views are).
    static_assert(concepts::std_formattable<types::not_opted_in, char>);
    static_assert(not std::formattable<types::not_opted_in, char>);
    static_assert(std::formattable<csl::ag::formatting::details::decorators::formatted_view_t<types::not_opted_in>, char>);

    // non-aggregates types are not supported
    struct not_formattable { explicit not_formattable() = default; };
    struct nested_not_formattable { not_formattable f; };
    static_assert(not concepts::std_formattable<not_formattable, char>);
    static_assert(not concepts::std_formattable<nested_not_formattable, char>);
    static_assert(not std::formattable<csl::ag::formatting::details::decorators::formatted_view_t<not_formattable>, char>);
    static_assert(not std::formattable<csl::ag::formatting::details::decorators::formatted_view_t<nested_not_formattable>, char>);
}

namespace {
#include <tests/ag/format_fixtures.hpp>

    namespace implementation {
        template <typename T, typename Char = char>
        using formatter = std::formatter<T, Char>;

        constexpr std::string_view name = "std";

        template <typename T>
        auto format(std::string_view fmt_str, const T & value) {
            return std::vformat(fmt_str, std::make_format_args(value));
        }
    }
} // namespace

#include <tests/ag/format_test_cases.hpp>
