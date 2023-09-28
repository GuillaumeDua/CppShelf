#pragma once

#if not __cplusplus >= 201703L
# error "csl/functional.hpp requires C++20"
#endif

#include <functional>
#include <tuple>
#include <type_traits>

namespace csl::functional::details::mp {
#if defined(__cpp_lib_type_identity)
    template <typename T>
    using type_identity = typename std::type_identity<T>;
#else
    template <typename T>
    struct type_identity{ using type = T; };
#endif
    template <typename ...>
    constexpr auto dependent_false_v = false;
}

namespace csl::functional::details::type_traits {
    template <typename T>
    struct remove_mem_func_pointer;
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...)> : details::mp::type_identity<R(Args...)>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) noexcept> : details::mp::type_identity<R(Args...) noexcept>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const> : details::mp::type_identity<R(Args...) const>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const noexcept> : details::mp::type_identity<R(Args...) const noexcept>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) volatile> : details::mp::type_identity<R(Args...) volatile>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) volatile noexcept> : details::mp::type_identity<R(Args...) volatile noexcept>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const volatile> : details::mp::type_identity<R(Args...) const volatile>{};
    template <typename R, typename C, typename ... Args>
    struct remove_mem_func_pointer<R(C::*)(Args...) const volatile noexcept> : details::mp::type_identity<R(Args...) const volatile noexcept>{};
    template <typename T>
    using remove_mem_func_pointer_t = typename remove_mem_func_pointer<T>::type;
}
namespace csl::functional::details::concepts {
#if __cplusplus < 202002L
    // C++17
    template<typename, typename = void>
    constexpr bool has_call_operator{};
    template<typename T>
    constexpr bool has_call_operator<
        T,
        std::void_t<decltype(&std::decay_t<T>::operator())>
    > = true;
#else
    template <typename T>
    concept has_call_operator = requires { &std::remove_cvref_t<T>::operator(); };
#endif
}
namespace csl::functional {

    // arguments
    template <typename ... Ts> using arguments = std::tuple<Ts...>;

    // overload
    template <typename ...> struct overload_bases{};
    template <class... Ts> struct overload : Ts... {
        using Ts::operator()...;
        using base_types = overload_bases<Ts...>;
    };
    template <class... Ts> overload(Ts...) -> overload<Ts...>;

    // function_trait
    // function_trait
    template <typename T> 
    struct function_trait;
#if __cplusplus >= 202002L
    template <typename T>
    requires details::concepts::has_call_operator<T>
    struct function_trait<T> : function_trait<
        details::type_traits::remove_mem_func_pointer_t<decltype(&std::remove_cvref_t<T>::operator())>
    >{};
#else
    template <typename T>
    struct function_trait
    : function_trait<
        details::type_traits::remove_mem_func_pointer_t<decltype(&std::decay_t<T>::operator())>
    >{
        static_assert(details::concepts::has_call_operator<T>);
    };
#endif

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
    struct overload_trait<csl::functional::overload<Ts...>> : details::mp::type_identity<
        std::tuple<function_trait<Ts>...>
    >{};
    template <typename T>
    using overload_trait_t = typename overload_trait<T>::type;

    // overload arguments
    template <typename T>
    struct overload_arguments;
    template <typename... Ts>
    struct overload_arguments<csl::functional::overload<Ts...>> : details::mp::type_identity<
        std::tuple<typename function_trait<Ts>::arguments_type...>
    >{};
    template <typename T>
    using overload_arguments_t = typename overload_arguments<T>::type;

    // overload arguments
    template <typename T>
    struct overload_result;
    template <typename... Ts>
    struct overload_result<csl::functional::overload<Ts...>> : details::mp::type_identity<
        std::tuple<typename function_trait<Ts>::result_type...>
    >{};
    template <typename T>
    using overload_result_t = typename overload_result<T>::type;
}
namespace csl::functional::type_traits {

    // is_invocable
    template <typename F, typename arguments_type>
    struct is_invocable {
        static_assert(details::mp::dependent_false_v<F, arguments_type>, "csl::functional::type_traits::is_invocable: usage: is_invocable<F, arguments<Ts...>>");
    };
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
#if __cplusplus >= 202002L
    // invocable_with
    template <typename F, typename arguments_type>
    concept invocable_with = type_traits::is_invocable_v<F, arguments_type>;

    // nothrow_invocable_with
    template <typename F, typename arguments_type>
    concept nothrow_invocable_with = type_traits::is_nothrow_invocable_v<F, arguments_type>;

    template <typename F>
    concept simple_callable = type_traits::is_simple_callable_v<F>;
#else
    template <typename F, typename arguments_type>
    constexpr auto invocable_with = type_traits::is_invocable_v<F, arguments_type>;

    // nothrow_invocable_with
    template <typename F, typename arguments_type>
    constexpr auto nothrow_invocable_with = type_traits::is_nothrow_invocable_v<F, arguments_type>;

    template <typename F>
    constexpr auto simple_callable = type_traits::is_simple_callable_v<F>;
#endif
}
