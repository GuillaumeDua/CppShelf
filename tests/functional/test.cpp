#include <csl/functional.hpp>
#include <type_traits>

namespace test::sample {

    [[maybe_unused]] static const auto lambda_const = [](int) -> char { return {}; };
    using lambda_const_t = decltype(lambda_const);
    [[maybe_unused]] static const auto lambda_mutable = [](int) -> char { return {}; };
    using lambda_mutable_t = decltype(lambda_mutable);
    [[maybe_unused]] static const auto lambda_const_noexcept = [](int) -> char { return {}; };
    using lambda_const_noexcept_t = decltype(lambda_const_noexcept);

    [[maybe_unused]] auto func(int) -> char { return {}; }
    [[maybe_unused]] auto func_noexcept(int) noexcept -> char { return {}; }

    struct user_defined_type {
        // NOLINTBEGIN
        [[maybe_unused]] auto mem_func(int) -> char { return {}; }
        [[maybe_unused]] auto mem_func_noexcept(int) noexcept -> char { return {}; }
        [[maybe_unused]] auto mem_func_const(int) const -> char { return {}; }
        [[maybe_unused]] auto mem_func_const_noexcept(int) const noexcept -> char { return {}; }
        // NOLINTEND
        [[maybe_unused]] static auto static_mem_func(int) -> char { return {}; }
        [[maybe_unused]] static auto static_mem_func_noexcept(int) noexcept -> char { return {}; }
    };
}

namespace test::function {

    using trait = csl::functional::function_trait<decltype(sample::func)>;
    static_assert(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
    static_assert(std::is_same_v<trait::result_type, char>);
}
namespace test::member_function {
    using trait = csl::functional::function_trait<decltype(&sample::user_defined_type::mem_func)>;
    static_assert(std::is_same_v<trait::arguments_type, csl::functional::arguments<sample::user_defined_type, int>>);
    static_assert(std::is_same_v<trait::result_type, char>);
}
namespace test::lambda {

    using trait = csl::functional::function_trait<sample::lambda_const_t>;
    static_assert(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
    static_assert(std::is_same_v<trait::result_type, char>);
}
namespace test::concepts {

    namespace ns = csl::functional;

    // simple_callable
    static_assert(ns::concepts::simple_callable<decltype(sample::lambda_const)>);
    static_assert(ns::concepts::simple_callable<decltype(sample::lambda_mutable)>);
    static_assert(ns::concepts::simple_callable<decltype(sample::lambda_const_noexcept)>);
    static_assert(ns::concepts::simple_callable<decltype(sample::func)>);
    static_assert(ns::concepts::simple_callable<decltype(sample::func_noexcept)>);
    static_assert(ns::concepts::simple_callable<decltype(&sample::user_defined_type::static_mem_func)>);
    static_assert(ns::concepts::simple_callable<decltype(&sample::user_defined_type::static_mem_func_noexcept)>);
    static_assert(ns::concepts::simple_callable<decltype(&sample::user_defined_type::mem_func)>);
    static_assert(ns::concepts::simple_callable<decltype(&sample::user_defined_type::mem_func_noexcept)>);
    static_assert(ns::concepts::simple_callable<decltype(&sample::user_defined_type::mem_func_const)>);
    static_assert(ns::concepts::simple_callable<decltype(&sample::user_defined_type::mem_func_const_noexcept)>);

    // invocable_with
    static_assert(ns::concepts::invocable_with<decltype(sample::lambda_const), ns::arguments<int>>);
    static_assert(ns::concepts::invocable_with<decltype(sample::lambda_mutable), ns::arguments<int>>);
    static_assert(ns::concepts::invocable_with<decltype(sample::lambda_const_noexcept), ns::arguments<int>>);
    static_assert(ns::concepts::invocable_with<decltype(sample::func), ns::arguments<int>>);
    static_assert(ns::concepts::invocable_with<decltype(sample::func_noexcept), ns::arguments<int>>);
    static_assert(ns::concepts::invocable_with<decltype(&sample::user_defined_type::static_mem_func), ns::arguments<int>>);
    static_assert(ns::concepts::invocable_with<decltype(&sample::user_defined_type::static_mem_func_noexcept), ns::arguments<int>>);

    static_assert(ns::concepts::invocable_with<decltype(&sample::user_defined_type::mem_func), ns::arguments<sample::user_defined_type, int>>);
    static_assert(ns::concepts::invocable_with<decltype(&sample::user_defined_type::mem_func_noexcept), ns::arguments<sample::user_defined_type, int>>);
    static_assert(ns::concepts::invocable_with<decltype(&sample::user_defined_type::mem_func_const), ns::arguments<sample::user_defined_type, int>>);
    static_assert(ns::concepts::invocable_with<decltype(&sample::user_defined_type::mem_func_const_noexcept), ns::arguments<sample::user_defined_type, int>>);

    // nothrow_invocable_with
    static_assert(not ns::concepts::nothrow_invocable_with<decltype(sample::lambda_const), ns::arguments<int>>);
    static_assert(not ns::concepts::nothrow_invocable_with<decltype(sample::lambda_mutable), ns::arguments<int>>);
    static_assert(not ns::concepts::nothrow_invocable_with<decltype(sample::lambda_const_noexcept), ns::arguments<int>>);
    static_assert(not ns::concepts::nothrow_invocable_with<decltype(sample::func), ns::arguments<int>>);
    static_assert(ns::concepts::nothrow_invocable_with<decltype(sample::func_noexcept), ns::arguments<int>>);
    static_assert(not ns::concepts::nothrow_invocable_with<decltype(&sample::user_defined_type::static_mem_func), ns::arguments<int>>);
    static_assert(ns::concepts::nothrow_invocable_with<decltype(&sample::user_defined_type::static_mem_func_noexcept), ns::arguments<int>>);

    static_assert(not ns::concepts::nothrow_invocable_with<decltype(&sample::user_defined_type::mem_func), ns::arguments<sample::user_defined_type, int>>);
    static_assert(ns::concepts::nothrow_invocable_with<decltype(&sample::user_defined_type::mem_func_noexcept), ns::arguments<sample::user_defined_type, int>>);
    static_assert(not ns::concepts::nothrow_invocable_with<decltype(&sample::user_defined_type::mem_func_const), ns::arguments<sample::user_defined_type, int>>);
    static_assert(ns::concepts::nothrow_invocable_with<decltype(&sample::user_defined_type::mem_func_const_noexcept), ns::arguments<sample::user_defined_type, int>>);
}

namespace test::overloads_traits::utils {

    template <typename overloaded_t>
    constexpr void check(){

        using trait = csl::functional::overload_trait_t<overloaded_t>;

        static_assert(std::tuple_size_v<trait> == 3);

        // overload: argument types
        static_assert(std::is_same_v<
            typename std::tuple_element_t<0, trait>::arguments_type,
            csl::functional::arguments<>
        >);
        static_assert(std::is_same_v<
            typename std::tuple_element_t<1, trait>::arguments_type,
            csl::functional::arguments<int>
        >);
        static_assert(std::is_same_v<
            typename std::tuple_element_t<2, trait>::arguments_type,
            csl::functional::arguments<char, bool>
        >);
        // overload: result types
        static_assert(std::is_same_v<bool,   typename std::tuple_element_t<0, trait>::result_type>);
        static_assert(std::is_same_v<float,  typename std::tuple_element_t<1, trait>::result_type>);
        static_assert(std::is_same_v<double, typename std::tuple_element_t<2, trait>::result_type>);
    }
}
namespace test::overloads_traits::overload {

    constexpr auto overloaded = csl::functional::overload {
        []() -> bool { return {}; },
        [](int) -> float { return {}; },
        [](char, bool) -> double { return {}; }
    };
    using overloaded_t = std::decay_t<decltype(overloaded)>;

    constexpr void do_check(){ utils::check<overloaded_t>(); }
}

// TODO(Guss): overload_set https://godbolt.org/z/YG1x9z3qo
//  Fs...
//  T, T::MemFs...

// namespace test::overloads_traits::function {
    
//     auto func() -> bool { return {}; }
//     auto func(int) -> float { return {}; }
//     auto func(char, bool) -> double { return {}; }

//     constexpr auto overloaded = csl::functional::overload {
//         static_cast<bool(*)()>(&func)
//     };
//     using overloaded_t = std::decay_t<decltype(overloaded)>;
    
//     using overloaded_t = std::decay_t<decltype(&func)>;

//     constexpr static void do_checks(){ utils::check<overloaded_t>(); }
// }
// namespace test::overloads_traits::user_defined_functor {
    
//     struct type {
//         auto operator()() -> bool { return {}; }
//         auto operator()(int) -> float { return {}; }
//         auto operator()(char, bool) -> double { return {}; }
//     };

//     constexpr static void do_checks(){ utils::check<type>(); }
// }

auto main() -> int {}

