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
    ;
    template <typename F1, typename F2, typename ... F1_args>
    concept are_independently_invocable = 
        std::invocable<F1, F1_args...> and
        std::invocable<F2>
    ;
    template <typename F1, typename F2, typename ... F1_args>
    concept are_only_independently_invocable =
        are_independently_invocable<F1, F2, F1_args...> and
        not is_invoke_result_forwardable<F1, F2, F1_args...>;
    ;
}
namespace utility {
    template <typename ... Ts>
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
    constexpr auto operator()(f1_args_t && ... f1_args_v) {

        if constexpr (not std::same_as<void, std::invoke_result_t<F1, f1_args_t&&...>>) {
            #pragma message("functor_1 invocation result will be discard")
        }

        std::invoke(f1, std::forward<decltype(f1_args_v)>(f1_args_v)...);
        std::invoke(f2);
    }
    template <typename ... f1_args_t>
    requires concepts::is_invoke_result_forwardable<F1, F2, f1_args_t...>
    constexpr auto operator()(f1_args_t && ... f1_args_v) {
        using F1_invoke_result_t = std::invoke_result_t<F1, decltype(std::forward<decltype(f1_args_v)>(f1_args_v))...>;
        return std::invoke(f2, std::invoke(f1, std::forward<decltype(f1_args_v)>(f1_args_v)...));
    }

private:
    std::decay_t<F1> f1;
    std::decay_t<F2> f2;
};

template <typename F1, typename F2>
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

template <typename F1, typename F2>
constexpr auto operator|(F1 && lhs, F2 && rhs) {
    
    return utility::overload<std::decay_t<F1>, std::decay_t<F2>>{
        std::forward<F1>(lhs),
        std::forward<F2>(rhs)
    };
}

#include <memory>
auto main() -> int {

    // operator>>=
    {
        auto node = [](){
            std::cout << "node 1\n";
            return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
        } >>= [](auto value){
            std::cout << "node 2 (value : " << *value << ")\n";
        };
        node();
    }
    {
        char filler = 'X';
        auto node = [&filler](int value){
            std::cout << "node 1\n";
            return std::make_unique<std::string>(value, filler);
        } >>= [](auto value){
            std::cout << "node 2 (value : " << *value << ")\n";
        };
        node(42);
    }
    {
        auto node = [](){
            std::cout << "node 1\n";
            return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
        } >>= [](auto value){
            std::cout << "node 2 (value : " << *value << ")\n";
        }  >>= [](){
            std::cout << "node 3\n";
        };
        node();
    }
    {
        auto node = [](){
            std::cout << "node 1\n";
            return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
        } >>= [](auto && value){
            std::cout << "node 2 (value : " << *value << ")\n";
            return std::forward<decltype(value)>(value);
        }  >>= [](auto && value){
            std::cout << "node 3 (value : " << *value << ")\n";
        };
        node();
    }

    // operator|
    {
        auto toto =
            [](int){} |
            [](char){} |
            [](std::string){}
            ;
        static_assert(std::is_invocable_v<decltype(toto), int>);
        static_assert(std::is_invocable_v<decltype(toto), char>);
        static_assert(std::is_invocable_v<decltype(toto), std::string>);

        auto titi =
            [](double){} |
            [](float){}
            ;

        static_assert(std::is_invocable_v<decltype(titi), double>);
        static_assert(std::is_invocable_v<decltype(titi), float>);

        auto result = toto | titi;

        static_assert(std::is_invocable_v<decltype(result), int>);
        static_assert(std::is_invocable_v<decltype(result), char>);
        static_assert(std::is_invocable_v<decltype(result), std::string>);
        static_assert(std::is_invocable_v<decltype(result), double>);
        static_assert(std::is_invocable_v<decltype(result), float>);
    }

}