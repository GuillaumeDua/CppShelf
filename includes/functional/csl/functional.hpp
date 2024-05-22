#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

namespace csl::functional::details::type_traits {
    template <typename T>
    struct remove_mem_func_pointer;
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...)> : std::type_identity<R(Args...)>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) noexcept> : std::type_identity<R(Args...) noexcept>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const> : std::type_identity<R(Args...) const>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const noexcept> : std::type_identity<R(Args...) const noexcept>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) volatile> : std::type_identity<R(Args...) volatile>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) volatile noexcept> : std::type_identity<R(Args...) volatile noexcept>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const volatile> : std::type_identity<R(Args...) const volatile>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const volatile noexcept> : std::type_identity<R(Args...) const volatile noexcept>{};
    template <typename T>
    using remove_mem_func_pointer_t = typename remove_mem_func_pointer<T>::type;
}
namespace csl::functional {

    // arguments
    template <typename ...> struct arguments{};

    // overload
    template <typename ...> struct overload_bases{};
    template<class... Ts> struct overload : Ts... {
        using Ts::operator()...;
        using base_types = overload_bases<Ts...>;
    };
    template<class... Ts> overload(Ts...) -> overload<Ts...>;

    // function_trait
    template <typename T> 
    struct function_trait;
    template <typename T>
    requires requires { &std::remove_cvref_t<T>::operator(); }
    struct function_trait<T> : function_trait<
        details::type_traits::remove_mem_func_pointer_t<decltype(&std::remove_cvref_t<T>::operator())>
    >{};

    namespace details {
        // helper
        template<typename R, typename... Args>
        struct function_trait_impl {
            using result_type = R;
            using arguments_type = arguments<Args...>;

            // TODO(): is_const, is_volatile, is_noexcept
            // TODO(): is_fun, is_memfun
        };
    }

    // member-functions
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...)>                         : details::function_trait_impl<R, C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) noexcept>                : details::function_trait_impl<R, C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const>                   : details::function_trait_impl<R, const C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const noexcept>          : details::function_trait_impl<R, const C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) volatile>                : details::function_trait_impl<R, volatile C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) volatile noexcept>       : details::function_trait_impl<R, volatile C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const volatile>          : details::function_trait_impl<R, const volatile C, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const volatile noexcept> : details::function_trait_impl<R, const volatile C, Args...>{};

    // function
    template <typename R, typename... Args> struct function_trait<R(Args...)>                                           : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) noexcept>                                  : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) const>                                     : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) const noexcept>                            : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) volatile>                                  : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) volatile noexcept>                         : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) const volatile>                            : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(Args...) const volatile noexcept>                   : details::function_trait_impl<R, Args...>{};

    // functions ptr
    template <typename R, typename... Args> struct function_trait<R(*)(Args...)>                                        : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<R(*)(Args...) noexcept>                               : details::function_trait_impl<R, Args...>{};

    // std::function
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...)>>                            : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) noexcept>>                   : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) const>>                      : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) const noexcept>>             : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) volatile>>                   : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) volatile noexcept>>          : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) const volatile>>             : details::function_trait_impl<R, Args...>{};
    template <typename R, typename... Args> struct function_trait<std::function<R(Args...) const volatile noexcept>>    : details::function_trait_impl<R, Args...>{};

    template <typename F>
    using function_trait_result_t = typename function_trait<F>::result_type;
    template <typename F>
    using function_trait_arguments_t = typename function_trait<F>::arguments_type;

    // overload trait
    template <typename T>
    struct overload_trait;
    template <typename... Ts>
    struct overload_trait<csl::functional::overload<Ts...>> : std::type_identity<
        std::tuple<function_trait<Ts>...>
    >{};
    template <typename T>
    using overload_trait_t = typename overload_trait<T>::type;

    // overload arguments
    template <typename T>
    struct overload_arguments;
    template <typename... Ts>
    struct overload_arguments<csl::functional::overload<Ts...>> : std::type_identity<
        std::tuple<typename function_trait<Ts>::arguments_type...>
    >{};
    template <typename T>
    using overload_arguments_t = typename overload_arguments<T>::type;

    // overload arguments
    template <typename T>
    struct overload_result;
    template <typename... Ts>
    struct overload_result<csl::functional::overload<Ts...>> : std::type_identity<
        std::tuple<typename function_trait<Ts>::result_type...>
    >{};
    template <typename T>
    using overload_result_t = typename overload_result<T>::type;
}
namespace csl::functional::type_traits {

    // is_invocable
    template <typename F, typename arguments_type>
    struct is_invocable : std::false_type{};
    template <typename F, typename ... Ts>
    struct is_invocable<F, arguments<Ts...>> : std::is_invocable<F, Ts...>{};
    template <typename T, typename arguments_type>
    constexpr bool is_invocable_v = is_invocable<T, arguments_type>::value;

    // is_nothrow_invocable
    template <typename F, typename arguments_type>
    struct is_nothrow_invocable : std::false_type{};
    template <typename F, typename ... Ts>
    struct is_nothrow_invocable<F, arguments<Ts...>> : std::is_nothrow_invocable<F, Ts...>{};
    template <typename T, typename arguments_type>
    constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<T, arguments_type>::value;

    // is_invocable_r
    template <typename R, typename F, typename arguments_type>
    struct is_invocable_r : std::false_type{};
    template <typename R, typename F, typename ... Ts>
    struct is_invocable_r<R, F, arguments<Ts...>> : std::is_invocable_r<R, F, Ts...>{};
    template <typename R, typename F, typename arguments_type>
    constexpr bool is_invocable_r_v = is_invocable_r<R, F, arguments_type>::value;

    // is_nothrow_invocable_r
    template <typename R, typename F, typename arguments_type>
    struct is_nothrow_invocable_r : std::false_type{};
    template <typename R, typename F, typename ... Ts>
    struct is_nothrow_invocable_r<R, F, arguments<Ts...>> : std::is_nothrow_invocable_r<R, F, Ts...>{};
    template <typename R, typename F, typename arguments_type>
    constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, F, arguments_type>::value;

    // is_simple_callable: one invocation candidate
    template <typename F, class = void>
    struct is_simple_callable : std::false_type{};
    template <typename F>
    struct is_simple_callable<
        F,
        std::void_t<typename csl::functional::function_trait<F>::arguments_type>
    > : std::true_type{};
    template <typename F>
    constexpr bool is_simple_callable_v = is_simple_callable<F>::value;
}
namespace csl::functional::concepts {
    // invocable_with
    template <typename F, typename arguments_type>
    concept invocable_with = type_traits::is_invocable_v<F, arguments_type>;

    // nothrow_invocable_with
    template <typename F, typename arguments_type>
    concept nothrow_invocable_with = type_traits::is_nothrow_invocable_v<F, arguments_type>;

    template <typename F>
    concept simple_callable = type_traits::is_simple_callable_v<F>;
}
