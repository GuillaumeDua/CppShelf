#pragma once

#include <functional>
#include <tuple>
#include <type_traits>

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
    requires std::is_member_function_pointer_v<T>
    struct function_trait<T> : function_trait<decltype(&std::remove_cvref_t<T>::operator())>{};

    template <typename T> 
    requires std::is_function_v<T>
    struct function_trait<T> : function_trait<decltype(&std::remove_cvref_t<T>::operator())>{};

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
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...)>                         : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) noexcept>                : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const>                   : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const noexcept>          : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) volatile>                : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) volatile noexcept>       : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const volatile>          : details::function_trait_impl<R, Args...>{};
    template <typename R, typename C, typename... Args>  struct function_trait<R(C::*)(Args...) const volatile noexcept> : details::function_trait_impl<R, Args...>{};

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
}
namespace csl::functional::type_traits {

    // overload arguments
    template <typename T>
    struct overload_arguments;
    template <typename... Ts>
    struct overload_arguments<csl::functional::overload<Ts...>> {
        using types = std::tuple<
            typename function_trait<decltype(&Ts::operator())>::arguments_type...
        >;
    };
    template <typename T>
    using overload_arguments_t = typename overload_arguments<T>::types;

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
}
namespace csl::functional::concepts {
    // invocable_with
    template <typename F, typename arguments_type>
    concept invocable_with = type_traits::is_invocable_v<F, arguments_type>;

    // nothrow_invocable_with
    template <typename F, typename arguments_type>
    concept nothrow_invocable_with = type_traits::is_nothrow_invocable_v<F, arguments_type>;
}
