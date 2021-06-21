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

namespace workflow::cx {
    template <typename T, typename ... Ts>
    constexpr bool are_unique_v = (not (std::is_same_v<T, Ts> or ...)) and are_unique_v<Ts...>;
    template <typename T>
    constexpr bool are_unique_v<T> = true;
}
namespace workflow::functional {

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

    template <typename ... f_ts, typename ... f_args_t, typename F>
    requires
        (not std::invocable<F, f_args_t&&...>)
        and requires {
            std::declval<F&&>().template operator()<f_ts..., f_args_t&&...>(std::declval<f_args_t&&>()...);
        }
    constexpr decltype(auto) invoke(F && f, f_args_t&& ... args)
    #if not __clang__
    noexcept (std::is_nothrow_invocable_v<
        decltype(&(std::decay_t<F>::template operator()<f_ts..., f_args_t&&...>)), F&&, f_args_t&&...
    >)
    #endif
    {
        return std::forward<F>(f).template operator()<f_ts...>(std::forward<f_args_t>(args)...);
    }

    template <typename F, typename ... f_args_t>
    requires
        std::invocable<F, f_args_t&&...>
    constexpr decltype(auto) invoke(F && f, f_args_t&& ... args)
    noexcept (std::is_nothrow_invocable_v<F&&, f_args_t&&...>)
    {
        return std::invoke(std::forward<F>(f), std::forward<f_args_t>(args)...);
    }
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

    template <class T>
    struct is_template : std::false_type {};
    template <class... T_args, template <class...> class T>
    struct is_template<T<T_args...>> : std::true_type {};
    template <auto... values, template <auto...> class T>
    struct is_template<T<values...>> : std::true_type {};
    template <class T>
    inline constexpr auto is_template_v = is_template<T>::value;
}
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

        // explicit template arguments, forwardable
        template <typename ... f1_ts, typename ... f1_args_t> // auto ... f1_vs, 
        requires 
        //     (not std::invocable<F1, f1_args_t&&...>)
        //     and std::invocable<decltype(&F1::template operator()<f1_ts..., f1_args_t...>), F1, f1_args_t&&...>
        //     and std::invocable<F2, std::invoke_result_t<decltype(&F1::template operator()<f1_ts..., f1_args_t...>), F1, f1_args_t&&...>>
        requires {
            functional::invoke(
                std::declval<std::decay_t<F2>>(),
                functional::invoke<f1_ts...>(std::declval<std::decay_t<F1>>(), std::declval<f1_args_t&&>()...)
            );
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const
        // noexcept(
        //     std::is_nothrow_invocable_v<decltype(&F1::template operator()<f1_ts..., f1_args_t&&...>), F1, f1_args_t&&...> and
        //     std::is_nothrow_invocable_v<F2, std::invoke_result_t<F1, decltype(std::forward<decltype(f1_args_v)>(f1_args_v))...>>
        // )
        {
            return functional::invoke(
                f2, 
                functional::invoke<f1_ts...>(f1, std::forward<f1_args_t>(f1_args_v)...));
        }
        // explicit template, not forwardable
        template <typename ... f1_ts, typename ... f1_args_t>
        requires
        requires {
            functional::invoke<f1_ts...>(std::declval<std::decay_t<F1>>(), std::declval<f1_args_t&&>()...);
            functional::invoke(std::declval<std::decay_t<F2>>());
        }
        and (not requires {
            functional::invoke(
                std::declval<std::decay_t<F2>>(),
                functional::invoke<f1_ts...>(std::declval<std::decay_t<F1>>(), std::declval<f1_args_t&&>()...)
            );
        })
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const
        {
            functional::invoke<f1_ts...>(f1, std::forward<f1_args_t>(f1_args_v)...); // discard return value (todo : warning)
            return functional::invoke(f2);
        }

        // not explicit template, forwardable
        // not explicit template, not forwardable

    private:
        mutable std::decay_t<F1> f1;
        mutable std::decay_t<F2> f2;
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
        
        return functional::overload<std::decay_t<F1>, std::decay_t<F2>>{
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
        ](auto &&... f1_args) constexpr mutable {

            //static_assert(std::is_invocable_v<decltype(_f1), decltype(f1_args)...>);
            using f1_invoke_result_t = decltype(functional::invoke(std::declval<decltype(_f1)>(), std::declval<decltype(f1_args)>()...));

            if constexpr (std::is_same_v<void, f1_invoke_result_t>)
            {
                //for (auto count : std::ranges::iota_view<std::size_t>{0, call_count}) // Clang 12.0 issue
                for (auto count = 0; count < call_count; ++count)
                    std::invoke(_f1, std::forward<decltype(f1_args)>(f1_args)...);
                ;
            }
            else return [&]() constexpr {
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

#ifdef CPP_SHELVE_STANDALONE_EDIT__
#include <memory>
#include <typeinfo>
auto main() -> int {

    using namespace workflow::operators;

    // operator>>=
    {
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
        // {   // TTP
        //     constexpr auto route_ttp_parameter = []<typename>(){} >>= [](){ return std::true_type{};};
        //     static_assert(std::is_same_v<
        //         decltype(route_ttp_parameter.template operator()<int>()),
        //         std::true_type
        //     >);
        // }
        // {   // NTTP
        //     constexpr auto route_nttp_parameter = []<bool>(){} >>= [](){ return std::true_type{};};
        //     static_assert(std::is_same_v<
        //         decltype(route_ttp_parameter.template operator()<true>()),
        //         std::true_type
        //     >);
        // }
    }
    // operator|
    {
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
        {   // TTP
            auto route = []<typename switch_type>(){
                using return_type = std::conditional_t<switch_type::value, int, char>;
                return return_type{};
            } >>= (
                [](int) { return std::true_type{}; } |
                [](char){ return std::false_type{}; }
            );

            // static_assert(std::is_same_v<
            //     decltype(route. template operator()<std::true_type>()),
            //     std::true_type
            // >);

            //static_assert(std::is_invocable_v<decltype(&decltype(route):: template operator()<std::true_type>), decltype(route)>);
        }
        {   // NTTP
            auto route = []<auto switch_value>(){
                using return_type = std::conditional_t<switch_value, int, char>;
                return return_type{};
            } >>= (
                [](int) { return std::true_type{}; } |
                [](char){ return std::false_type{}; }
            );

            // route.template operator()<true>();

            // static_assert(std::is_invocable_v<decltype(&decltype(route):: template operator()<true>), decltype(route)>);
            
            // {
            //     auto qwe = []<typename T>(){};

            //     static_assert(not std::is_invocable_v<decltype(qwe)>);
            //     using type = decltype(qwe);

            //     static_assert(std::is_invocable_v<decltype(&type:: template operator()<int>), type>);
            // }

            // static_assert(std::is_same_v<
            //     std::true_type,
            //     std::invoke_result_t<decltype(route)<true>()>
            // >);
            // static_assert(std::is_invocable_r_v<std::false_type, decltype(route), std::false_type>);
        }
    }
    // operator*
    {
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
    // (F1 >>= F2) * int
    {
        {   // runtime test
            auto call_count = 0;
            const auto requested_call_count = 3;
            auto route = ([i = 0]() mutable { return ++i; }
                >>= [&call_count](auto input){
                    if (input not_eq ++call_count)
                        throw std::runtime_error{"(F1 >>= F2) * int : unexpected F2 input"};
                }) * 3;
            route();
            if (call_count not_eq requested_call_count)
                throw std::runtime_error{"(F1 >>= F2) * int : bad call_count"};
        }
        {   // constexpr nodes
            auto route = (
                    [](std::string_view && value) constexpr -> std::string_view { return value; }
                |   [](auto && value) constexpr -> std::string_view
                    requires requires {
                        std::begin(value); 
                        std::end(value);
                    }
                    { return std::string_view { std::begin(value), std::end(value) };}
            )
            >>= (
                [](auto && value) constexpr {
                    static_assert(std::is_same_v<decltype(value), std::string_view>);
                    value.remove_prefix(std::min(value.find_first_not_of(" "), value.size()));
                    return value;
                } * 3
            );
            // >>= [expected_min_size = 5](auto && values) constexpr {
            //     // return expected_min_size >= value.size();
            //     return true;
            // }
            ;
            // static_assert(std::is_invocable_v<decltype(route), std::string>);
            // static_assert(std::is_invocable_r_v<bool, decltype(route), std::string>);
            // static_assert(std::is_invocable_r_v<bool, decltype(route), std::string_view>);
            // static_assert(std::is_invocable_r_v<bool, decltype(route), std::vector<char>>);
        }
        {   // constexpr route

        }
    }

    {   // invoke synthaxes ...
        auto qwe = []<typename T = char>(int){};

        using result_t   = std::invoke_result_t<decltype(&decltype(qwe):: template operator()<bool>), decltype(qwe), int>;
        using result_t_2 = std::invoke_result_t<decltype(&decltype(qwe):: template operator()<>), decltype(qwe), int>;
        using result_t_3 = std::invoke_result_t<decltype(qwe), int>;
    }
}
#endif
