#pragma once

// Evolutions :
//  Consider P2347 "Argument type deduction for non-trailing parameter packs" to deduce a single non-trailing parameters pack for function arguments

#include <utility>
#include <type_traits>
#include <variant>
#include <concepts>
#include <functional>
#include <ranges>
#include <vector>
#include <array>
#include <string_view>
#include <tuple>

// -- dev/wip/debug only
namespace gcl::cx {
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
// --

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)

// todo : poc a cleaner design ?
//  details::apply that is unsafe but hidden
//  apply safe, part of the API
//  applyable as requires { apply }
//  nothrow_applyable as requires { apply } noexcept
// however, STL's invocable is requires { std::invoke(...) }
//  https://en.cppreference.com/w/cpp/concepts/invocable

// is_(nothrow_)invocable(_r), invoke_result
// is_(nothrow_)applyable(_before|_after)
namespace csl::wf::mp {

    // ttps -> pack of ttps
    template <typename ...>
    struct ttps{};

    template <typename T>
    constexpr bool is_ttps_v = false;
    template <typename ... Ts>
    constexpr bool is_ttps_v<ttps<Ts...>> = true;

    // is_invocable<F, [ttps<...>,] args_types...>
    template <typename F, typename... args_types>
    struct is_invocable : std::is_invocable<F, args_types...>{};
    template <typename F, typename ... args_types>
    struct is_invocable<F, ttps<>, args_types...> : is_invocable<F, args_types...>{};
    template <typename F, typename ... ttps_args, typename ... args_types>
    struct is_invocable<F, ttps<ttps_args...>, args_types...> {
        constexpr static bool value = requires{
            std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...);
        };
    };
    template <typename F, typename... Ts>
    constexpr bool is_invocable_v = is_invocable<F, Ts...>::value;

    // is_nothrow_invocable<F, [ttps<...>,] args_types...>
    template <typename F, typename... args_types>
    struct is_nothrow_invocable : std::is_nothrow_invocable<F, args_types...>{};
    template <typename F, typename ... args_types>
    struct is_nothrow_invocable<F, ttps<>, args_types...> : is_nothrow_invocable<F, args_types...>{};
    template <typename F, typename ... ttps_args, typename ... args_types>
    struct is_nothrow_invocable<F, ttps<ttps_args...>, args_types...> {
        constexpr static bool value = []() constexpr {
            if constexpr (requires {
                std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...);
            }) return noexcept(
                std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...)
            );
            else return false;
        }();
         
    };
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_invocable_v = is_nothrow_invocable<F, Ts...>::value;

    // is_invocable_r<R, F, [ttps<...>,] args_types...>
    template <typename R, typename F, typename... args_types>
    struct is_invocable_r : std::is_invocable_r<R, F, args_types...>{};
    template <typename R, typename F, typename ... args_types>
    struct is_invocable_r<R, F, ttps<>, args_types...> : is_invocable_r<R, F, args_types...>{};
    template <typename R, typename F, typename ... ttps_args, typename ... args_types>
    struct is_invocable_r<R, F, ttps<ttps_args...>, args_types...> {
        constexpr static bool value = requires{
            { std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...) } -> std::convertible_to<R>;
        };
    };
    template <typename R, typename F, typename... Ts>
    constexpr bool is_invocable_r_v = is_invocable_r<R, F, Ts...>::value;

    // is_nothrow_invocable_r<F, [ttps<...>,] args_types...>
    template <typename R, typename F, typename... args_types>
    struct is_nothrow_invocable_r : std::is_nothrow_invocable_r<R, F, args_types...>{};
    template <typename R, typename F, typename ... args_types>
    struct is_nothrow_invocable_r<R, F, ttps<>, args_types...> : is_nothrow_invocable_r<R, F, args_types...>{};
    template <typename R, typename F, typename ... ttps_args, typename ... args_types>
    struct is_nothrow_invocable_r<R, F, ttps<ttps_args...>, args_types...> {
        constexpr static bool value = []() constexpr {
            if constexpr (requires{
                { std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...) } -> std::convertible_to<R>;
            }) return noexcept(
                std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...)
            );
            else return false;
        }();
         
    };
    template <typename R, typename F, typename... Ts>
    constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, F, Ts...>::value;

    // invoke_result
    template <typename F, typename... args_types>
    struct invoke_result : std::invoke_result<F, args_types...>{};
    template <typename F, typename ... args_types>
    struct invoke_result<F, ttps<>, args_types...> : invoke_result<F, args_types...>{};
    template <typename F, typename ... ttps_args, typename ... args_types>
    struct invoke_result<F, ttps<ttps_args...>, args_types...> {
        using type = decltype(
            std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...)
        );
    };
    template <typename F, typename... Ts>
    using invoke_result_t = invoke_result<F, Ts...>::type;

    // ---

    template <typename T>
    concept tuple_interface = requires { std::tuple_size<std::remove_reference_t<T>>{}; };

    // is_applyable
    template <typename F, typename...>
    struct is_applyable //: std::false_type{};
    {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename F, typename ... ttps_args, tuple_interface tuple_type>
    struct is_applyable<F, ttps<ttps_args...>, tuple_type> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, ttps<ttps_args...>, decltype(std::get<indexes>(std::declval<tuple_type>()))...>; // std::get to preserve cvref qualifiers
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, tuple_interface tuple_type>
    struct is_applyable<F, tuple_type> : is_applyable<F, ttps<>, tuple_type>{};
    template <typename F, typename... Ts>
    constexpr bool is_applyable_v = is_applyable<F, Ts...>::value;

    // is_nothrow_applyable
    template <typename F, typename...>
    struct is_nothrow_applyable {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename F, typename ... ttps_args, tuple_interface tuple_type>
    struct is_nothrow_applyable<F, ttps<ttps_args...>, tuple_type> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, ttps<ttps_args...>, decltype(std::get<indexes>(std::declval<tuple_type>()))...>; // std::get to preserve cvref qualifiers
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, tuple_interface tuple_type>
    struct is_nothrow_applyable<F, tuple_type> : is_nothrow_applyable<F, ttps<>, tuple_type>{};
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_applyable_v = is_nothrow_applyable<F, Ts...>::value;

    // is_applyable_before
    template <typename F, typename...>
    struct is_applyable_before //: std::false_type{};
    {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_before<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, ttps<f_ts...>, decltype(std::get<indexes>(std::declval<tuple_type>()))..., func_args_t...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_before<F, tuple_type, func_args_t...>
    : is_applyable_before<F, ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_applyable_before_v = is_applyable_before<F, Ts...>::value;

    // is_nothrow_applyable_before
    template <typename F, typename...>
    struct is_nothrow_applyable_before {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_before<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, ttps<f_ts...>, decltype(std::get<indexes>(std::declval<tuple_type>()))..., func_args_t...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_before<F, tuple_type, func_args_t...>
    : is_nothrow_applyable_before<F, mp::ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_applyable_before_v = is_nothrow_applyable_before<F, Ts...>::value;

    // is_applyable_after
    template <typename F, typename...>
    struct is_applyable_after {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_after<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, ttps<f_ts...>, func_args_t..., decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_after<F, tuple_type, func_args_t...>
    : is_applyable_after<F, mp::ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_applyable_after_v = is_applyable_after<F, Ts...>::value;

    // is_nothrow_applyable_after
    template <typename F, typename...>
    struct is_nothrow_applyable_after {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_after<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, ttps<f_ts...>, func_args_t..., decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_after<F, tuple_type, func_args_t...>
    : is_nothrow_applyable_after<F, mp::ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_applyable_after_v = is_nothrow_applyable_after<F, Ts...>::value;
}
// is(_nothrow_)invocable_with
namespace csl::wf::mp {
    // Extension to handle both `ttps` and `args` as pack
    // more convenient for pack_traits - like filters - applications
    //
    // Less restrictive than `is_applyable`, as `args` does not match TupleInterface.
    // Also, easier to handle types that does not fit in std::tuple, array and paires, like `void`.

    template <typename ...>
    struct args{};

    // invocable_with<F, ttps<...>, args<...>>
    template <typename F, typename...>
    struct is_invocable_with{
        static_assert([](){ return false; }(), "parameters must be wrapped in `ttps` and `args`");
    };
    template <typename F, typename ... ttps_args, typename ... args_types>
    struct is_invocable_with<F, ttps<ttps_args...>, args<args_types...>>
    : is_invocable<F, ttps<ttps_args...>, args_types...>{};
    template <typename F, typename ttps, typename args>
    constexpr bool is_invocable_with_v = is_invocable_with<F, ttps, args>::value;
    template <typename F, typename ttps, typename args>
    concept invocable_with = is_invocable_with_v<F, ttps, args>;

    // nothrow_invocable_with<F, ttps<...>, args<...>>
    template <typename F, typename...>
    struct is_nothrow_invocable_with{
        static_assert([](){ return false; }(), "parameters must be wrapped in `ttps` and `args`");
    };
    template <typename F, typename ... ttps_args, typename ... args_types>
    struct is_nothrow_invocable_with<F, ttps<ttps_args...>, args<args_types...>>
    : is_nothrow_invocable<F, ttps<ttps_args...>, args_types...>{};
    template <typename F, typename ttps, typename args>
    constexpr bool is_nothrow_invocable_with_v = is_nothrow_invocable_with<F, ttps, args>::value;
    template <typename F, typename ttps, typename args>
    concept nothrow_invocable_with = is_nothrow_invocable_with_v<F, ttps, args>;
}
namespace csl::wf::details::mp {

    template <typename T>
    struct empty_if_void {
        static_assert([](){ return false; }(), "parameter must be a parameter-pack");
    };
    template <template <typename...> typename pack_type, typename ... Ts>
    struct empty_if_void<pack_type<Ts...>> {
        using type = pack_type<Ts...>;
    };
    template <template <typename...> typename pack_type>
    struct empty_if_void<pack_type<void>> {
        using type = pack_type<>;
    };
    template <typename T>
    using empty_if_void_t = empty_if_void<T>::type;

    template <typename T, typename ... Ts>
    constexpr bool are_unique_v = (not (std::is_same_v<T, Ts> or ...)) and are_unique_v<Ts...>;
    template <typename T>
    constexpr bool are_unique_v<T> = true;

    template <typename T>
    concept bindable = 
        std::is_constructible_v<std::decay_t<T>, T>
        and std::is_move_constructible_v<std::decay_t<T>>
    ;
}
// apply(,_after,_before), invoke
// front_binder, bind_front
namespace csl::wf {
    // todo : Universal template declaration ... (p1985)
    //  ttps -> tt{1,}ps + ntt{1,}ps 

    // invoke
    template <typename ... ttps_args, typename F, typename ... args_types>
    requires
        mp::is_invocable_v<F&&, mp::ttps<ttps_args...>, args_types&&...>
    constexpr decltype(auto) invoke(F && f, args_types&& ... args)
    noexcept(mp::is_nothrow_invocable_v<F&&, mp::ttps<ttps_args...>, args_types&&...>)
    {
        if constexpr (sizeof...(ttps_args) == 0)
            return std::invoke(std::forward<F>(f), std::forward<args_types>(args)...);
        else return std::forward<F>(f).template operator()<ttps_args...>(std::forward<args_types>(args)...);
    }
    template <typename F, typename ... args_types>
    requires
        (not mp::is_invocable_v<F&&, args_types&&...>)
        and std::invocable<F&&, args_types...>
    constexpr decltype(auto) invoke(F && f, args_types&& ... args)
    noexcept (std::is_nothrow_invocable_v<F&&, args_types&&...>)
    {
        return std::invoke(std::forward<F>(f), std::forward<args_types>(args)...);
    }

    // apply
    template <typename ... f_ts, typename F, mp::tuple_interface args_as_tuple_t>
    requires mp::is_applyable_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t>
    constexpr decltype(auto) apply(F && f, args_as_tuple_t&& args)
    noexcept(mp::is_nothrow_applyable_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_invocable_v<F&&, mp::ttps<f_ts...>, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), std::get<indexes>(std::forward<args_as_tuple_t>(args))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }

    // apply_before
    template <typename ... f_ts, typename F, mp::tuple_interface args_as_tuple_t, typename ... func_args_t>
    requires mp::is_applyable_before_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>
    constexpr decltype(auto) apply_before(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args)
    noexcept(mp::is_nothrow_applyable_before_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_applyable_before_v<F, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), fwd(std::get<indexes>(fwd(args)))..., fwd(func_args)...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }

    // apply_after
    template <typename ... f_ts, typename F, mp::tuple_interface args_as_tuple_t, typename ... func_args_t>
    requires mp::is_applyable_after_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>
    constexpr decltype(auto) apply_after(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args)
    noexcept(mp::is_nothrow_applyable_after_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_applyable_after_v<F, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), fwd(std::get<indexes>(fwd(args)))..., fwd(func_args)...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }

    // front_binder
    //  todo :  copy, move constructors
    //          operator=
    //          operator==, operator not_eq
    //  todo :  member-variables binding ?
    template <
        details::mp::bindable F,
        typename ttps_pack_type,
        details::mp::bindable ... bounded_args_t
    >
    class front_binder;
    template <
        details::mp::bindable F,
        typename ... ttps_bounded_args_t,
        details::mp::bindable ... bounded_args_t
    >
    class front_binder<F, mp::ttps<ttps_bounded_args_t...>, bounded_args_t...> {

        using type = front_binder<F, mp::ttps<ttps_bounded_args_t...>, bounded_args_t...>;

        using bounded_args_storage_type = std::tuple<bounded_args_t...>;
        bounded_args_storage_type bounded_arguments;
        F f;

    public:
        constexpr front_binder(front_binder &&) = default;

        constexpr front_binder(auto && f_arg, mp::ttps<ttps_bounded_args_t...>, auto && ... args)
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}
        constexpr front_binder(auto && f_arg, auto && ... args)
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {
            static_assert(sizeof...(ttps_bounded_args_t) == 0);
        }

        template <typename ... ttps, typename ... parameters_t>
        requires mp::is_applyable_before_v<
            F&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            bounded_args_storage_type&, parameters_t&&...
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) &
        noexcept(mp::is_nothrow_applyable_before_v<
            F&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            bounded_args_storage_type&, parameters_t&&...
        >)
        {
            return apply_before<ttps_bounded_args_t..., ttps...>(f, bounded_arguments, std::forward<decltype(parameters)>(parameters)...);
        }

        template <typename ... ttps, typename ... parameters_t>
        requires mp::is_applyable_before_v<
            const F&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            const bounded_args_storage_type&, parameters_t&&...
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) const &
        noexcept (mp::is_nothrow_applyable_before_v<
            const F&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            const bounded_args_storage_type&, parameters_t&&...
        >)
        {
            return apply_before<ttps_bounded_args_t..., ttps...>(f, bounded_arguments, std::forward<decltype(parameters)>(parameters)...);
        }

        template <typename ... ttps, typename ... parameters_t>
        requires mp::is_applyable_before_v<
            F&&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            bounded_args_storage_type&&, parameters_t&&...
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) &&
        noexcept (mp::is_nothrow_applyable_before_v<
            F&&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            bounded_args_storage_type&&, parameters_t&&...
        >) {
            return apply_before<ttps_bounded_args_t..., ttps...>(std::move(f), std::move(bounded_arguments), std::forward<decltype(parameters)>(parameters)...);
        }

        template <typename ... ttps, typename ... parameters_t>
        requires mp::is_applyable_before_v<
            const F&&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            const bounded_args_storage_type&&, parameters_t&&...
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) const &&
        noexcept(mp::is_nothrow_applyable_before_v<
            const F&&,
            mp::ttps<ttps_bounded_args_t..., ttps...>,
            const bounded_args_storage_type&&, parameters_t&&...
        >) {
            return apply_before<ttps_bounded_args_t..., ttps...>(std::move(f), std::move(bounded_arguments), std::forward<decltype(parameters)>(parameters)...);
        }

        // template <typename ... ttps, typename ... parameters_t>
        // constexpr decltype(auto) operator()(parameters_t && ... parameters) {
        //     static_assert([](){ return false; }(), "front_binder::operator() : no overload candidates matched");
        // }
    };
    template <typename F, typename ... ttps_bounded_args_t, typename ... bounded_args_t>
    front_binder(F&&, mp::ttps<ttps_bounded_args_t...>, bounded_args_t&&...) -> front_binder<F, mp::ttps<ttps_bounded_args_t...>, bounded_args_t...>;
    template <typename F, typename ... bounded_args_t>
    front_binder(F&&, bounded_args_t&&...) -> front_binder<F, mp::ttps<>, bounded_args_t...>;

    // binder_front
    //  same as `std::bind_front`, but also bound/allow ttps
    //  (waiting for proposal p1985 to extend this to nttps ...)
    template <typename ... ttps_bounded_args_t, typename F, typename ... args_t>
    constexpr auto bind_front(F&& f, args_t && ... args) {
        // front_binder factory.
        // produces the same behavior as std::bind_front (cvref-qualifiers correctness)
        using bind_front_t = front_binder<std::remove_cvref_t<F>, mp::ttps<ttps_bounded_args_t...>, std::remove_cvref_t<args_t>...>;
        return bind_front_t{
            std::forward<F>(f),
            mp::ttps<ttps_bounded_args_t...>{},
            std::forward<args_t>(args)...
        };
    }
}
// chain, packed_chain_trait
namespace csl::wf {
    template <typename ...>
    struct chain_trait;
    template <typename node, typename next, typename ...rest>
    requires (sizeof...(rest) not_eq 0)
    struct chain_trait<node, next, rest...> {

        template <typename ... args_type>
        constexpr static bool is_invocable = [](){
            if constexpr (not chain_trait<node, next>::template is_invocable<args_type...>)
                return false;
            else return chain_trait<rest...>::template is_invocable<
                typename chain_trait<node, next>::template invoke_result_t<args_type...>
            >;
        }();
        template <typename ... args_type>
        constexpr static bool is_nodiscard_invocable = [](){
            if constexpr (not chain_trait<node, next>::template is_nodiscard_invocable<args_type...>)
                return false;
            else return chain_trait<rest...>::template is_nodiscard_invocable<
                typename chain_trait<node, next>::template invoke_result_t<args_type...>
            >;
        }();
        template <typename ... args_type>
        constexpr static bool is_nothrow_invocable = [](){
            if constexpr (not chain_trait<node, next>::template is_nothrow_invocable<args_type...>)
                return false;
            else return chain_trait<rest...>::template is_nothrow_invocable<
                typename chain_trait<node, next>::template invoke_result_t<args_type...>
            >;
        }();
        template <typename ... args_type>
        constexpr static bool is_nothrow_nodiscard_invocable = [](){
            if constexpr (not chain_trait<node, next>::template is_nothrow_nodiscard_invocable<args_type...>)
                return false;
            else return chain_trait<rest...>::template is_nothrow_nodiscard_invocable<
                typename chain_trait<node, next>::template invoke_result_t<args_type...>
            >;
        }();

        template <typename ... args_type>
        requires
            is_invocable<args_type...> or
            is_invocable<>
        using invoke_result_t = chain_trait<rest...>::template invoke_result_t<
            typename chain_trait<node, next>::template invoke_result_t<args_type...>
        >;
    };
    template <typename node, typename next>
    struct chain_trait<node, next> {

        template <typename ... args_type>
        constexpr static bool is_invocable = [](){
            if constexpr (not chain_trait<node>::template is_invocable<args_type...>)
                return false;
            else return chain_trait<next>::template is_invocable<
                    typename chain_trait<node>::template invoke_result_t<args_type...>
                >
            ;
        }();
        template <typename ... args_type>
        constexpr static bool is_nodiscard_invocable = [](){
            if constexpr (not chain_trait<node>::template is_nodiscard_invocable<args_type...>)
                return false;
            else return chain_trait<next>::template is_nodiscard_invocable<
                typename chain_trait<node>::template invoke_result_t<args_type...>
            >;
        }();
        template <typename ... args_type>
        constexpr static bool is_nothrow_invocable = [](){
            if constexpr (not chain_trait<node>::template is_nothrow_invocable<args_type...>)
                return false;
            else return
                chain_trait<next>::template is_nothrow_invocable<
                    typename chain_trait<node>::template invoke_result_t<args_type...>
                >
            ;
        }();
        template <typename ... args_type>
        constexpr static bool is_nothrow_nodiscard_invocable = [](){

            if constexpr (not chain_trait<node>::template is_nothrow_nodiscard_invocable<args_type...>)
                return false;
            else return chain_trait<next>::template is_nothrow_nodiscard_invocable<
                typename chain_trait<node>::template invoke_result_t<args_type...>
            >;
        }();

        template <typename ... args_type>
        requires
            is_invocable<args_type...> or
            is_invocable<>
        using invoke_result_t = typename chain_trait<next>::template invoke_result_t<
            typename chain_trait<node>::template invoke_result_t<args_type...>
        >;
    };
    template <typename node>
    struct chain_trait<node> {

        template <typename ... args_type>
        constexpr static bool is_nodiscard_invocable =
            mp::is_invocable_v<node, args_type...>;
        template <typename ... args_type>
        constexpr static bool is_invocable =
            is_nodiscard_invocable<args_type...> or
            mp::is_invocable_v<node>
        ;
        template <typename ... args_type>
        constexpr static bool is_nothrow_nodiscard_invocable =
            mp::is_nothrow_invocable_v<node, args_type...>;
        template <typename ... args_type>
        constexpr static bool is_nothrow_invocable =
            is_nothrow_nodiscard_invocable<args_type...> or
            mp::is_nothrow_invocable_v<node>
        ;

        template <typename ... args_type>
        requires is_invocable<args_type...>
        using invoke_result_t = std::conditional_t<
            is_nodiscard_invocable<args_type...>,
            mp::invoke_result<node, args_type...>,
            mp::invoke_result<node>
        >::type;
    };
}
// route details
namespace csl::wf::details {
    // todo :
    //  - constraints using concepts
    //  - noexcept
    //  - TTPS, NTTPS
    // - multiples return values => csl::wf::mp::args<...> with tuple-like storage ? (or strong-type for std::tuple...)

    // apply_with_or_discard
    template <typename F, typename tuple_type>
    requires
        wf::mp::tuple_interface<tuple_type> and
        wf::mp::is_applyable_v<F&&, tuple_type&&>
    decltype(auto) apply_with_or_discard(F && functor, tuple_type && args_as_tuple)
    noexcept(wf::mp::is_nothrow_applyable_v<F&&, tuple_type&&>)
    {
        return wf::apply(fwd(functor), fwd(args_as_tuple));
    }
    template <typename F, typename tuple_type>
    requires
        wf::mp::tuple_interface<tuple_type> and
        (not wf::mp::is_applyable_v<F, tuple_type&&>)
    decltype(auto) apply_with_or_discard(F && functor, tuple_type && args_as_tuple)
    noexcept(wf::mp::is_nothrow_invocable_v<F&&>)
    {
        static_assert(wf::mp::is_invocable_v<F&&>);
        return wf::invoke(fwd(functor));
    }
    
    // todo : multiple returns value (including ttps<...>)
    decltype(auto) invoke_into_tuple(auto && functor, auto && ... args)
    noexcept (wf::mp::is_nothrow_invocable_v<decltype(functor), decltype(args)...>)
    requires wf::mp::is_invocable_v<decltype(functor), decltype(args)...>
    {   // not really tuple, but something that match TupleInterface
        using invoke_result_t = std::invoke_result_t<decltype(functor), decltype(args)...>;

        if constexpr (std::is_void_v<invoke_result_t>) {
            wf::invoke(fwd(functor), fwd(args)...);
            return std::tuple{}; // no return (void)
        }
        else // or use std::tuple to handle multiples return values here ?
            return std::array{wf::invoke(fwd(functor), fwd(args)...)};
    }
}
// route
// todo : value semantic correctness for route/binder storage
//  cvref qualifiers for binder::operator()
//  always owning ? mix owning and non-owning functors in storage ?
namespace csl::wf {
    // particular case : always nodiscard operator() ?
    // bind_front -> take care of template arguments

    /*
    template <typename ... Fs>
    struct route {
        using chain_trait_type = typename csl::wf::chain_trait<Fs...>;
        using storage_type = std::tuple<decltype(csl::wf::bind_front(std::declval<Fs&&>()))...>;

        route(Fs && ... functors)
        : storage{ fwd(functors)... }
        {}

        template <typename ... ttps_args, typename ... args_t>
        requires (chain_trait_type::template is_invocable<
            csl::wf::mp::ttps<ttps_args...>, args_t&&...
        >)
        decltype(auto) operator()(args_t && ... args)
        noexcept (chain_trait_type::template is_nothrow_invocable<
            csl::wf::mp::ttps<ttps_args...>, args_t&&...
        >)
        {
            // todo : requirement std::is_invocable get<0>(storage) ?
            auto storage_view = [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return std::forward_as_tuple(std::get<indexes>(storage)...);
            }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<storage_type>>>{});
            return invoke_impl<ttps_args...>(fwd(storage_view), fwd(args)...);
        }

    private:
        template <typename ... ttps_args_t, typename ... args_t>
        static decltype(auto) invoke_impl(auto && storage_view, args_t && ... args) {
            constexpr auto storage_view_size = std::tuple_size_v<std::remove_cvref_t<decltype(storage_view)>>;
            static_assert(storage_view_size not_eq 0);

            auto && head = std::get<0>(storage_view);
            // todo : maybe discard args
            auto return_value = csl::wf::invoke<ttps_args_t...>(fwd(head), fwd(args)...);

            // todo : transform if/else into require ?
            if constexpr (storage_view_size == 1) // head == tail
                return return_value;
            else
                return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                    return invoke_impl(
                        std::forward_as_tuple(std::get<(1 + indexes)>(storage_view)...),
                        return_value
                    );
                }(std::make_index_sequence<storage_view_size - 1>{});
        }

        storage_type storage;
    };
    template <>
    struct route <> {};

    template <typename ... Fs>
    route(Fs&& ...) -> route<Fs...>;
    */

    template <typename node, mp::tuple_interface args_as_tuple_t>
    decltype(auto) chain_invoke(std::tuple<node> && functors, args_as_tuple_t && args_as_tuple) {
        return details::apply_with_or_discard(std::get<0>(fwd(functors)), fwd(args_as_tuple));
    }
    template <typename node, typename ... rest, mp::tuple_interface args_as_tuple_t>
    decltype(auto) chain_invoke(std::tuple<node, rest...> && functors, args_as_tuple_t && args) {
        static_assert(sizeof...(rest) not_eq 0);

        auto result = details::invoke_into_tuple([](auto && ... args){
                return details::apply_with_or_discard(fwd(args)...);
            },
            std::get<0>(fwd(functors)), fwd(args)
        );

        return chain_invoke(
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return std::forward_as_tuple(std::get<1 + indexes>(fwd(functors))...);
            }(std::make_index_sequence<sizeof...(rest)>{}),
            std::move(result)
        );
    }

    // todo : owning by default, non-owning (opt-out) policy ?

    template <typename ... Fs>
    requires (sizeof...(Fs) not_eq 0)
    struct binder {
        static_assert((not std::is_reference_v<Fs> && ...));

        using storage_type = std::tuple<Fs...>;

        constexpr binder(auto && ... args)
        : storage{ fwd(args)... }
        {}

        constexpr decltype(auto) operator()(auto && ... args) & {
            return chain_invoke(
                [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) -> std::tuple<Fs&...> {
                    return { std::get<indexes>(storage)... };
                }(std::make_index_sequence<std::tuple_size_v<storage_type>>{}),
                std::forward_as_tuple(fwd(args)...)
            );
        }
        constexpr decltype(auto) operator()(auto && ... args) && {
            return chain_invoke(
                [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) -> std::tuple<Fs&&...> {
                    return std::forward_as_tuple(std::get<indexes>(fwd(storage))...);
                }(std::make_index_sequence<std::tuple_size_v<storage_type>>{}),
                std::forward_as_tuple(fwd(args)...)
            );
        }
        constexpr decltype(auto) operator()(auto && ... args) const & {
            return chain_invoke(
                [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) -> std::tuple<const Fs&...> {
                    return { std::get<indexes>(storage)... };
                }(std::make_index_sequence<std::tuple_size_v<storage_type>>{}),
                std::forward_as_tuple(fwd(args)...)
            );
        }
        constexpr decltype(auto) operator()(auto && ... args) const && {
            return chain_invoke(
                [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) -> std::tuple<const Fs&&...> {
                    return { std::get<indexes>(static_cast<const storage_type &&>(storage))... };
                }(std::make_index_sequence<std::tuple_size_v<storage_type>>{}),
                std::forward_as_tuple(fwd(args)...)
            );
        }

    private:

        storage_type storage;
    };
    template <typename ... Ts>
    binder(Ts &&...) -> binder<std::remove_cvref_t<Ts>...>;

    template <typename ... Fs>
    using route = binder<Fs...>;
}
// detections
namespace csl::wf::details::mp::detect {
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
namespace csl::wf::details {

    template <typename ... Ts>
    requires (mp::are_unique_v<Ts...>)
    struct overload : Ts... {
        using Ts::operator()...;
    };
    template <typename ... Ts>
    overload(Ts&&...) -> overload<Ts...>;
    template <typename ... Ts, typename ... Us> // merge
    overload(overload<Ts...>&&, overload<Us...>&&) -> overload<Ts..., Us...>;
    template <typename ... Ts, typename U> // append (back)
    overload(overload<Ts...>&&, U &&) -> overload<Ts..., U>;
    template <typename U, typename ... Ts> // append (front)
    overload(U &&, overload<Ts...> &&) -> overload<U, Ts...>;
}

// namespace aggregation
namespace csl::wf {
    using namespace csl::wf::mp;
}

#undef fwd

// wip : handle no return : https://godbolt.org/z/6x3b5Pf37

// todo : function should be able to return a mp::ttps + return value
//  route :
//      - invoke
//      - same for apply ?
//  => as invoke_policy : invoke | apply ?
//     or automated : is_applyable ? apply : invoke
//
// todo : constexpr
// todo : better error messages
// todo : static-analysis (sonar-lint, clang-tidy)
//  cppcoreguideline

// extensions :
// - coroutines, futures
// - async generators as entry-point for redundant calls
