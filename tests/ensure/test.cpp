#include <concepts>
#include <csl/ensure.hpp>
#include <type_traits>
#include <cassert>

namespace test::strong_type::type_traits {

    template <typename T>
    concept has_type = requires { typename T::type; };

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
    static_assert(not has_type<tt::underlying_type<int>>);

    // tag_type
    static_assert(    std::is_same_v<meters_tag, tt::tag_type_t<meters>>);
    static_assert(not std::is_same_v<char,       tt::tag_type_t<meters>>);
    static_assert(not has_type<tt::tag_type<meters_tag>>);
}
namespace test::strong_type::concepts {
    namespace c = csl::ensure::concepts;
    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    static_assert(c::StrongType<meters>);
    static_assert(c::NotStrongType<int>);
    static_assert(c::StrongTypeOf<meters, int>);
}
namespace test::strong_type::construction {
    using String = csl::ensure::strong_type<std::string, struct string_tag>;
    static_assert(std::constructible_from<String>);
    static_assert(std::constructible_from<String, std::string&&>);
    static_assert(std::constructible_from<String, const std::string&>);
    static_assert(std::constructible_from<String, const char*>);
    static_assert(std::constructible_from<String, std::size_t, std::string::value_type>);
}
namespace test::strong_type::comparisons {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    static_assert(42 == meters{ 42 });                  // NOLINT
    static_assert(meters{ 42 } == 42);                  // NOLINT
    static_assert(meters{ 42 } == meters{ 42 });    // NOLINT 

    static_assert(std::three_way_comparable_with<meters, meters>);
    static_assert(std::three_way_comparable_with<meters, const int>);
    static_assert(std::three_way_comparable_with<int,    const meters>);
}
namespace test::strong_type::arithmetic {

    using meters = csl::ensure::strong_type<int, struct meters_tag>;

    template <typename T, typename U>
    concept supports_plus = requires { std::declval<T&>() + std::declval<const U &>(); };

    static_assert(supports_plus<meters, meters>);
    static_assert(supports_plus<meters, int>);
    static_assert(supports_plus<int,    meters>);
}
namespace test::implicit_conversion {

    constexpr void func(int){}

    using cm = csl::ensure::strong_type<int, struct cm_tag>;
    static_assert(std::invocable<decltype(func), cm>);
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
namespace test::CPO {
    // std::hash::operator() does not produce an integral constant
    void std_hash(){
        using mm = csl::ensure::strong_type<int, struct mm_tag>;
        assert(std::hash<mm>{}(mm{42}) == std::hash<int>{}(42));
    }
    void hasher(){
        using mm = csl::ensure::strong_type<int, struct mm_tag>;
        const auto hasher = csl::ensure::strong_type_hasher{};
        assert(hasher(mm{42}) == std::hash<int>{}(42));
    }
}
#include <iostream>
namespace test::io_ {
    void shift_to_ostream_support(){
        using namespace csl::io;
        using mm = csl::ensure::strong_type<int, struct mm_tag>;
        std::cout << mm{42};
    }
    void fmt_support(){
    #if defined (FMT_CORE_H_)
        using mm = csl::ensure::strong_type<int, struct mm_tag>;
        fmt::print("value = {}", meters{42});
    #endif
    }
}

auto main() -> int {
    test::CPO::std_hash();
}
