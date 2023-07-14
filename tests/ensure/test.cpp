// TODO: cxx_17 cmake target

#if __cplusplus >= 202002L
#include <csl/ensure.hpp>
#include <concepts>
#else
#include <csl/cxx_17/ensure.hpp>
#endif
#include <type_traits>

namespace test::utils {
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
    static_assert(not utils::has_type_v<tt::tag_type<meters_tag>>);

    // tag_type
    static_assert(    std::is_same_v<meters_tag, tt::tag_type_t<meters>>);
    static_assert(not std::is_same_v<char,       tt::tag_type_t<meters>>);
    static_assert(not utils::has_type_v<tt::underlying_type<int>>);
}
#if __cplusplus >= 202002L
namespace test::strong_type::concepts {
    namespace c = csl::ensure::concepts;
    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    static_assert(c::StrongType<meters>);
    static_assert(c::NotStrongType<int>);
    static_assert(c::StrongTypeOf<meters, int>);
}
#endif
namespace test::strong_type::construction {
    using String = csl::ensure::strong_type<std::string, struct string_tag>;
    static_assert(std::is_constructible_v<String>);
    static_assert(std::is_constructible_v<String, std::string&&>);
    static_assert(std::is_constructible_v<String, const std::string&>);
    static_assert(std::is_constructible_v<String, const char*>);
    static_assert(std::is_constructible_v<String, std::size_t, std::string::value_type>);
}
namespace test::strong_type::comparisons {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    static_assert(42 == meters{ 42 });                  // NOLINT
    static_assert(meters{ 42 } == 42);                  // NOLINT
    static_assert(meters{ 42 } == meters{ 42 });    // NOLINT 

#if __cplusplus >= 202002L
    static_assert(std::three_way_comparable_with<meters, meters>);
    static_assert(std::three_way_comparable_with<meters, const int>);
    static_assert(std::three_way_comparable_with<int,    const meters>);
#endif
}
#if __cplusplus >= 202002L
namespace test::strong_type::arithmetic {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    // TODO: C++17
    template <typename T, typename U>
    concept supports_plus = requires { std::declval<T&>() + std::declval<const U &>(); };

    static_assert(supports_plus<meters, meters>);
    static_assert(supports_plus<meters, int>);
    static_assert(supports_plus<int,    meters>);
}
#endif
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
