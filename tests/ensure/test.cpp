// TODO: cxx_17 specific cmake target

#if __cplusplus >= 202002L
#include <csl/ensure.hpp>
#include <concepts>
#else
#include <csl/cxx_17/ensure.hpp>
#endif
#include <type_traits>
#include <array>

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
    constexpr bool has_type_v = has_type<T>::value;
#endif

// supports_op_plus_with
#if __cplusplus >= 202002L
    template <typename T, typename U>
    concept supports_op_plus_with_v = requires { std::declval<T &>() + std::declval<const U &>(); };
#else
    template <class T, class U>
    constexpr bool supports_op_plus_with_v = csl::ensure::details::mp::type_traits::supports_op_plus_with<T,U>::value;
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
namespace test::strong_type::comparisons {

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
namespace test::strong_type::arithmetic {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    // using underlying
    static_assert(utils::type_traits::supports_op_plus_with_v<meters, meters>);
    static_assert(utils::type_traits::supports_op_plus_with_v<meters, int>);
    static_assert(utils::type_traits::supports_op_plus_with_v<int,    meters>);

    // using op forwarding
    using name = csl::ensure::strong_type<std::string, struct name_tag>;
    // namespace tt = csl::ensure::details::mp::type_traits;

    // static_assert(tt::supports_op_plus_with_v<name, name>);
    // static_assert(utils::supports_op_plus_with_v<name, std::string>);
    // static_assert(utils::supports_op_plus_with_v<std::string, name>);
}
namespace implicit_conversion {

    constexpr void func(int){}

    using cm = csl::ensure::strong_type<int, struct cm_tag>;
    static_assert(std::is_invocable_v<decltype(func), cm>);
}
namespace test::overload_resolution {

    using mm = csl::ensure::strong_type<int, struct mm_tag>;
    using cm = csl::ensure::strong_type<int, struct cm_tag>;

    constexpr int func(int){ return 0; }
    constexpr int func(mm){ return 1;}
    constexpr int func(cm){ return 2; }

    static_assert(1 == func(mm{42}));    // NOLINT
    static_assert(2 == func(cm{42}));    // NOLINT
}

#include <iostream>
auto main() -> int {
    std::cout << "running test for C++ " << __cplusplus << '\n';
}
