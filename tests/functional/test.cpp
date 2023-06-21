#include <csl/functional.hpp>

namespace test::function {

    auto func(int){ return 'a'; }
    using trait = csl::functional::function_trait<decltype(func)>;
    static_assert(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
    static_assert(std::is_same_v<trait::result_type, char>);
}
namespace test::member_function {
    struct toto {
        auto func(int){ return 'a'; } // NOLINT(readability-convert-member-functions-to-static)
    };
    using trait = csl::functional::function_trait<decltype(&toto::func)>;
    static_assert(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
    static_assert(std::is_same_v<trait::result_type, char>);
}
namespace test::lambda {

    [[maybe_unused]] const auto func = [](int){ return 'a'; };

    using trait = csl::functional::function_trait<decltype(func)>;
    static_assert(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
    static_assert(std::is_same_v<trait::result_type, char>);
}

auto main() -> int {}
