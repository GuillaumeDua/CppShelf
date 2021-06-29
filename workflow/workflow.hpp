#ifndef CPP_SHELVE_STANDALONE_EDIT__
#pragma once
#else
#include <iostream>
#endif

#include <utility>
#include <type_traits>
#include <variant>
#include <concepts>
#include <functional>
#include <ranges>
#include <vector>
#include <array>
#include <string_view>

#ifndef CPP_SHELVE_STANDALONE_EDIT__
namespace workflow::cx { // gcl::cx
    template <typename T>
    static constexpr /*consteval*/ std::string_view type_name(/*no parameters allowed*/)
    {
    #if defined(__GNUC__) or defined(__clang__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        const char prefix[] = "T = ";
        str_view.remove_prefix(str_view.find(prefix) + sizeof(prefix) - 1);
        str_view.remove_suffix(str_view.length() - str_view.find_first_of(";]"));
    #elif defined(_MSC_VER)
        std::string_view str_view = __FUNCSIG__;
        str_view.remove_prefix(str_view.find(__func__) + sizeof(__func__));
        if (auto enum_token_pos = str_view.find("enum "); enum_token_pos == 0)
            str_view.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        str_view.remove_suffix(str_view.length() - str_view.rfind(">(void)"));
    #else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
    #endif
        return str_view;
    }
    template <typename T>
    constexpr inline auto type_name_v = type_name<T>();
    template <auto value>
    static constexpr std::string_view type_name(/*no parameters allowed*/)
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr std::string_view value_name(/*no parameters allowed*/)
    {
    #if defined(__GNUC__) or defined(__clang__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        const char prefix[] = "value = ";
        str_view.remove_prefix(str_view.find(prefix) + sizeof(prefix) - 1);
        str_view.remove_suffix(str_view.length() - str_view.find_first_of(";]"));
    #elif defined(_MSC_VER)
        std::string_view str_view = __FUNCSIG__;
        str_view.remove_prefix(str_view.find(__func__) + sizeof(__func__));
        if (auto enum_token_pos = str_view.find("enum "); enum_token_pos == 0)
            str_view.remove_prefix(enum_token_pos + sizeof("enum ") - 1);
        str_view.remove_suffix(str_view.length() - str_view.rfind(">(void)"));
    #else
        static_assert(false, "gcl::cx::typeinfo : unhandled plateform");
    #endif
        return str_view;
    }
    template <auto value>
    constexpr inline auto value_name_v = value_name<value>();
}
#endif
namespace workflow::mp {
    template <typename T, typename ... Ts>
    constexpr bool are_unique_v = (not (std::is_same_v<T, Ts> or ...)) and are_unique_v<Ts...>;
    template <typename T>
    constexpr bool are_unique_v<T> = true;
}
namespace workflow::functional {

    template <typename ... Ts>
    requires (mp::are_unique_v<Ts...>)
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

    // poc : https://godbolt.org/z/4sqEY8P57
    // todo : NTTP, mix TTP/NTTP ...
    template <typename F, typename ... f_ts, typename ... f_args_t>
    requires
        (not std::invocable<F&&, f_args_t&&...>)
        and requires (F&& f, f_args_t&& ... args){
            std::forward<F>(f).template operator()<f_ts...>(std::forward<f_args_t>(args)...);
        }
    constexpr decltype(auto) invoke(F && f, f_args_t&& ... args)
    // TODO : noexcept(noexcept(expr))
    // #if not __clang__
    // noexcept (std::is_nothrow_invocable_v<
    //     decltype(&(std::decay_t<F>::template operator()<f_ts..., f_args_t&&...>)), F&&, f_args_t&&...
    // >)
    // #endif
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
// todo : storage should be conditional mutable ?
// - inject mutable (no ref / const)
// or
// - conditional operator() const-qualifier
namespace workflow {
    template <typename F1, typename F2>
    requires
        std::move_constructible<F1> and
        std::move_constructible<F2> and
        (not std::is_const_v<std::remove_reference_t<F1>>) and
        (not std::is_const_v<std::remove_reference_t<F2>>)
    struct then {

        using _F1 = std::remove_cvref_t<F1>;
        using _F2 = std::remove_cvref_t<F2>;

        then() = delete;
        then(const then&) = delete;
        constexpr then(then&&) = default;

        then& operator=(const then&) = delete;
        then& operator=(then&&) = delete;

        constexpr then(_F1 && f1_value, _F2 && f2_value)
        : f1(std::forward<_F1>(f1_value))
        , f2(std::forward<_F2>(f2_value))
        {}

        //explicit template arguments, forwardable
        // TODO : noexcept
        template <typename ... f1_ts, typename ... f1_args_t> // auto ... f1_vs, 
        requires 
        requires {
            functional::invoke<_F2>(
                std::declval<_F2&&>(),
                functional::invoke<_F1, f1_ts...>(std::declval<_F1&&>(), std::declval<f1_args_t&&>()...)
            );
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const
        // noexcept(
        //     std::is_nothrow_invocable_v<decltype(&F1::template operator()<f1_ts..., f1_args_t&&...>), F1, f1_args_t&&...> and
        //     std::is_nothrow_invocable_v<F2, std::invoke_result_t<F1, decltype(std::forward<decltype(f1_args_v)>(f1_args_v))...>>
        // )
        {
            return functional::invoke<_F2>(
                static_cast<_F2&&>(f2), 
                functional::invoke<_F1, f1_ts..., f1_args_t...>(static_cast<_F1&&>(f1), std::forward<f1_args_t>(f1_args_v)...));
        }
        
        // explicit template, not forwardable
        // TODO : noexcept, warning on discard
        template <typename ... f1_ts, typename ... f1_args_t>
        requires
        requires {
            functional::invoke<_F1, f1_ts...>(std::declval<_F1&&>(), std::declval<f1_args_t&&>()...);
            functional::invoke<_F2>(std::declval<_F2&&>());
        }
        and (not requires (_F2 && arg2, _F1 && arg1){
            functional::invoke<_F2>(
                std::forward<_F2>(arg2),
                42
            );
        })
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const
        {   // discard _F1 invocation return value (TODO : warning)
            functional::invoke<_F1, f1_ts..., f1_args_t...>(static_cast<_F1&&>(f1), std::forward<f1_args_t>(f1_args_v)...);
            return functional::invoke(f2);
        }

    private:
        mutable _F1 f1;
        mutable _F2 f2;
    };
    template <typename F1, typename F2>
    then(F1 &&, F2 &&) -> then<std::remove_cvref_t<F1>, std::remove_cvref_t<F2>>;

    template <typename F>
    requires (
        std::move_constructible<F> and
        not std::is_reference_v<F> and
        not std::is_const_v<F>
    )
    struct rt_repeater {
        rt_repeater() = delete;
        constexpr rt_repeater(F && arg, std::size_t count)
        : _f{std::forward<F>(arg)}
        , call_count{count}
        {}
        constexpr rt_repeater(rt_repeater&&) = default;
        rt_repeater(const rt_repeater&) = delete;

        constexpr rt_repeater& operator=(rt_repeater&&) = default;
        rt_repeater& operator=(const rt_repeater&) = delete;

        template <typename ... f_ts, typename ... f_args_t>
        requires requires (F &&f){
            functional::invoke<F, f_ts...>(std::forward<F>(f), std::declval<f_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f_args_t&&... f_args) const {

            static_assert(not std::is_const_v<decltype(_f)>);
            static_assert(not std::is_reference_v<decltype(_f)>);

            using f_invoke_result_t = std::remove_cvref_t<
                decltype(functional::invoke<F, f_ts...>(std::declval<F>(), std::declval<f_args_t>()...))
            >;

            if constexpr (std::is_same_v<void, f_invoke_result_t>)
            {
                //for (auto count : std::ranges::iota_view<std::size_t>{0, call_count}) // Clang 2.0 issue
                for (auto count = 0; count < call_count; ++count)
                    functional::invoke<F, f_ts...>(static_cast<F&&>(_f), std::forward<f_args_t>(f_args)...);
                ;
            }
            else return [&]() constexpr {
                static_assert(std::semiregular<f_invoke_result_t>);
                using return_type = std::vector<f_invoke_result_t>;
                return_type return_value(call_count);
                std::generate(std::begin(return_value), std::end(return_value), [&](){
                    return functional::invoke<F, f_ts...>(static_cast<F&&>(_f), std::forward<f_args_t>(f_args)...);
                });
                return return_value;
            }();
        }

        const std::size_t call_count;
    private:
        mutable F _f;
    };
    template <typename F>
    rt_repeater(F&&, std::size_t) -> rt_repeater<std::remove_cvref_t<F>>;
}
namespace workflow::operators {

    // todo : operator / operator&
    // todo : tuple interface -> tuple, pair, array
    //        destructured bindings
    //  special then to wrap such thing ?
    //  [](){ return std::tuple{'a', 42};} >>= workflow::unfold >>= ([](char){} & [](int){})
    //  that internally call std::apply based on overload resolution
    //
    // ([](){ return 'a';} + [](){ return 42;}) >>= workflow::unfold >>= ([](char){} & [](int){})

    template <
        typename F1, typename F2,
        typename std::enable_if_t<not type_traits::have_shift_equal_operator_v<F1, F2>> * = nullptr
    >   // should add more requirements, that detects template operator()
    requires
        std::move_constructible<F1> and
        std::move_constructible<F2> and
        (not std::is_const_v<std::remove_reference_t<F1>>) and
        (not std::is_const_v<std::remove_reference_t<F2>>)
    constexpr decltype(auto) operator>>=(F1 && f1_value, F2&& f2_value) noexcept {

        using _F1 = std::remove_cvref_t<F1>;
        using _F2 = std::remove_cvref_t<F2>;
        return then<_F1,_F2>{static_cast<_F1&&>(f1_value), static_cast<_F2&&>(f2_value)};
    }
    template <
        typename F1, typename F2,
        typename std::enable_if_t<not type_traits::have_pipe_operator_v<F1, F2>> * = nullptr
    >   // should add more requirements, that detects template operator()
    requires
        std::move_constructible<F1> and
        std::move_constructible<F2> and
        (not std::is_const_v<std::remove_reference_t<F1>>) and
        (not std::is_const_v<std::remove_reference_t<F2>>)
    constexpr decltype(auto) operator|(F1 && lhs, F2 && rhs) {
        
        using _F1 = std::remove_cvref_t<F1>;
        using _F2 = std::remove_cvref_t<F2>;
        return functional::overload<_F1, _F2>{
            std::forward<_F1>(lhs),
            std::forward<_F2>(rhs)
        };
    }
    template <
        typename F,
        typename std::enable_if_t<not type_traits::have_plus_operator_v<F, std::size_t>> * = nullptr
    >   // should add more requirements, that detects template operator()
    requires
        std::move_constructible<F> and
        (not std::is_const_v<std::remove_reference_t<F>>)
    constexpr decltype(auto) operator*(F && f_value, std::size_t call_count) noexcept {

        // static_assert(not std::is_const_v<F>);
        // static_assert(not std::is_reference_v<F>);

        return rt_repeater{ std::move(f_value), call_count };

        // return [
        //     _f = std::move(f_value),
        //     call_count
        // ]<typename ... f_ts>(auto &&... f_args) constexpr mutable -> decltype(auto)
        // requires requires (F &&f){
        //     functional::invoke<F, f_ts...>(std::forward<F>(f), std::declval<decltype(f_args)&&>()...);
        // }
        // {
        //     static_assert(not std::is_const_v<decltype(_f)>);
        //     static_assert(not std::is_reference_v<decltype(_f)>);

        //     using f_invoke_result_t = std::remove_cvref_t<
        //         decltype(functional::invoke(std::declval<decltype(_f)>(), std::declval<decltype(f_args)>()...))
        //     >;

        //     if constexpr (std::is_same_v<void, f_invoke_result_t>)
        //     {
        //         //for (auto count : std::ranges::iota_view<std::size_t>{0, call_count}) // Clang 2.0 issue
        //         for (auto count = 0; count < call_count; ++count)
        //             std::invoke(_f, std::forward<decltype(f_args)>(f_args)...);
        //         ;
        //     }
        //     else return [&]() constexpr {
        //         static_assert(std::semiregular<f_invoke_result_t>);
        //         using return_type = std::vector<f_invoke_result_t>;
        //         return_type return_value(call_count);
        //         std::generate(std::begin(return_value), std::end(return_value), [&](){
        //             return std::invoke(_f, std::forward<decltype(f_args)>(f_args)...);
        //         });
        //         return return_value;
        //     }();
        // };
    }
    template <
        typename F, auto call_count,
        typename std::enable_if_t<
            not type_traits::have_plus_operator_v<
                F,
                std::integral_constant<std::size_t, call_count>
            >
        > * = nullptr
    >   // should add more requirements, that detects template operator()
    requires
        std::move_constructible<F> and
        (not std::is_const_v<std::remove_reference_t<F>>)
    constexpr decltype(auto) operator*(F && f_value, std::integral_constant<decltype(call_count), call_count>) noexcept {

        // TODO : strong type instead of lambda
        using _F = F;
        return [
            _f = std::forward<_F>(f_value)
        ]<typename ... f_ts>(auto &&... f_args) constexpr mutable  -> decltype(auto)
        requires requires (_F && f){
            functional::invoke<_F, f_ts...>(std::forward<_F>(f), std::declval<decltype(f_args)&&>()...);
        }
        {
            using f_invoke_result_t = std::remove_cvref_t<
                decltype(functional::invoke<_F, f_ts...>(std::declval<_F>(), std::declval<decltype(f_args)>()...))
            >;

            if constexpr (std::is_same_v<void, f_invoke_result_t>)
                return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
                    (((void)indexes, std::invoke(static_cast<_F&&>(_f), std::forward<decltype(f_args)>(f_args)...)), ...);
                }(std::make_index_sequence<call_count>{});
            else
                return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
                    using return_type = std::array<f_invoke_result_t, call_count>;
                    return return_type {
                        typename return_type::value_type{
                            ((void)indexes,
                            functional::invoke<_F, f_ts...>(static_cast<_F&&>(_f), std::forward<decltype(f_args)>(f_args)...))
                        }...
                    };
                }(std::make_index_sequence<call_count>{});
        };
    }
}

#ifdef CPP_SHELVE_STANDALONE_EDIT__
#include <memory>
auto main() -> int {

    using namespace workflow::operators;

    // operator>>=
    {
        {
            auto route = [](){
                std::cout << "then 1\n";
                return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
            } >>= [](auto value){
                std::cout << "then 2 (value : " << *value << ")\n";
                return std::integral_constant<int, 42>{};
            };
            static_assert(std::is_invocable_r_v<std::integral_constant<int, 42>, decltype(route)>);
        }
        {
            char filler = 'X';
            auto route = [&filler](int value){
                std::cout << "then 1\n";
                return std::make_unique<std::string>(value, filler);
            } >>= [](auto value){
                std::cout << "then 2 (value : " << *value << ")\n";
            };
            static_assert(std::is_invocable_v<decltype(route), int>);
        }
        {
            auto route = [](){
                std::cout << "then 1\n";
                return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
            } >>= [](auto value){
                std::cout << "then 2 (value : " << *value << ")\n";
            }  >>= [](){
                std::cout << "then 3\n";
                return std::integral_constant<int, 42>{};
            };
            static_assert(std::is_invocable_r_v<std::integral_constant<int, 42>, decltype(route)>);
        }
        {
            auto route = [](){
                std::cout << "then 1\n";
                return std::make_unique<std::string>("toto titi tata tutu qwe qwe qwe");
            } >>= [](auto && value){
                std::cout << "then 2 (value : " << *value << ")\n";
                return std::forward<decltype(value)>(value);
            }  >>= [](auto && value){
                std::cout << "then 3 (value : " << *value << ")\n";
                return value;
            };
            static_assert(std::is_invocable_r_v<std::unique_ptr<std::string>&&, decltype(route)>);
        }
        {   // TTP
            constexpr auto route_ttp_parameter = []<typename>(){} >>= [](){ return std::true_type{};};
            static_assert(std::is_same_v<
                decltype(route_ttp_parameter.template operator()<int>()),
                std::true_type
            >);
        }
        {   // NTTP : TODO
            // constexpr auto route_nttp_parameter = []<bool>(){} >>= [](){ return std::true_type{};};
            // static_assert(std::is_same_v<
            //     decltype(route_nttp_parameter.template operator()<true>()),
            //     std::true_type
            // >);
        }
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
                    std::cout << "then - T\n";
                    return std::true_type{};
                } |
                [](char){
                    std::cout << "then - char\n";
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
                std::cout << "then 1\n";
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

            static_assert(std::is_invocable_r_v<
                std::true_type,
                decltype(&decltype(route):: template operator()<std::true_type>),
                decltype(route)
            >);
            static_assert(std::is_same_v<
                decltype(route. template operator()<std::true_type>()),
                std::true_type
            >);
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
        {   // constexpr
            // using call_count_t = std::integral_constant<int, 3>;
            // constexpr auto route =
            //     [call_counter = 0]() mutable {  return ++call_counter; } *
            //     call_count_t{}
            // ;
            // static_assert(std::is_invocable_v<decltype(route)>);
            // static_assert(std::is_invocable_r_v<std::array<int, call_count_t::value>, decltype(route)>);
            // if (route() not_eq std::array{1,2,3})
            //     throw std::runtime_error{"operator*<F, std::size_t> : unexpected return values"};
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
        {   // TTP
            using call_count_t = std::integral_constant<int, 3>;
            auto route = []<typename switch_type>(){
                using return_type = std::conditional_t<switch_type::value, int, char>;
                return return_type{};
            } * call_count_t{};
            static_assert(std::is_same_v<
                decltype(route.template operator()<std::true_type>()),
                std::array<int, 3>
            >);
        }
        {   // NTTP

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
            // constexpr auto lhs = (
            //         [](std::string_view && value) constexpr -> std::string_view { return value; }
            //     |   [](auto && value) constexpr -> std::string_view
            //         requires requires {
            //             std::begin(value); 
            //             std::end(value);
            //         }
            //         { return std::string_view { std::begin(value), std::end(value) };}
            // );
            // static_assert(std::is_invocable_r_v<std::string_view, decltype(lhs), std::string>);
            // static_assert(std::is_invocable_r_v<std::string_view, decltype(lhs), std::vector<char>>);
            // static_assert(std::is_invocable_r_v<std::string_view, decltype(lhs), std::string_view>);

            // auto rhs = (
            //     [](auto && value) constexpr noexcept
            //     requires (std::same_as<std::decay_t<decltype(value)>, std::string_view>)
            //     {
            //         if (not value.empty())
            //             value.remove_prefix(std::min(value.find_first_of(" ") + 1, value.size()));
            //         return value;
            //     } * 3
            // );
            // static_assert(std::is_invocable_r_v<std::vector<std::string_view>, decltype(rhs), std::string_view&&>);
            
            // auto route = (lhs >>= rhs);
            // using namespace std::string_view_literals;
            
            // if (route("a b c d"sv) not_eq std::vector<std::string_view>{
            //     "b c d"sv,
            //     "c d"sv,
            //     "d"sv
            // })
            //    throw std::runtime_error{"(F1 >>= F2) * int : unexpected values"};
        }
        {   // constexpr nodes : TODO, dont violate constness
            // constexpr auto rhs = (
            //     [](auto && value) constexpr noexcept
            //     requires (std::same_as<std::decay_t<decltype(value)>, std::string_view>)
            //     {
            //         if (not value.empty())
            //             value.remove_prefix(std::min(value.find_first_of(" ") + 1, value.size()));
            //         return value;
            //     } * std::integral_constant<int, 3>{}
            // );
            // static_assert(std::is_invocable_v<decltype(rhs), std::string_view&&>);
            // static_assert(std::is_invocable_r_v<std::array<std::string_view, 3>, decltype(rhs), std::string_view&&>);

            // constexpr auto node_1 = [](){};
            // constexpr auto node_2 = [](){};
            // constexpr auto route = [
            //         f1 = std::move(node_1),
            //         f2 = std::move(node_2)
            // ]() constexpr  {
            //     f1();
            //     f2();
            // };
            // route();  
        }
        {
            struct functor {
                functor() = default;
                functor(functor&&) = default;
                functor(const functor&) = delete;
                auto operator()() const {
                    return 42;
                }
            };
            {
                auto route = functor{} * 3;
                static_assert(std::is_invocable_v<decltype(route)>);
            }
            {
                const auto route = functor{} * 3;
                static_assert(std::is_invocable_v<decltype(route)>);
            }
        }
        {
            auto lhs = [](){};
            auto route = std::move(lhs) * 3;
            static_assert(std::is_invocable_v<decltype(route)>);
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