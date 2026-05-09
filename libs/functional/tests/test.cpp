#include <csl/functional.hpp>
#include <catch2/catch_test_macros.hpp>
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

TEST_CASE("functional::function_trait", "[functional][compile_time]") {

    SECTION("function") {
        using trait = csl::functional::function_trait<decltype(test::sample::func)>;
        STATIC_REQUIRE(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
        STATIC_REQUIRE(std::is_same_v<trait::result_type, char>);
        STATIC_REQUIRE(std::tuple_size_v<trait::arguments_type> == 1);
        STATIC_REQUIRE(std::is_same_v<std::tuple_element_t<0, trait::arguments_type>, int>);
    }

    SECTION("member_function") {
        using trait = csl::functional::function_trait<decltype(&test::sample::user_defined_type::mem_func)>;
        STATIC_REQUIRE(std::is_same_v<trait::arguments_type, csl::functional::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(std::is_same_v<trait::result_type, char>);
        STATIC_REQUIRE(std::tuple_size_v<trait::arguments_type> == 2);
        STATIC_REQUIRE(std::is_same_v<std::tuple_element_t<0, trait::arguments_type>, test::sample::user_defined_type>);
        STATIC_REQUIRE(std::is_same_v<std::tuple_element_t<1, trait::arguments_type>, int>);
    }

    SECTION("lambda") {
        using trait = csl::functional::function_trait<test::sample::lambda_const_t>;
        STATIC_REQUIRE(std::is_same_v<trait::arguments_type, csl::functional::arguments<int>>);
        STATIC_REQUIRE(std::is_same_v<trait::result_type, char>);
        STATIC_REQUIRE(std::tuple_size_v<trait::arguments_type> == 1);
        STATIC_REQUIRE(std::is_same_v<std::tuple_element_t<0, trait::arguments_type>, int>);
    }
}

TEST_CASE("functional::concepts::simple_callable", "[functional][compile_time]") {
    namespace ns = csl::functional;
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(test::sample::lambda_const)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(test::sample::lambda_mutable)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(test::sample::lambda_const_noexcept)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(test::sample::func)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(test::sample::func_noexcept)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(&test::sample::user_defined_type::static_mem_func)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(&test::sample::user_defined_type::static_mem_func_noexcept)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(&test::sample::user_defined_type::mem_func)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(&test::sample::user_defined_type::mem_func_noexcept)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(&test::sample::user_defined_type::mem_func_const)>);
    STATIC_REQUIRE(ns::concepts::simple_callable<decltype(&test::sample::user_defined_type::mem_func_const_noexcept)>);
}

TEST_CASE("functional::concepts::invocable_with", "[functional][compile_time]") {
    namespace ns = csl::functional;

    SECTION("simple callables") {
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(test::sample::lambda_const), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(test::sample::lambda_mutable), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(test::sample::lambda_const_noexcept), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(test::sample::func), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(test::sample::func_noexcept), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(&test::sample::user_defined_type::static_mem_func), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(&test::sample::user_defined_type::static_mem_func_noexcept), ns::arguments<int>>);
    }

    SECTION("member functions") {
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(&test::sample::user_defined_type::mem_func), ns::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(&test::sample::user_defined_type::mem_func_noexcept), ns::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(&test::sample::user_defined_type::mem_func_const), ns::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(ns::concepts::invocable_with<decltype(&test::sample::user_defined_type::mem_func_const_noexcept), ns::arguments<test::sample::user_defined_type, int>>);
    }
}

TEST_CASE("functional::concepts::nothrow_invocable_with", "[functional][compile_time]") {
    namespace ns = csl::functional;

    SECTION("simple callables") {
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(test::sample::lambda_const), ns::arguments<int>>);
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(test::sample::lambda_mutable), ns::arguments<int>>);
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(test::sample::lambda_const_noexcept), ns::arguments<int>>);
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(test::sample::func), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::nothrow_invocable_with<decltype(test::sample::func_noexcept), ns::arguments<int>>);
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(&test::sample::user_defined_type::static_mem_func), ns::arguments<int>>);
        STATIC_REQUIRE(ns::concepts::nothrow_invocable_with<decltype(&test::sample::user_defined_type::static_mem_func_noexcept), ns::arguments<int>>);
    }

    SECTION("member functions") {
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(&test::sample::user_defined_type::mem_func), ns::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(ns::concepts::nothrow_invocable_with<decltype(&test::sample::user_defined_type::mem_func_noexcept), ns::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(not ns::concepts::nothrow_invocable_with<decltype(&test::sample::user_defined_type::mem_func_const), ns::arguments<test::sample::user_defined_type, int>>);
        STATIC_REQUIRE(ns::concepts::nothrow_invocable_with<decltype(&test::sample::user_defined_type::mem_func_const_noexcept), ns::arguments<test::sample::user_defined_type, int>>);
    }
}

TEST_CASE("functional::overload_trait", "[functional][compile_time]") {
    constexpr auto overloaded = csl::functional::overload {
        []() -> bool { return {}; },
        [](int) -> float { return {}; },
        [](char, bool) -> double { return {}; }
    };
    using overloaded_t = std::decay_t<decltype(overloaded)>;
    using trait = csl::functional::overload_trait_t<overloaded_t>;

    STATIC_REQUIRE(std::tuple_size_v<trait> == 3);

    STATIC_REQUIRE(std::is_same_v<
        typename std::tuple_element_t<0, trait>::arguments_type,
        csl::functional::arguments<>
    >);
    STATIC_REQUIRE(std::is_same_v<
        typename std::tuple_element_t<1, trait>::arguments_type,
        csl::functional::arguments<int>
    >);
    STATIC_REQUIRE(std::is_same_v<
        typename std::tuple_element_t<2, trait>::arguments_type,
        csl::functional::arguments<char, bool>
    >);

    STATIC_REQUIRE(std::is_same_v<bool,   typename std::tuple_element_t<0, trait>::result_type>);
    STATIC_REQUIRE(std::is_same_v<float,  typename std::tuple_element_t<1, trait>::result_type>);
    STATIC_REQUIRE(std::is_same_v<double, typename std::tuple_element_t<2, trait>::result_type>);
}

// TODO(Guss): overload_set https://godbolt.org/z/YG1x9z3qo
//  Fs...
//  T, T::MemFs...
