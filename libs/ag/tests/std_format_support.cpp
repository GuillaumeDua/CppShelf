#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#include <csl/ag.hpp>
#include <csl/ag/formatting/format.hpp>
#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
#   include <csl/ag/formatting/typeinfo.hpp>
#endif
#include <tests/types.hpp>
#include <tests/ag/typeinfo_specializations.hpp>

#include <format>

namespace types = test::ag::types;

// csl::ag::io::details::concepts::std_formattable: see csl/ag/formatting/format.hpp.
namespace tests::concepts::std_formattable {

    namespace concepts = csl::ag::io::details::concepts;

    static_assert(concepts::std_formattable<types::field_1, char>);
    static_assert(concepts::std_formattable<types::field_2, char>);
    static_assert(concepts::std_formattable<types::field_3_nested, char>);
    // P2286-independent: this library's view machinery formats tuple-like/range elements itself,
    // so these hold even when the STL lacks std::formatter<tuple-like> (e.g. libstdc++ 13)
    static_assert(concepts::std_formattable<types::field_3_nested_tuplelike, char>);
    static_assert(concepts::std_formattable<types::field_4_nested_range, char>);
    static_assert(concepts::std_formattable<types::field_everything, char>);

    static_assert(concepts::std_formattable<int, char>);
    static_assert(concepts::std_formattable<std::string, char>);

    // wired into std::formatter<formatted_view_t<T>> (composite view):
    // a non-formattable field disables the specialization, so std::formattable answers false.
    // NOTE: the field must be a non-aggregate - an (even empty) aggregate is formattable
    //       through this library's own blanket std::formatter
    struct not_formattable_field { explicit not_formattable_field() = default; };
    struct with_not_formattable_field { not_formattable_field f; };
    static_assert(not concepts::std_formattable<with_not_formattable_field, char>);
    static_assert(not std::formattable<csl::ag::io::details::decorators::formatted_view_t<with_not_formattable_field>, char>);

    // recursion: the non-formattable leaf is reached through a nested aggregate
    struct nested_with_not_formattable_field { with_not_formattable_field w; int i; };
    static_assert(not concepts::std_formattable<nested_with_not_formattable_field, char>);
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

#define CSL_AG_TEST__HAS_TO_STRING 1
#include <tests/ag/format_test_cases.hpp>
