#pragma once
#include <csl/mp.hpp>
#include <cstdint>

// STL tuplelikes
#include <tuple>
#include <array>
#include <utility> // std::pair

#include <tests/mp/details/user_defined.hpp>

// NOLINTBEGIN(*-avoid-magic-numbers)

// concepts
namespace test::tuples::concepts::tuple_like {

    static_assert(csl::mp::concepts::tuple_like<std::array<int, 2>>);
    static_assert(csl::mp::concepts::tuple_like<std::pair<int, float>>);

    static_assert(csl::mp::concepts::tuple_like<std::tuple<>>);
    static_assert(csl::mp::concepts::tuple_like<std::tuple<int> &>);
    static_assert(csl::mp::concepts::tuple_like<std::tuple<int, int> &&>);
    static_assert(csl::mp::concepts::tuple_like<const std::tuple<int, float> &>);

    static_assert(csl::mp::concepts::tuple_like<csl::mp::tuple<>>);
    static_assert(csl::mp::concepts::tuple_like<csl::mp::tuple<int> &>);
    static_assert(csl::mp::concepts::tuple_like<csl::mp::tuple<int, int> &&>);
    static_assert(csl::mp::concepts::tuple_like<const csl::mp::tuple<int, float> &>);
} // namespace test::tuples::concepts::tuple_like
namespace test::tuples::concepts::pair_like {

    // std::array
    static_assert(not csl::mp::concepts::pair_like<std::array<int, 1>>);
    static_assert(csl::mp::concepts::pair_like<std::array<int, 2>>);
    static_assert(not csl::mp::concepts::pair_like<std::array<int, 3>>);

    // std::pair
    static_assert(csl::mp::concepts::pair_like<std::pair<int, float>>);

    // std::tuple
    static_assert(not csl::mp::concepts::pair_like<std::tuple<>>);
    static_assert(not csl::mp::concepts::pair_like<std::tuple<int>>);
    static_assert(csl::mp::concepts::pair_like<std::tuple<int, int>>);
    static_assert(csl::mp::concepts::pair_like<std::tuple<int, float>>);

    // tuple
    static_assert(not csl::mp::concepts::pair_like<csl::mp::tuple<>>);
    static_assert(not csl::mp::concepts::pair_like<csl::mp::tuple<int>>);
    static_assert(csl::mp::concepts::pair_like<csl::mp::tuple<int, int>>);
    static_assert(csl::mp::concepts::pair_like<csl::mp::tuple<int, float>>);
} // namespace test::tuples::concepts::pair_like
namespace test::concepts::std_array {
    static_assert(csl::mp::concepts::std_array<std::array<int, 1>>);
    static_assert(not csl::mp::concepts::std_array<char[1]>); // NOLINT(*-c-arrays)
    static_assert(not csl::mp::concepts::std_array<std::tuple<int>>);
    static_assert(not csl::mp::concepts::std_array<csl::mp::tuple<int>>);
} // namespace test::concepts::std_array
namespace test::tuples::concepts::sized {

    // empty
    static_assert(csl::mp::concepts::empty<csl::mp::tuple<>>);
    static_assert(csl::mp::concepts::empty<std::tuple<>>);
    static_assert(csl::mp::concepts::empty<std::array<int, 0>>);

    // not_empty
    static_assert(not csl::mp::concepts::empty<std::pair<int, char>>);
    static_assert(csl::mp::concepts::not_empty<std::pair<int, char>>);
    static_assert(not csl::mp::concepts::empty<std::tuple<int>>);
    static_assert(csl::mp::concepts::not_empty<std::tuple<int>>);

    // sized, at_least
    static_assert(csl::mp::concepts::sized<csl::mp::tuple<>, 0>);
    static_assert(csl::mp::concepts::sized<csl::mp::tuple<int>, 1>);
    static_assert(csl::mp::concepts::sized_at_least<csl::mp::tuple<int>, 1>);
    static_assert(not csl::mp::concepts::sized_at_least<csl::mp::tuple<int>, 2>);
    static_assert(csl::mp::concepts::sized<std::pair<int, char>, 2>);
} // namespace test::tuples::concepts::sized
namespace test::tuples::concepts::tuple_element {

    // int
    static_assert(not csl::mp::concepts::tuple_element<int, 0>);

    // std::array
    static_assert(csl::mp::concepts::tuple_element<std::array<int, 2>, 0>);
    static_assert(csl::mp::concepts::tuple_element<std::array<int, 2>, 1>);
    static_assert(not csl::mp::concepts::tuple_element<std::array<int, 2>, 2>);

    // std::pair
    static_assert(csl::mp::concepts::tuple_element<std::pair<int, float>, 0>);
    static_assert(not csl::mp::concepts::tuple_element<std::pair<int, float>, 2>);

    // tuple
    using T = csl::mp::tuple<int, float>;
    static_assert(csl::mp::concepts::tuple_element<T, 0>);
    static_assert(csl::mp::concepts::tuple_element<T, 1>);
    static_assert(not csl::mp::concepts::tuple_element<T, 2>);
} // namespace test::tuples::concepts::tuple_element

// tuples: details::concepts
namespace test::tuples::concepts::deductible {
    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates    = csl::mp::tuple<int, char, int>;

    // can_deduce_by_type
    static_assert(csl::mp::details::concepts::can_deduce_by_type<without_duplicates, int>);
    static_assert(not csl::mp::details::concepts::can_deduce_by_type<with_duplicates, int>);

    // can_deduce_by_index
    static_assert(csl::mp::details::concepts::can_deduce_by_index<without_duplicates, 0>);
    static_assert(csl::mp::details::concepts::can_deduce_by_index<with_duplicates, 0>);
} // namespace test::tuples::concepts::deductible

// tuples: API
namespace test::tuples::size {
    static_assert(0 == csl::mp::tuple<>::size);
    static_assert(1 == csl::mp::tuple<int>::size);
    static_assert(2 == csl::mp::tuple<int, char>::size);
    static_assert(3 == csl::mp::tuple<int, char, bool>::size);
    // duplicates
    static_assert(2 == csl::mp::tuple<int, int>::size);
} // namespace test::tuples::size
namespace test::tuples::size {
    static_assert(0 == csl::mp::size_v<csl::mp::tuple<>>);
    static_assert(1 == csl::mp::size_v<csl::mp::tuple<int>>);
    static_assert(2 == csl::mp::size_v<csl::mp::tuple<int, char>>);
    static_assert(3 == csl::mp::size_v<csl::mp::tuple<int, char, bool>>);
    // duplicates
    static_assert(2 == csl::mp::size_v<csl::mp::tuple<int, int>>);
} // namespace test::tuples::size
namespace test::tuples::empty {
    static_assert(csl::mp::type_traits::empty_v<csl::mp::tuple<>>);
    static_assert(not csl::mp::type_traits::empty_v<csl::mp::tuple<int>>);

    static_assert(csl::mp::concepts::empty<csl::mp::tuple<>>);
    static_assert(not csl::mp::concepts::empty<csl::mp::tuple<int>>);
} // namespace test::tuples::empty

namespace test::tuples::type_gettable {
    static_assert(csl::mp::type_traits::is_type_gettable_v<csl::mp::tuple<int>, int>);
    static_assert(csl::mp::type_traits::is_type_gettable_v<csl::mp::tuple<int, char>, int>);
    static_assert(csl::mp::type_traits::is_type_gettable_v<csl::mp::tuple<int, char>, char>);

    static_assert(csl::mp::type_traits::is_type_gettable_v<std::tuple<int>, int>);
    static_assert(csl::mp::type_traits::is_type_gettable_v<std::tuple<int, char>, int>);
    static_assert(csl::mp::type_traits::is_type_gettable_v<std::tuple<int, char>, char>);

    static_assert(not csl::mp::type_traits::is_type_gettable_v<csl::mp::tuple<int, int>, int>);
    static_assert(not csl::mp::type_traits::is_type_gettable_v<std::array<int, 1>, int>);
} // namespace test::tuples::type_gettable
namespace test::tuples::support_get_by_type {

    using without_duplicates = csl::mp::tuple<int, char, bool>;
    using with_duplicates    = csl::mp::tuple<int, char, int>;

    static_assert(not csl::mp::type_traits::support_get_by_type_v<with_duplicates>);
    static_assert(csl::mp::type_traits::support_get_by_type_v<without_duplicates>);
    static_assert(csl::mp::type_traits::support_get_by_type_v<csl::mp::tuple<int>>);
    static_assert(csl::mp::type_traits::support_get_by_type_v<csl::mp::tuple<int, char>>);
    static_assert(csl::mp::type_traits::support_get_by_type_v<csl::mp::tuple<int, char, bool>>);

    static_assert(csl::mp::type_traits::support_get_by_type_v<std::tuple<>>);
    static_assert(csl::mp::type_traits::support_get_by_type_v<csl::mp::tuple<>>);
    static_assert(not csl::mp::type_traits::support_get_by_type_v<std::array<int, 1>>);
    static_assert(not csl::mp::type_traits::support_get_by_type_v<std::tuple<int, int>>);
    static_assert(csl::mp::type_traits::support_get_by_type_v<std::tuple<int, char>>);
} // namespace test::tuples::support_get_by_type

namespace test::tuples::index_gettable {

    static_assert(csl::mp::concepts::index_gettable<csl::mp::tuple<int>, 0>);
    static_assert(not csl::mp::concepts::index_gettable<csl::mp::tuple<int>, 1>);

    // user_defined
    static_assert(csl::mp::type_traits::is_index_gettable_v<test::tuples::user_defined::well_formed, 0>);
    static_assert(csl::mp::type_traits::is_index_gettable_v<test::tuples::user_defined::well_formed, 1>);
    static_assert(csl::mp::type_traits::is_index_gettable_v<test::tuples::user_defined::well_formed, 2>);
    static_assert(not csl::mp::type_traits::is_index_gettable_v<test::tuples::user_defined::well_formed, 3>); // out of bounds

    static_assert(not csl::mp::type_traits::is_index_gettable_v<test::tuples::user_defined::ill_formed, 0>);
} // namespace test::tuples::index_gettable
namespace test::tuples::support_get_by_index {

    static_assert(csl::mp::type_traits::support_get_by_index_v<csl::mp::tuple<>>);
    static_assert(csl::mp::type_traits::support_get_by_index_v<csl::mp::tuple<int>>);
    static_assert(csl::mp::type_traits::support_get_by_index_v<std::tuple<>>);
    static_assert(csl::mp::type_traits::support_get_by_index_v<std::tuple<int>>);
    static_assert(csl::mp::type_traits::support_get_by_index_v<std::array<int, 2>>);
    static_assert(csl::mp::type_traits::support_get_by_index_v<std::pair<int, char>>);

    // user_defined
    static_assert(csl::mp::type_traits::support_get_by_index_v<test::tuples::user_defined::well_formed>);
    static_assert(csl::mp::concepts::support_get_by_index<test::tuples::user_defined::well_formed>);
    static_assert(not csl::mp::concepts::support_get_by_index<test::tuples::user_defined::ill_formed>);
} // namespace test::tuples::support_get_by_index

namespace test::tuples::compare::tuple {

    template <
        csl::mp::concepts::tuple_like lhs_t,
        csl::mp::concepts::tuple_like rhs_t>
    struct impl {
        constexpr static void common_reference() {
            static_assert(requires { std::common_reference_t<lhs_t>{}; });
            static_assert(requires { std::common_reference_t<rhs_t>{}; });

            static_assert(std::same_as<
                csl::mp::tuple_common_reference_t<
                    lhs_t,
                    rhs_t,
                    std::type_identity_t,
                    std::type_identity_t
                >,
                csl::mp::tuple<
                    std::common_reference_t<
                        std::tuple_element_t<0, lhs_t>,
                        std::tuple_element_t<0, rhs_t>
                    >,
                    std::common_reference_t<
                        std::tuple_element_t<1, lhs_t>,
                        std::tuple_element_t<1, rhs_t>
                    >
                >
            >);

            static_assert(
                requires {
                    typename csl::mp::tuple_common_reference_t<
                        lhs_t,
                        rhs_t,
                        std::type_identity_t,
                        std::type_identity_t>;
                }
            );

            static_assert(std::common_reference_with<lhs_t, rhs_t>);
        }

        constexpr static void equality() {
            static_assert(std::equality_comparable<lhs_t>);
            static_assert(std::equality_comparable<rhs_t>);
            static_assert(requires {
                lhs_t{} == rhs_t{};
            });

            static_assert(lhs_t{} == lhs_t{});
            static_assert(lhs_t{{}, {}} == lhs_t{{}, {}});
            static_assert(lhs_t{{}, 'a'} != lhs_t{1.F, 'a'});
            static_assert(lhs_t{42, {}} != lhs_t{42.F, 'a'});

            // NOTE: C++23
            static_assert(std::equality_comparable_with<lhs_t, rhs_t>);

#if CSL_MP_TUPLE__IMPLICIT_CONVERSION
            [[maybe_unused]] constexpr static auto narrowing = csl::mp::tuple<char>{42};
            static_assert(lhs_t{42, 'a'} == lhs_t{42, 'a'});
            static_assert(lhs_t{{}, 'a'} != lhs_t{42, 'a'});
            static_assert(lhs_t{42, {}} != lhs_t{42, 'a'});
#endif
        }

        constexpr static void three_way() {
            static_assert(std::three_way_comparable<lhs_t>);
            static_assert(std::three_way_comparable<rhs_t>);
            static_assert(requires {
                lhs_t{} <=> rhs_t{};
            });
            static_assert(std::three_way_comparable_with<lhs_t, rhs_t>);
            static_assert(lhs_t{0.F, {}} < rhs_t{1.F, {}});
            static_assert(lhs_t{0.F, {}} < rhs_t{0.F, 1});
        }
    };

    using lhs_t = csl::mp::tuple<float, char>;
    using rhs_t = csl::mp::tuple<double, int>;

    using symetrical_ok = impl<lhs_t, lhs_t>;
    // #if defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION) and CSL_MP_TUPLE__IMPLICIT_CONVERSION
    using asymetrical_ok = impl<lhs_t, lhs_t>;
    // #endif
} // namespace test::tuples::compare::tuple
// QUESTION: #if CSL_MP_TUPLE__IMPLICIT_CONVERSION
namespace test::tuples::compare::tuplelikes {

    static_assert(std::tuple{'a', 42} == std::array<int, 2>{'a', 42}); // WARNING: implicit conversion
    static_assert(std::tuple{'a', 42} == std::pair{'a', 42});

    // TODO(GUILLAUME) issue #285
    // static_assert(csl::mp::tuple{ 'a', 42 } == std::tuple{'a', 42});
    // static_assert(csl::mp::tuple{ 'a', 42 } == std::array<int, 2>{'a', 42});
    // static_assert(csl::mp::tuple{ 'a', 42 } == std::pair{ 'a', 42 });

    // and the other way around
} // namespace test::tuples::compare::tuplelikes
namespace test::tuples::fwd_as_tuple {

    constexpr auto c = 'a';
    static_assert(std::is_same_v<
        csl::mp::tuple<int &&, const char &>,
        decltype(csl::mp::functions::forward_as_tuple(42, c))
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<csl::mp::tuple<int> &&>,
        decltype(csl::mp::functions::forward_as_tuple(csl::mp::tuple{42}))
    >);
} // namespace test::tuples::fwd_as_tuple
namespace test::tuples::cat {

    // empty
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::type_traits::cat_result_t<>
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::type_traits::cat_result_t<csl::mp::tuple<>>
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<>,
        csl::mp::type_traits::cat_result_t<
            csl::mp::tuple<>,
            csl::mp::tuple<>
        >
    >);

    // type
    static_assert(std::is_same_v<
        csl::mp::tuple<int, char, double, float>,
        csl::mp::type_traits::cat_result_t<
            csl::mp::tuple<int, char>,
            csl::mp::tuple<double, float>
        >
    >);
    static_assert(std::is_same_v<
        csl::mp::tuple<int, char, double, float>,
        csl::mp::type_traits::cat_result_t<
            csl::mp::tuple<>,
            csl::mp::tuple<int>,
            csl::mp::tuple<>,
            csl::mp::tuple<char>,
            csl::mp::tuple<double, float>,
            csl::mp::tuple<>
        >
    >);

    // value
    static_assert(
            csl::mp::tuple<char, int>{'a', 42}
        ==  csl::mp::functions::cat(
                csl::mp::tuple<char>{'a'},
                csl::mp::tuple<int>{42}
            )
    );
} // namespace test::tuples::cat
namespace test::tuples::indexes {

    using with_duplicates = csl::mp::tuple<char, double, float, int, int>;
    static_assert(3 == csl::mp::type_traits::index_of_v<with_duplicates, int>);
    static_assert(4 == csl::mp::type_traits::last_index_of_v<with_duplicates, int>);

    using uniqued = csl::mp::tuple<char, double, float, int>;
    static_assert(3 == csl::mp::type_traits::index_of_v<uniqued, int>);
    static_assert(3 == csl::mp::type_traits::last_index_of_v<uniqued, int>);
} // namespace test::tuples::indexes
// construction
namespace test::tuples::storage::constructors::default_ {
    using type                            = csl::mp::tuple<int, char>;
    [[maybe_unused]] constexpr auto value = type{};
} // namespace test::tuples::storage::constructors::default_
namespace test::tuples::storage::constructors::value::no_cvref {

    static_assert(std::constructible_from<
        std::tuple<int, char>,
        int,
        char
    >);
    static_assert(std::constructible_from<
        std::tuple<int, char>,
        int &,
        char &
    >);
    static_assert(std::constructible_from<
        std::tuple<int, char>,
        const int &,
        const char &
    >);
    static_assert(std::constructible_from<
        std::tuple<int, char>,
        int &&,
        char &&
    >);
    [[maybe_unused]] constexpr auto v1 = csl::mp::tuple<int, char>{ 42, 'a' };
} // namespace test::tuples::storage::constructors::value::no_cvref
namespace test::tuples::storage::constructors::value::lvalue_ref {

    static_assert(not std::constructible_from<
        std::tuple<int &, char &>,
        int,
        char
    >);
    static_assert(std::constructible_from<
        std::tuple<int &, char &>,
        int &,
        char &
    >);
    static_assert(not std::constructible_from<
        std::tuple<int &, char &>,
        const int &,
        const char &
    >);
    static_assert(not std::constructible_from<
        std::tuple<int &, char &>,
        int &&,
        char &&
    >);
} // namespace test::tuples::storage::constructors::value::lvalue_ref
namespace test::tuples::storage::constructors::value::const_lvalue_ref {

    static_assert(std::constructible_from<
        std::tuple<const int &, const char &>,
        int,
        char
    >);
    static_assert(std::constructible_from<
        std::tuple<const int &, const char &>,
        int &,
        char &
    >);
    static_assert(std::constructible_from<
        std::tuple<const int &, const char &>,
        const int &,
        const char &
    >);
    static_assert(not std::constructible_from<
        std::tuple<int &, const char &>,
        int &&,
        char &&
    >);
} // namespace test::tuples::storage::constructors::value::const_lvalue_ref
namespace test::tuples::storage::constructors::value::rvalue_ref {

    static_assert(std::constructible_from<
        std::tuple<int &&, char &&>,
        int,
        char
    >);
    static_assert(not std::constructible_from<
        std::tuple<int &&, char &&>,
        int &,
        char &
    >);
    static_assert(not std::constructible_from<
        std::tuple<int &&, char &&>,
        const int &,
        const char &
    >);
    static_assert(std::constructible_from<
        std::tuple<int &&, char &&>,
        int &&,
        char &&
    >);
} // namespace test::tuples::storage::constructors::value::rvalue_ref
namespace test::tuples::storage::constructors::copy {
    using type                            = csl::mp::tuple<int, char>;
    constexpr auto                  value = type{42, 'a'};
    [[maybe_unused]] constexpr auto copy  = value;
    static_assert(value == copy);
} // namespace test::tuples::storage::constructors::copy

#include <string_view>
namespace test::tuples::storage::constructors::move {
    using type                               = csl::mp::tuple<int, char, std::string_view>;
    [[maybe_unused]] constexpr auto moved_to = [] {
        auto tmp   = type{42, 'a', std::string_view{}};
        auto value = std::move(tmp); // NOLINT(performance-move-const-arg)
        return value;
    }();
} // namespace test::tuples::storage::constructors::move
namespace test::tuples::storage::constructors::conversion::safe {

#if CSL_MP_TUPLE__IMPLICIT_CONVERSION >= CSL_MP_TUPLE__IMPLICIT_CONVERSION_SAFE
    [[maybe_unused]] constexpr csl::mp::tuple<std::int32_t>               d = csl::mp::tuple<std::int8_t>{};
    [[maybe_unused]] constexpr csl::mp::tuple<std::int32_t, std::int64_t> e = csl::mp::tuple<std::int8_t, std::int8_t>{};
    [[maybe_unused]] constexpr csl::mp::tuple<double>                     f{float{}};
#endif
} // namespace test::tuples::storage::constructors::conversion::safe
namespace test::tuples::storage::constructors::conversion::unsafe {
#if CSL_MP_TUPLE__IMPLICIT_CONVERSION == CSL_MP_TUPLE__IMPLICIT_CONVERSION_UNSAFE
    [[maybe_unused]] constexpr csl::mp::tuple<int, char> a = csl::mp::tuple<double, int>{.0, 0};
    [[maybe_unused]] constexpr csl::mp::tuple<int>       b{.0F};
    [[maybe_unused]] constexpr csl::mp::tuple<float>     c{.0};
#endif
} // namespace test::tuples::storage::constructors::conversion::unsafe
namespace test::tuples::storage::of_refs {
    using type = csl::mp::tuple<int &&, const char &>;

    [[maybe_unused]] static void impl() {
        const auto            c     = 'a';
        auto                  i     = 42;
        [[maybe_unused]] auto value = type{std::move(i), c}; // NOLINT(performance-move-*)
    }
} // namespace test::tuples::storage::of_refs

namespace test::tuples::deduction_guide {
    static_assert(std::same_as<
        csl::mp::tuple<>,
        decltype(csl::mp::tuple{})>
    );
    static_assert(std::same_as<
        csl::mp::tuple<int, char>,
        decltype(csl::mp::tuple{42, 'a'})>
    );
} // namespace test::tuples::deduction_guide
namespace test::tuples::get {
    using type           = csl::mp::tuple<int, char>;
    constexpr auto value = type{42, 'a'};

    static_assert(42  == value.template get<0>());
    static_assert('a' == value.template get<1>());
} // namespace test::tuples::get
namespace test::tuples::get::cvref {

    using tuple_element_t = csl::mp::element_t<0, type>;

    using expecting_lvalue = decltype(std::declval<type &>().get<0>());
    static_assert(std::same_as<tuple_element_t &, expecting_lvalue>);

    using expecting_rvalue = decltype(std::declval<type &&>().get<0>());
    static_assert(std::same_as<tuple_element_t &&, expecting_rvalue>);

    using expecting_const_lvalue = decltype(std::declval<const type &>().get<0>());
    static_assert(std::same_as<const tuple_element_t &, expecting_const_lvalue>);

    using expecting_const_rvalue = decltype(std::declval<const type &&>().get<0>());
    static_assert(std::same_as<const tuple_element_t &&, expecting_const_rvalue>);
} // namespace test::tuples::get::cvref
namespace test::tuples::get::cvref::consistency {

    static_assert(std::same_as<
        decltype(std::declval<type &>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<type &>()))
    >);
    static_assert(std::same_as<
        decltype(std::declval<type &&>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<type &&>()))
    >);
    static_assert(std::same_as<
        decltype(std::declval<const type &>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<const type &>()))
    >);
    static_assert(std::same_as<
        decltype(std::declval<const type &&>().get<0>()),
        decltype(csl::mp::get<0>(std::declval<const type &&>()))
    >);
} // namespace test::tuples::get::cvref::consistency

// std::tuple interface/inter-operatiblity
namespace test::tuples::std_interopterability::tuple_size {
    using valid_tuple   = csl::mp::tuple<int, char>;
    using invalid_tuple = csl::mp::tuple<int, char, int>;

    static_assert(std::tuple_size_v<valid_tuple> == 2);
    static_assert(std::tuple_size_v<invalid_tuple> == 3);
} // namespace test::tuples::std_interopterability::tuple_size
namespace test::tuples::std_interopterability::tuple_element {
    using valid_tuple = csl::mp::tuple<int, char>;
    static_assert(std::is_same_v<int, std::tuple_element_t<0, valid_tuple>>);
    static_assert(std::is_same_v<char, std::tuple_element_t<1, valid_tuple>>);

    using invalid_tuple = csl::mp::tuple<int, char, int>;
    static_assert(std::is_same_v<int, std::tuple_element_t<0, invalid_tuple>>);
    static_assert(std::is_same_v<char, std::tuple_element_t<1, invalid_tuple>>);
    static_assert(std::is_same_v<int, std::tuple_element_t<0, invalid_tuple>>);
} // namespace test::tuples::std_interopterability::tuple_element
namespace test::tuples::std_interopterability::get {

    using std::get; // NOTE: std::get is not a customization point

    using type           = csl::mp::tuple<int, char>;
    constexpr auto value = type{ 42, 'a' };

    static_assert(42  == get<0>(value));
    static_assert('a' == get<1>(value));

    static_assert(get<0>(value) == get<int>(value));
    static_assert(get<1>(value) == get<char>(value));
} // namespace test::tuples::std_interopterability::get

// ADL
namespace test::tuples::get::ADL {

    using csl::mp::get;

    using type           = csl::mp::tuple<int, char>;
    constexpr auto value = type{42, 'a'};

    static_assert(42 == get<0>(value));
    static_assert('a' == get<1>(value));
} // namespace test::tuples::get::ADL

namespace test::tuples::get::square_bracket_op {

    using type           = csl::mp::tuple<int, char>;
    constexpr auto value = type{42, 'a'};

    static_assert(42 == value[csl::mp::index_t<0>{}]);
    static_assert(42 == value[csl::mp::index<0>]);

    namespace literals {
        using namespace csl::mp::literals;
        static_assert(42 == value[0_index]);
    } // namespace literals
} // namespace test::tuples::get::square_bracket_op

// structured binding
namespace test::tuples::structured_binding {

    using type = csl::mp::tuple<int, char>;

    constexpr void lvalue() {
        auto value = type{ 42, 'a' };
        [[maybe_unused]] auto & [i, c] = value; // NOLINT(*-qualified-auto)

        static_assert(std::is_same_v<decltype(i), int &>);
        static_assert(std::is_same_v<decltype(c), char &>);

        // static_assert(i == get<0>(value));
        // static_assert(c == get<1>(value));
    }
    constexpr void const_lvalue() {
        constexpr auto value                 = type{ 42, 'a' };
        [[maybe_unused]] const auto & [i, c] = value;

        static_assert(std::is_same_v<decltype(i), int const &>);
        static_assert(std::is_same_v<decltype(c), char const &>);

        static_assert(i == csl::mp::get<0>(value));
        static_assert(c == csl::mp::get<1>(value));
    }
    constexpr void rvalue() {
        [[maybe_unused]] auto && [i, c] = type{ 42, 'a' };

        static_assert(i == csl::mp::get<0>(value));
        static_assert(c == csl::mp::get<1>(value));
    }
    constexpr void const_rvalue() {
        [[maybe_unused]] const auto && [i, c] = type{ 42, 'a' };

        static_assert(i == csl::mp::get<0>(value));
        static_assert(c == csl::mp::get<1>(value));
    }
} // namespace test::tuples::structured_binding

// NOLINTEND(*-avoid-magic-numbers)
