#pragma once

#include <iostream>
#include <utility>
#include <type_traits>
#include <variant>
#include <concepts>
#include <functional>

namespace concepts {

    template <typename F1, typename F2, typename ... F1_args>
    concept is_invoke_result_forwardable = 
        std::invocable<F1, F1_args...> and
        std::invocable<F2, std::invoke_result_t<F1, F1_args...>>
    ; // is_invocable_r ?
    template <typename F1, typename F2, typename ... F1_args>
    concept are_independently_invocable = 
        std::invocable<F1, F1_args...> and
        std::invocable<F2>
    ;
    template <typename F1, typename F2, typename ... F1_args>
    concept are_only_independently_invocable =
        not is_invoke_result_forwardable<F1, F2, F1_args...>
        and are_independently_invocable<F1, F2, F1_args...>
        ;
    ;
}
namespace type_traits {
    // avoid recursive concepts
    // could use std::detected_t (library fundamentals TS v2)

    template <typename T, typename U, typename = void>
    struct have_pipe_operator : std::false_type{};
    template <typename T, typename U>
    struct have_pipe_operator<T, U, std::void_t<decltype(std::declval<T>() | std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_pipe_operator_v = have_pipe_operator<T, U>::value;

    template <typename T, typename U, typename = void>
    struct have_shift_equal_operator : std::false_type{};
    template <typename T, typename U>
    struct have_shift_equal_operator<T, U, std::void_t<decltype(std::declval<T>() >>= std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_shift_equal_operator_v = have_shift_equal_operator<T, U>::value;
}

namespace cx {
    template <typename T, typename ... Ts>
    constexpr bool are_unique_v = (not (std::is_same_v<T, Ts> or ...)) and are_unique_v<Ts...>;
    template <typename T>
    constexpr bool are_unique_v<T> = true;
}
namespace utility {
    template <typename ... Ts>
    requires (cx::are_unique_v<Ts...>)
    struct overload : Ts... {
        using Ts::operator()...;
    };
    template <typename ... Ts>
    overload(Ts&&...) -> overload<Ts...>;
    // collapse
    template <typename ... Ts, typename ... Us>
    overload(overload<Ts...>&&, overload<Us...>&&) -> overload<Ts..., Us...>;
    template <typename ... Ts, typename U>
    overload(overload<Ts...>&&, U &&) -> overload<Ts..., U>;
}

template <typename F1, typename F2>
struct node {

    node() = delete;
    node(const node&) = delete;
    node(node&&) = default;

    node& operator=(const node&) = delete;
    node& operator=(node&&) = delete;

    constexpr node(F1 && f1_value, F2 && f2_value)
    : f1(std::forward<F1>(f1_value))
    , f2(std::forward<F2>(f2_value))
    {}

    template <typename ... f1_args_t>
    requires concepts::are_only_independently_invocable<F1, F2, f1_args_t...>
    constexpr auto operator()(f1_args_t && ... f1_args_v)
    noexcept(
        std::is_nothrow_invocable_v<F1, f1_args_t&&...> and
        std::is_nothrow_invocable_v<F2>
    )
    {
        if constexpr (not std::same_as<void, std::invoke_result_t<F1, f1_args_t&&...>>) {
            #pragma message("functor_1 invocation result will be discard")
        }

        std::invoke(f1, std::forward<decltype(f1_args_v)>(f1_args_v)...);
        std::invoke(f2);
    }
    template <typename ... f1_args_t>
    requires concepts::is_invoke_result_forwardable<F1, F2, f1_args_t...>
    constexpr auto operator()(f1_args_t && ... f1_args_v)
    noexcept(
        std::is_nothrow_invocable_v<F1, f1_args_t&&...> and
        std::is_nothrow_invocable_v<F2, std::invoke_result_t<F1, decltype(std::forward<decltype(f1_args_v)>(f1_args_v))...>>
    )
    {
        using F1_invoke_result_t = std::invoke_result_t<F1, decltype(std::forward<decltype(f1_args_v)>(f1_args_v))...>;
        return std::invoke(f2, std::invoke(f1, std::forward<decltype(f1_args_v)>(f1_args_v)...));
    }

private:
    std::decay_t<F1> f1;
    std::decay_t<F2> f2;
};

template <
    typename F1, typename F2,
    typename std::enable_if_t<not type_traits::have_shift_equal_operator_v<F1, F2>> * = nullptr
>   // should add more requirements, that detects template operator()
constexpr auto operator>>=(F1 && f1_value, F2&& f2_value) noexcept {

    return node<F1,F2>{std::forward<F1>(f1_value), std::forward<F2>(f2_value)};

    // return [
    //     _f1 = std::move(f1_value),
    //     _f2 = std::move(f2_value)
    // ](auto &&... f1_args) mutable {
    //     static_assert(std::is_invocable_v<F1, decltype(f1_args)...>);

    //     using F1_invoke_result_t = std::invoke_result_t<F1, decltype(std::forward<decltype(f1_args)>(f1_args))...>;
    //     if  constexpr (std::is_same_v<void, F1_invoke_result_t>) {
    //         // std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...);
    //         // static_assert(std::is_invocable_v<F2>);
    //         // std::invoke(_f2);
    //     }
    //     else {
    //         static_assert(std::is_invocable_v<F2, F1_invoke_result_t>);
    //         std::invoke(_f2, std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...));
    //     }
    // };
}


// todo : +operator,
// todo : *operator
// todo : tuple interface -> tuple, pair, array
//        destructured bindings

template <
    typename F1, typename F2,
    typename std::enable_if_t<not type_traits::have_pipe_operator_v<F1, F2>> * = nullptr
>   // should add more requirements, that detects template operator()
constexpr auto operator|(F1 && lhs, F2 && rhs) {
    
    return utility::overload<std::decay_t<F1>, std::decay_t<F2>>{
        std::forward<F1>(lhs),
        std::forward<F2>(rhs)
    };
}

#include <memory>
auto main() -> int {

    // operator>>=
    
        auto route = [](){
            std::cout << "node 1\n";
            return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
        } >>= [](auto value){
            std::cout << "node 2 (value : " << *value << ")\n";
            return std::integral_constant<int, 42>{};
        };
        static_assert(std::is_invocable_r_v<std::integral_constant<int, 42>, decltype(route)>);
    }
    {
        char filler = 'X';
        auto route = [&filler](int value){
            std::cout << "node 1\n";
            return std::make_unique<std::string>(value, filler);
        } >>= [](auto value){
            std::cout << "node 2 (value : " << *value << ")\n";
        };
        static_assert(std::is_invocable_v<decltype(route), int>);
    }
    {
        auto route = [](){
            std::cout << "node 1\n";
            return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
        } >>= [](auto value){
            std::cout << "node 2 (value : " << *value << ")\n";
        }  >>= [](){
            std::cout << "node 3\n";
            return std::integral_constant<int, 42>{};
        };
        static_assert(std::is_invocable_r_v<std::integral_constant<int, 42>, decltype(route)>);
    }
    {
        auto route = [](){
            std::cout << "node 1\n";
            return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
        } >>= [](auto && value){
            std::cout << "node 2 (value : " << *value << ")\n";
            return std::forward<decltype(value)>(value);
        }  >>= [](auto && value){
            std::cout << "node 3 (value : " << *value << ")\n";
            return value;
        };
        static_assert(std::is_invocable_r_v<std::unique_ptr<std::string>&&, decltype(route)>);
    }
    {   // todo TTP, NTTP
        auto route_ttp_parameter = []<typename>(){} >>= [](){ return std::true_type{};};
        auto route_nttp_parameter = []<bool>(){} >>= [](){ return std::true_type{};};
    }

    // operator|
    {
        auto node_1 =
            [](int){} |
            [](char){} |
            [](std::string){}
            ;
        static_assert(std::is_invocable_v<decltype(node_1), int>);
        static_assert(std::is_invocable_v<decltype(node_1), char>);
        static_assert(std::is_invocable_v<decltype(node_1), std::string>);

        auto node_2 =
            [](double){} |
            [](float){}
            ;

        static_assert(std::is_invocable_v<decltype(node_2), double>);
        static_assert(std::is_invocable_v<decltype(node_2), float>);

        auto route = node_1 | node_2;

        static_assert(std::is_invocable_v<decltype(route), int>);
        static_assert(std::is_invocable_v<decltype(route), char>);
        static_assert(std::is_invocable_v<decltype(route), std::string>);
        static_assert(std::is_invocable_v<decltype(route), double>);
        static_assert(std::is_invocable_v<decltype(route), float>);
    }
    {
        auto route =
            []<typename T>() {
                std::cout << "node - T\n";
                return std::true_type{};
            } |
            [](char){
                std::cout << "node - char\n";
                return std::false_type{};
            }
            ;
        static_assert(std::is_same_v<
            std::true_type,
            decltype(route.template operator()<char>())
        >);
        static_assert(std::is_invocable_r_v<
            std::false_type,
            decltype(route),
            char
        >);
    }
    {
        auto route = [](auto switch_value){
            std::cout << "node 1\n";
            using return_type = std::conditional_t<decltype(switch_value)::value, int, char>;
            return return_type{};
        } >>= (
            [](int) { std::cout << "node2 - int\n"; return std::true_type{}; } |
            [](char){ std::cout << "node2 - char\n"; return std::false_type{}; }
        );
        static_assert(std::is_invocable_r_v<std::true_type, decltype(route), std::true_type>);
        static_assert(std::is_invocable_r_v<std::false_type, decltype(route), std::false_type>);
    }
    {   // todo TTP, NTTP
        auto route = []<auto switch_value>(){
            std::cout << "node 1\n";
            using return_type = std::conditional_t<switch_value, int, char>;
            return return_type{};
        } >>= (
            [](int) { std::cout << "node2 - int\n"; return std::true_type{}; } |
            [](char){ std::cout << "node2 - char\n"; return std::false_type{}; }
        );

        // route.operator()<true>();

        // static_assert(std::is_same_v<
        //     std::true_type,
        //     decltype(route.template operator()<true>())
        // >);
        // static_assert(std::is_invocable_r_v<std::false_type, decltype(route), std::false_type>);
    }

}