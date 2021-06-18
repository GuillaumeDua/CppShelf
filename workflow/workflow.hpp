#pragma once

#include <iostream>
#include <utility>
#include <type_traits>
#include <variant>
#include <concepts>
#include <functional>
#include <ranges>
#include <vector>
#include <array>

namespace workflow::concepts {

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
namespace workflow::type_traits {
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

    template <typename T, typename U, typename = void>
    struct have_plus_operator : std::false_type{};
    template <typename T, typename U>
    struct have_plus_operator<T, U, std::void_t<decltype(std::declval<T>() + std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_plus_operator_v = have_plus_operator<T, U>::value;

    template <typename T, typename U, typename = void>
    struct have_multiply_operator : std::false_type{};
    template <typename T, typename U>
    struct have_multiply_operator<T, U, std::void_t<decltype(std::declval<T>() * std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_multiply_operator_v = have_multiply_operator<T, U>::value;
}
namespace workflow::cx {
    template <typename T, typename ... Ts>
    constexpr bool are_unique_v = (not (std::is_same_v<T, Ts> or ...)) and are_unique_v<Ts...>;
    template <typename T>
    constexpr bool are_unique_v<T> = true;
}
namespace workflow::utility {
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

namespace workflow {
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

        // template <typename ... f1_ts, auto ... f1_vs, typename ... f1_args_t>
        // requires concepts::are_only_independently_invocable<F1, F2, f1_args_t...>
        // constexpr auto operator()(f1_args_t && ... f1_args_v)
        // noexcept(
        //     std::is_nothrow_invocable_v<F1, f1_args_t&&...> and
        //     std::is_nothrow_invocable_v<F2>
        // )
        // {
        //     if constexpr (not std::same_as<void, std::invoke_result_t<F1, f1_args_t&&...>>) {
        //         #pragma message("functor_1 invocation result will be discard")
        //     }

        //     if constexpr (sizeof...(f1_ts) == 0 and sizeof...(f1_vs) == 0)
        //         std::invoke(f1, std::forward<decltype(f1_args_v)>(f1_args_v)...);
        //     else
        //         std::invoke(F1::template operator()<f1_ts..., f1_vs...>(), f1, std::forward<decltype(f1_args_v)>(f1_args_v)...);
            
        //     return std::invoke(f2);
        // }

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
            return std::invoke(f2);
        }
        template <typename ... f1_ts, auto ... f1_vs, typename ... f1_args_t>
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
}
namespace workflow::operators {

    // todo : operator / operator&
    // todo : tuple interface -> tuple, pair, array
    //        destructured bindings
    //  special node to wrap such thing ?
    //  [](){ return std::tuple{'a', 42};} >>= workflow::unfold >>= ([](char){} & [](int){})
    //  that internally call std::apply based on overload resolution
    //
    // ([](){ return 'a';} + [](){ return 42;}) >>= workflow::unfold >>= ([](char){} & [](int){})

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
    template <
        typename F1,
        typename std::enable_if_t<not type_traits::have_plus_operator_v<F1, std::size_t>> * = nullptr
    >   // should add more requirements, that detects template operator()
    constexpr auto operator*(F1 && f1_value, std::size_t call_count) noexcept {

        return [
            _f1 = std::move(f1_value),
            call_count
        ](auto &&... f1_args) mutable {

            static_assert(std::is_invocable_v<decltype(_f1), decltype(f1_args)...>);
            using f1_invoke_result_t = std::invoke_result_t<decltype(_f1), decltype(f1_args)...>;

            if constexpr (std::is_same_v<void, f1_invoke_result_t>)
            {
                //for (auto count : std::ranges::iota_view<std::size_t>{0, call_count}) // Clang 12.0 issue
                for (auto count = 0; count < call_count; ++count)
                    std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...);
                ;
            }
            else return [&](){
                static_assert(std::semiregular<f1_invoke_result_t>);
                using return_type = std::vector<f1_invoke_result_t>;
                return_type return_value(call_count);
                std::generate(std::begin(return_value), std::end(return_value), [&](){
                    return std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...);
                });
                return return_value;
            }();
        };
    }
    template <
        typename F1,
        auto call_count,
        typename std::enable_if_t<
            not type_traits::have_plus_operator_v<
                F1,
                std::integral_constant<std::size_t, call_count>
            >
        > * = nullptr
    >   // should add more requirements, that detects template operator()
    constexpr auto operator*(F1 && f1_value, std::integral_constant<decltype(call_count), call_count>) noexcept {

        return [
            _f1 = std::move(f1_value)
        ](auto &&... f1_args) constexpr mutable {

            static_assert(std::is_invocable_v<decltype(_f1), decltype(f1_args)...>);
            using f1_invoke_result_t = std::invoke_result_t<decltype(_f1), decltype(f1_args)...>;

            if constexpr (std::is_same_v<void, f1_invoke_result_t>)
                return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
                    (((void)indexes, std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...)), ...);
                }(std::make_index_sequence<call_count>{});
            else
                return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
                    using return_type = std::array<f1_invoke_result_t, call_count>;
                    return return_type {
                        typename return_type::value_type{
                            ((void)indexes,
                            std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...))
                        }...
                    };
                }(std::make_index_sequence<call_count>{});
        };
    }
}

#include <memory>
#include <typeinfo>
auto main() -> int {

    using namespace workflow::operators;

    // operator>>=
    {
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

    // operator*
    {
        auto route =
            [call_counter = 0]() mutable { return ++call_counter; } *
            3
        ;
        static_assert(std::is_invocable_r_v<std::vector<int>, decltype(route)>);
        if (route() not_eq std::vector{1,2,3})
            throw std::runtime_error{"operator*<F, std::size_t> : unexpected return values"};
    }
    {
        using call_count_t = std::integral_constant<int, 3>;
        auto route =
            [call_counter = 0]() mutable {  return ++call_counter; } *
            call_count_t{}
        ;
        static_assert(std::is_invocable_r_v<std::array<int, call_count_t::value>, decltype(route)>);
        if (route() not_eq std::array{1,2,3})
            throw std::runtime_error{"operator*<F, std::size_t> : unexpected return values"};
    }
    {   // F * integral_constant => compile-time value
        using call_count_t = std::integral_constant<int, 3>;
        auto route =
            []() constexpr {  return 42; } *
            call_count_t{}
        ;
        static_assert(std::is_invocable_r_v<std::array<int, call_count_t::value>, decltype(route)>);
        static_assert(route() == std::array{42,42,42});
    }
}