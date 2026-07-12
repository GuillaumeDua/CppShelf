#if not defined(FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT)
#  error "FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT is not set"
#endif
#undef  CSL_AG__ENABLE_BITFIELDS_SUPPORT
#if FORCE_CSL_AG__ENABLE_BITFIELDS_SUPPORT
#  define CSL_AG__ENABLE_BITFIELDS_SUPPORT true
#endif

#include <csl/ag.hpp>
#include <csl/ag/formatting/fmt.hpp>
#if defined(CSL_AG_TEST__WITH_TYPEINFO) and CSL_AG_TEST__WITH_TYPEINFO
#   include <csl/ag/formatting/typeinfo.hpp>
#endif
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

    // wired into fmt::formatter<formatted_view_t<T>> (composite view):
    // a non-formattable field disables the specialization, so fmt::is_formattable answers false.
    // NOTE: the field must be a non-aggregate - an (even empty) aggregate is formattable
    //       through this library's own blanket fmt::formatter
    struct not_formattable_field { explicit not_formattable_field() = default; };
    struct with_not_formattable_field { not_formattable_field f; };
    static_assert(not concepts::fmt_formattable<with_not_formattable_field, char>);
    static_assert(not fmt::is_formattable<csl::ag::io::details::decorators::formatted_view_t<with_not_formattable_field>, char>::value);
}

namespace {
#include <tests/ag/format_fixtures.hpp>

    namespace implementation {
        template <typename T, typename Char = char>
        using formatter = fmt::formatter<T, Char>;

        constexpr std::string_view name = "fmt";

        template <typename T>
        auto format(std::string_view fmt_str, const T & value) {
            return fmt::vformat(fmt_str, fmt::make_format_args(value));
        }
    }
} // namespace

#include <tests/ag/format_test_cases.hpp>
