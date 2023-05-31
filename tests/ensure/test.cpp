#include <csl/ensure.hpp>

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
namespace test::strong_type::comparisons {
    using meters = csl::ensure::strong_type<int, struct meters_tag>;
    static_assert(42 == meters{ 42 });
    static_assert(meters{ 42 } == 42);
    static_assert(meters{ 42 } == meters{ 42 });
}

auto main() -> int {
    
}