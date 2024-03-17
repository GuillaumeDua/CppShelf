#include <csl/ensure.hpp>

#include <type_traits>
#include <array>
#include <cassert>
#include <string>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)

namespace test::utils::type_traits {

// has_type
#if __cplusplus >= 202002L
    template <typename T>
    concept has_type_v = requires { typename T::type; };
#else
    template<class, class = void>
    struct has_type : std::false_type {};
    template <class T>
    struct has_type<T, std::void_t<typename T::type>> : std::true_type {};
    template <typename T>
    constexpr inline static bool has_type_v = has_type<T>::value;
#endif

// supports_op_plus_with
#if __cplusplus >= 202002L
    template <typename T, typename U>
    concept supports_op_plus_with_v = requires { std::declval<T &>() + std::declval<const U &>(); };
#else
    template <class T, class U>
    constexpr inline static bool supports_op_plus_with_v = csl::ensure::details::mp::type_traits::arythmetic::supports_op_plus_with<T,U>::value;
#endif
}

namespace test::strong_type::details::comparison {
    struct eq_comparable_t{ bool operator==(const eq_comparable_t&) const { return {}; } };
    struct not_eq_comparable_t{ bool operator not_eq(const not_eq_comparable_t&) const { return {}; } };
    struct less_than_comparable { bool operator< (const less_than_comparable & ) const { return {}; } };
    struct less_or_eq_comparable{ bool operator<=(const less_or_eq_comparable &) const { return {}; } };
    struct greater_than_comparable { bool operator> (const greater_than_comparable & ) const { return {}; } };
    struct greater_or_eq_comparable{ bool operator>=(const greater_or_eq_comparable &) const { return {}; } };
}

namespace test::strong_type::details { // NOLINT(*-concat-nested-namespaces)
#if __cplusplus >= 202002L
// C++20: concepts
namespace comparison {
    namespace cs = csl::ensure::details::concepts::comparison;
}
namespace comparison::equality {
    // operator==
    static_assert(cs::equality_with<eq_comparable_t, eq_comparable_t>);
    static_assert(not cs::equality_with<eq_comparable_t, not_eq_comparable_t>);
    static_assert(not cs::equality_with<less_or_eq_comparable, less_or_eq_comparable>);
    // operator not_eq
    static_assert(cs::not_equality_with<eq_comparable_t, eq_comparable_t>);
    static_assert(cs::not_equality_with<not_eq_comparable_t, not_eq_comparable_t>);
    static_assert(not cs::not_equality_with<not_eq_comparable_t, int>);
}
namespace comparison::less {
    // operator<
    static_assert(cs::less_than_comparable_with<less_than_comparable, less_than_comparable>);
    static_assert(not cs::less_than_comparable_with<less_or_eq_comparable, less_or_eq_comparable>);
    static_assert(not cs::less_than_comparable_with<less_than_comparable, int>);
    // operator<=
    static_assert(cs::less_than_or_equal_to_comparable_with<less_or_eq_comparable, less_or_eq_comparable>);
    static_assert(not cs::less_than_or_equal_to_comparable_with<less_than_comparable, less_than_comparable>);
    static_assert(not cs::less_than_or_equal_to_comparable_with<less_than_comparable, int>);
}
namespace comparison::greater {
    // operator<
    static_assert(cs::greater_than_comparable_with<greater_than_comparable, greater_than_comparable>);
    static_assert(not cs::greater_than_comparable_with<greater_than_comparable, greater_or_eq_comparable>);
    static_assert(not cs::greater_than_comparable_with<greater_than_comparable, int>);
    // operator<=
    static_assert(cs::greater_than_or_equal_comparable_with<greater_or_eq_comparable, greater_or_eq_comparable>);
    static_assert(not cs::greater_than_or_equal_comparable_with<greater_or_eq_comparable, greater_than_comparable>);
    static_assert(not cs::greater_than_or_equal_comparable_with<greater_or_eq_comparable, int>);
}
namespace arythmetic {
}
#else
// C++17: type_traits
namespace comparison {
namespace tt = csl::ensure::details::mp::type_traits::comparison;
}
namespace comparison::equality {
    // operator==
    static_assert(tt::is_equality_comparable_v<eq_comparable_t>);
    static_assert(not tt::is_equality_comparable_v<not_eq_comparable_t>);
    // operator not_eq
    static_assert(tt::is_not_equality_comparable_v<not_eq_comparable_t>);
    static_assert(not tt::is_not_equality_comparable_v<eq_comparable_t>);
}
namespace comparison::less {
    // operator<
    static_assert(tt::is_less_than_comparable_v<less_than_comparable>);
    static_assert(not tt::is_less_than_comparable_v<less_or_eq_comparable>);
    // operator<=
    static_assert(tt::is_less_equal_comparable_v<less_or_eq_comparable>);
    static_assert(not tt::is_less_equal_comparable_v<less_than_comparable>);
}
namespace comparison::greater {
    // operator>
    static_assert(tt::is_greater_than_comparable_v<greater_than_comparable>);
    static_assert(not tt::is_greater_than_comparable_v<greater_or_eq_comparable>);
    // operator>=
    static_assert(tt::is_greater_equal_comparable_v<greater_or_eq_comparable>);
    static_assert(not tt::is_greater_equal_comparable_v<greater_than_comparable>);
}
namespace arythmetic {
    namespace tt = csl::ensure::details::mp::type_traits::arythmetic;
    // TODO(Guss)
}
#endif
}

namespace test::strong_type::type_traits {

    namespace tt = csl::ensure::type_traits;
    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    // is_strong_type
    static_assert(    tt::is_strong_type_v<meters>);
    static_assert(not tt::is_strong_type_v<int>);

    // is_strong_type_of
    static_assert(    tt::is_strong_type_of_v<meters, int>);
    static_assert(not tt::is_strong_type_of_v<meters, char>);
    static_assert(not tt::is_strong_type_of_v<int, int>);

    // is_tagged_by
    static_assert(    tt::is_tagged_by_v<meters, struct meters_tag>);
    static_assert(not tt::is_tagged_by_v<meters, char>);
    static_assert(not tt::is_tagged_by_v<int, int>);

    // underlying_type
    static_assert(    std::is_same_v<int,  tt::underlying_type_t<meters>>);
    static_assert(not std::is_same_v<char, tt::underlying_type_t<meters>>);
    static_assert(not utils::type_traits::has_type_v<tt::tag_type<meters_tag>>);

    // tag_type
    static_assert(    std::is_same_v<meters_tag, tt::tag_type_t<meters>>);
    static_assert(not std::is_same_v<char,       tt::tag_type_t<meters>>);
    static_assert(not utils::type_traits::has_type_v<tt::underlying_type<int>>);
}
#if __cplusplus >= 202002L
namespace test::strong_type::concepts {
    namespace c = csl::ensure::concepts;
    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    static_assert(c::StrongType<meters>);
    static_assert(c::NotStrongType<int>);
    static_assert(c::StrongTypeOf<meters, int>);
    static_assert(c::TaggedBy<meters, meters_tag>);
}
#endif
namespace test::strong_type::construction {
    using String = csl::ensure::strong_type<std::string, struct string_tag>;
    static_assert(std::is_default_constructible_v<String>);
    static_assert(std::is_constructible_v<String>);
    static_assert(std::is_constructible_v<String, std::string&&>);
    static_assert(std::is_constructible_v<String, const std::string&>);
    static_assert(std::is_constructible_v<String, const char*>);
    static_assert(std::is_constructible_v<String, std::size_t, std::string::value_type>);
}
namespace test::strong_type::comparisons::eq_not_eq {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    static_assert(42 == meters{ 42 });                  // NOLINT
    static_assert(0 not_eq meters{ 42 });               // NOLINT
    static_assert(meters{ 42 } == 42);                  // NOLINT
    static_assert(meters{ 42 } == meters{ 42 });    // NOLINT 

#if __cplusplus >= 202002L
    static_assert(std::three_way_comparable_with<meters, meters>);
    static_assert(std::three_way_comparable_with<meters, const int>);
    static_assert(std::three_way_comparable_with<int,    const meters>);
#endif

    using name = csl::ensure::strong_type<std::string_view, struct name_tag>;
    static_assert(name{"toto"} == std::string_view{"toto"});
    static_assert(name{"toto"} not_eq std::string_view{"xxxx"});

    // asymetrical comparable
    struct eq_only {
        constexpr eq_only() = default;
        constexpr bool operator==(const eq_only &) const noexcept { return true; }
        constexpr bool operator not_eq(const eq_only &) = delete;
    };
    using strong_eq_only = csl::ensure::strong_type<eq_only, struct strong_eq_only_tag>;
    static_assert(strong_eq_only{} == strong_eq_only{});

    struct not_eq_only {
        constexpr not_eq_only() = default;
        constexpr bool operator not_eq(const not_eq_only &) const noexcept { return true; }
        constexpr bool operator ==(const not_eq_only &) = delete;
    };
    using strong_not_eq_only = csl::ensure::strong_type<not_eq_only, struct strong_not_eq_only_tag>;
    static_assert(strong_not_eq_only{} not_eq strong_not_eq_only{});
}
namespace test::strong_type::comparisons::lt_eq_gt {
    struct less_comparable{
        constexpr bool operator<(int) const {
            return true;
        }
    };
    using strong_less_comparable = csl::ensure::strong_type<less_comparable, struct less_comparable_tag>;
    // static_assert(strong_less_comparable{} < 42); // TODO(Guss)
}
namespace test::strong_type::assign {
    using name = csl::ensure::strong_type<std::string, struct name_tag>;
    static_assert(std::is_assignable_v<name&, name&&>);
    static_assert(std::is_assignable_v<name&, const name&>);
    static_assert(std::is_assignable_v<name&, const std::string&>);
    static_assert(std::is_assignable_v<name&, std::string &&>);
}
namespace test::strong_type::arithmetic {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    // using underlying
    static_assert(utils::type_traits::supports_op_plus_with_v<meters, meters>);
    static_assert(utils::type_traits::supports_op_plus_with_v<meters, int>);
    static_assert(utils::type_traits::supports_op_plus_with_v<int,    meters>);

    // using op forwarding
    using name = csl::ensure::strong_type<std::string, struct name_tag>;
    // WIP
    namespace tt = test::utils::type_traits;
    static_assert(tt::supports_op_plus_with_v<std::string, std::string>);
    // static_assert(tt::supports_op_plus_with_v<name, std::string>);
    // static_assert(tt::supports_op_plus_with_v<std::string, name>);
    // static_assert(tt::arythmetic::supports_op_plus_with_v<name, name>);
}
namespace test::implicit_conversion {

    constexpr void func(int){}

    using cm = csl::ensure::strong_type<int, struct cm_tag>;
    static_assert(std::is_invocable_v<decltype(func), cm>);
}
namespace test::overload_resolution::better_match {

    using mm = csl::ensure::strong_type<int, struct mm_tag>;
    using cm = csl::ensure::strong_type<int, struct cm_tag>;

    constexpr int func(int){ return 0; }
    constexpr int func(mm){ return 1;}
    constexpr int func(cm){ return 2; }

    static_assert(1 == func(mm{42}));    // NOLINT
    static_assert(2 == func(cm{42}));    // NOLINT
}
namespace test::overload_resolution::match_or_implicit_conversion {

    using mm = csl::ensure::strong_type<int, struct mm_tag>;
    using cm = csl::ensure::strong_type<int, struct cm_tag>;

    constexpr int func(int){ return 0; }
    constexpr int func(mm){ return 1;}

    static_assert(1 == func(mm{42}));    // NOLINT
    static_assert(0 == func(cm{42}));    // NOLINT
}

namespace test::invocation {
    using String = csl::ensure::strong_type<std::string, struct string_tag>;
    constexpr inline auto func = [](const String &){};
    using func_t = decltype(func);

    static_assert(std::is_invocable_v<func_t, const String &>);
    // TODO(Guss): opt-in implicit cast, explicit by default
    // static_assert(not std::is_invocable_v<func_t, String &>);
    // static_assert(not std::is_invocable_v<func_t, String &&>);
    // static_assert(not std::is_invocable_v<func_t, const String &&>);
}

namespace test::CPO {
    using mm = csl::ensure::strong_type<int, struct mm_tag>;
    // std::hash::operator() does not produce an integral constant
    void std_hash(){
        assert(std::hash<mm>{}(mm{42}) == std::hash<int>{}(42));
    }
    void hasher(){
        const auto hasher = csl::ensure::strong_type_hasher{};
        assert(hasher(mm{42}) == std::hash<int>{}(42));
    }
    void comparator(){
        constexpr auto comparator = csl::ensure::strong_type_comparator{};
        static_assert(comparator(mm{42}, mm{42}));
    }
}
namespace test::io_ {
    using mm = csl::ensure::strong_type<int, struct mm_tag>;
    void shift_to_ostream_support(){
        #if defined(CSL_ENSURE__OPT_IN__IOSTREAM_SUPPORT)
        using namespace csl::io;
        std::cout << "CSL_ENSURE__OPT_IN__IOSTREAM_SUPPORT: value = " << mm{42} << '\n';
        #endif
    }

    void fmt_support(){
        #if defined(CSL_ENSURE__OPT_IN__FMT_SUPPORT)
        fmt::print("CSL_ENSURE__OPT_IN__FMT_SUPPORT: value = {}\n", mm{42});
        #endif
    }
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

#include <iostream>
auto main() -> int {
    test::CPO::std_hash();
    test::CPO::hasher();

    // opt-ins
    test::io_::shift_to_ostream_support();
    test::io_::fmt_support();
}
