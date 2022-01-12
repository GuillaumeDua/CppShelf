#pragma once

// Evolutions :
//  - Consider P1985 "Universal template declaration"
//  - Consider P2347 "Argument type deduction for non-trailing parameter packs" to deduce a single non-trailing parameters pack for function arguments
//  - std::any_invocable ?

#include <utility>
#include <type_traits>
#include <variant>
#include <concepts>
#include <functional>
//#include <ranges>
#include <vector>
#include <array>
#include <string_view>
#include <tuple>

// -- dev/wip/debug only
namespace gcl::cx {
    template <typename T>
    static constexpr /*consteval*/ auto type_name(/*no parameters allowed*/)
    -> std::string_view
    {
    #if defined(__GNUC__) or defined(__clang__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        constexpr std::string_view prefix = "T = ";
        str_view.remove_prefix(str_view.find(prefix) + prefix.length());
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
    static constexpr auto type_name(/*no parameters allowed*/)
    -> std::string_view
    {
        return type_name<decltype(value)>();
    }

    template <auto value>
    static constexpr auto value_name(/*no parameters allowed*/)
    -> std::string_view
    {
    #if defined(__GNUC__) or defined(__clang__)
        std::string_view str_view = __PRETTY_FUNCTION__;
        str_view.remove_prefix(str_view.find(__FUNCTION__) + sizeof(__FUNCTION__));
        constexpr std::string_view prefix = "value = ";
        str_view.remove_prefix(str_view.find(prefix) + prefix.length());
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

#define fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(cppcoreguidelines-macro-usage)

// todo : poc a cleaner design ?
//  details::apply which would be unsafe but hidden
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
    struct is_ttps : std::false_type{};
    template <typename ... Ts>
    struct is_ttps<ttps<Ts...>> : std::true_type{};
    template <typename T>
    constexpr bool is_ttps_v = is_ttps<T>::value;

    // template <typename T>
    // constexpr bool is_ttps_v = false;
    // template <typename ... Ts>
    // constexpr bool is_ttps_v<ttps<Ts...>> = true;

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
    using invoke_result_t = typename invoke_result<F, Ts...>::type;

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
// invoke
// apply(,_after,_before)
// front_binder, bind_front
namespace csl::wf {

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

    // front_bindable
    template <typename T>
    concept front_bindable = 
        std::is_constructible_v<std::decay_t<T>, T>
        and std::is_move_constructible_v<std::decay_t<T>>
    ;
    // front_binder
    //  todo :  copy, move constructors
    //          operator=
    //          operator==, operator not_eq
    //  todo :  member-variables binding ?
    template <
        wf::front_bindable F,
        typename ttps_pack_type,
        wf::front_bindable ... bounded_args_t
    >
    class front_binder;
    template <
        wf::front_bindable F,
        typename ... ttps_bounded_args_t,
        wf::front_bindable ... bounded_args_t
    >
    class front_binder<F, mp::ttps<ttps_bounded_args_t...>, bounded_args_t...> {

        using type = front_binder<F, mp::ttps<ttps_bounded_args_t...>, bounded_args_t...>;

        using bounded_args_storage_type = std::tuple<bounded_args_t...>;
        bounded_args_storage_type bounded_arguments;
        F f;

    public:

        constexpr front_binder(auto && f_arg, mp::ttps<ttps_bounded_args_t...>, auto && ... args)
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}
        constexpr explicit front_binder(auto && f_arg, auto && ... args)
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
        using invoke_result_t = typename chain_trait<rest...>::template invoke_result_t<
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
        using invoke_result_t = typename std::conditional_t<
            is_nodiscard_invocable<args_type...>,
            mp::invoke_result<node, args_type...>,
            mp::invoke_result<node>
        >::type;
    };
}
// mp::are_unique_v<Ts..>
// mp::is_instance_of<pack<...>, T>
namespace csl::wf::details::mp {

    template <typename T, typename ... Ts>
    constexpr bool are_unique_v = (not (std::is_same_v<T, Ts> or ...)) and are_unique_v<Ts...>;
    template <typename T>
    constexpr bool are_unique_v<T> = true;

    // is_instance_of
    template <template <typename...> typename type, typename T>
    struct is_instance_of : std::false_type{};
    template <template <typename...> typename type, typename ... Ts>
    struct is_instance_of<type, type<Ts...>>  : std::true_type{};
    template <template <typename...> typename type, typename T>
    constexpr bool is_instance_of_v = is_instance_of<type, T>::value;
    template <template <typename...> typename type, typename T>
    concept InstanceOf = is_instance_of_v<type, T>;
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
    constexpr decltype(auto) apply_with_or_discard(F && functor, tuple_type && args_as_tuple)
    noexcept(wf::mp::is_nothrow_applyable_v<F&&, tuple_type&&>)
    {
        return wf::apply(fwd(functor), fwd(args_as_tuple));
    }
    template <typename F, typename tuple_type>
    requires
        wf::mp::tuple_interface<tuple_type> and
        (not wf::mp::is_applyable_v<F, tuple_type&&>)
    constexpr decltype(auto) apply_with_or_discard(F && functor, tuple_type &&)
    noexcept(wf::mp::is_nothrow_invocable_v<F&&>)
    {
        static_assert(wf::mp::is_invocable_v<F&&>);
        return wf::invoke(fwd(functor));
    }
    
    // todo : multiple returns value (including ttps<...>)
    constexpr decltype(auto) invoke_into_tuple(auto && functor, auto && ... args)
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
// todo :
//  always owning ? mix owning and non-owning functors in storage ?
//  if non-owning, value semantic correctness for route/binder storage
namespace csl::wf {
    // particular case : always nodiscard operator() ?
    // bind_front -> take care of template arguments

    template <typename node, mp::tuple_interface args_as_tuple_t>
    constexpr decltype(auto) chain_invoke(std::tuple<node> && functors, args_as_tuple_t && args_as_tuple) {
        return details::apply_with_or_discard(std::get<0>(fwd(functors)), fwd(args_as_tuple));
    }
    template <typename node, typename ... rest, mp::tuple_interface args_as_tuple_t>
    requires (sizeof...(rest) not_eq 0)
    constexpr decltype(auto) chain_invoke(std::tuple<node, rest...> && functors, args_as_tuple_t && args) {

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
    struct route {
        static_assert(sizeof...(Fs) not_eq 0,               "csl::wf::binder : binds nothing");
        static_assert((not std::is_reference_v<Fs> && ...), "csl::wf::binder : non-owning");

        using storage_type = std::tuple<Fs...>;

        constexpr explicit route(auto && ... args)
        : storage{ fwd(args)... }
        {}

        // todo : conditionally noexcept
        // todo : conditionally enabled
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

        template <std::size_t index>
        using node_t = std::tuple_element_t<index, storage_type>;

        template <std::size_t index> constexpr decltype(auto) at() &         noexcept { return std::get<index>(storage); }
        template <std::size_t index> constexpr decltype(auto) at() const &   noexcept { return std::get<index>(storage); }
        template <std::size_t index> constexpr decltype(auto) at() &&        noexcept { return std::get<index>(std::move(storage)); }
        template <std::size_t index> constexpr decltype(auto) at() const &&  noexcept { return std::get<index>(std::move(storage)); }

        constexpr static std::size_t size = std::tuple_size_v<storage_type>;

    private:

        storage_type storage;
    };
    template <typename ... Ts>
    route(Ts &&...) -> route<std::remove_cvref_t<Ts>...>;

    // template <typename ... Fs>
    // using route = binder<Fs...>;

    template <std::size_t index, typename T>
    requires csl::wf::details::mp::InstanceOf<csl::wf::route, T>
    constexpr decltype(auto) get(T && value) noexcept {
        return fwd(value).template at<index>();
    }
}
// overload
namespace csl::wf::details {

    template <typename ... Ts>
    requires (mp::are_unique_v<Ts...>)
    struct overload : Ts... {

        static_assert((not std::is_reference_v<Ts> && ...));
        static_assert((not std::is_const_v<Ts> && ...));

        using Ts::operator()...;

        template <typename ... args_ts>
        constexpr explicit overload(args_ts &&... args)
        : Ts{ fwd(args) }...
        {}
    };
    template <typename ... Ts>
    overload(Ts&&...) -> overload<std::remove_cvref_t<Ts>...>;
}
// repeat
// - invoke_n_times
// - repeater
namespace csl::wf {
    // invoke_n_times
    template <auto times, typename F, typename ... Ts>
    requires(not csl::wf::mp::is_invocable_v<F&&, Ts&&...>) 
    constexpr decltype(auto) invoke_n_times(F && func, Ts && ... args) = delete;
    template <auto times, typename F, typename ... Ts>
    constexpr decltype(auto) invoke_n_times(F && func, Ts && ... args)
    noexcept(noexcept(csl::wf::invoke(fwd(func), fwd(args)...)))
    requires(std::is_void_v<csl::wf::mp::invoke_result_t<F&&, Ts&&...>>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((static_cast<void>(indexes), csl::wf::invoke(fwd(func), fwd(args)...)), ...);
        }(std::make_index_sequence<times>{});
    }
    template <auto times, typename F, typename ... Ts>
    constexpr decltype(auto) invoke_n_times(F && func, Ts && ... args)
    noexcept(noexcept(csl::wf::invoke(fwd(func), fwd(args)...)))
    {
        using invoke_result_t = csl::wf::mp::invoke_result_t<F&&, Ts&&...>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return std::array<invoke_result_t, times>{
                (static_cast<void>(indexes), csl::wf::invoke(fwd(func), fwd(args)...))...
            };
        }(std::make_index_sequence<times>{});
    }

    // repeater
    //  todo : cx vs. rt
    template <auto times, typename F> struct repeater;

    template <typename T>
    struct is_repeater : std::false_type{};
    template <auto times, typename T>
    struct is_repeater<repeater<times, T>> : std::true_type{};
    template <typename T>
    constexpr bool is_repeater_v = is_repeater<T>::value;

    template <auto times_, typename F>
    struct repeater {
        static_assert(not std::is_reference_v<F>);
        static_assert(not std::is_const_v<F>);

        constexpr static auto times = times_;
        using underlying_type = F;

        template <typename T>
        constexpr explicit repeater(T && func)
        requires (not is_repeater_v<std::remove_cvref_t<T>>)
        : storage{ fwd(func) }
        {}

        // flattening
    #pragma region flattening
        template <auto N, typename fun>
        friend class repeater;

        template <auto other_times, typename other_F>
        constexpr repeater(repeater<other_times, other_F> && func)
        : storage{ std::move(func.storage) }
        {}
        template <auto other_times, typename other_F>
        constexpr repeater(const repeater<other_times, other_F> & func)
        : storage{ func.storage }
        {}
    #pragma endregion

        // todo : ttps
        template <typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) &
        noexcept(noexcept(invoke_n_times<times>(std::declval<F&>(), fwd(args)...)))
        requires requires { invoke_n_times<times>(std::declval<F&>(), fwd(args)...); }
        {
            return invoke_n_times<times>(storage, fwd(args)...);
        }
        template <typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) const &
        noexcept(noexcept(invoke_n_times<times>(std::declval<const F&>(), fwd(args)...)))
        requires requires { invoke_n_times<times>(std::declval<const F&>(), fwd(args)...); }
        {
            return invoke_n_times<times>(storage, fwd(args)...);
        }
        template <typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) &&
        noexcept(noexcept(invoke_n_times<times>(std::declval<F&&>(), fwd(args)...)))
        requires requires { invoke_n_times<times>(std::declval<F&&>(), fwd(args)...); }
        {
            return invoke_n_times<times>(std::move(storage), fwd(args)...);
        }
        template <typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) const &&
        noexcept(noexcept(invoke_n_times<times>(std::declval<const F&&>(), fwd(args)...)))
        requires requires { invoke_n_times<times>(std::declval<const F&&>(), fwd(args)...); }
        {
            return invoke_n_times<times>(std::move(storage), fwd(args)...);
        }
        
    private:
        using storage_type = F;
        storage_type storage;
    };

    // repeater_factory
    //  ADL helper
    template <auto times>
    struct repeater_factory {

        template <typename F>
        static constexpr auto make(F && arg) {
            return repeater<times, std::remove_cvref_t<F>>{ fwd(arg) };
        }

        // flattening
        template <typename F>
        requires (is_repeater_v<std::remove_cvref_t<F>>)
        static constexpr auto make(F && arg) {
            using f_type = std::remove_cvref_t<F>;
            constexpr auto repetition_times = times * f_type::times;
            using underlying_underlying_type  = typename f_type::underlying_type;

            using repeater_type = repeater<repetition_times, underlying_underlying_type>;
            return repeater_type{ fwd(arg) };
        }
    };
}
// flattening utility
// - mp::unfold_to<dest<...>, T>
// - mp::flatten_of_t<pack<...>, Ts...>
// - mp::unfold_super_into<pack<...>, Ts...>
// - mp::make_flatten_super(super<...>, Ts&&...)
namespace csl::wf::details::mp {

    // unfold_to
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_to : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_to<destination, from<Ts...>> : unfold_to<destination, Ts...>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_to_t = typename unfold_to<destination, from...>::type;

    // flatten_of_t
    template <template <typename ...> typename pack_type, typename ... Ts>
    using flatten_of_t = unfold_to_t<
        pack_type,
        decltype(std::tuple_cat(
            std::conditional_t<
                is_instance_of_v<pack_type, Ts>,
                unfold_to_t<std::tuple, Ts>,
                std::tuple<Ts>
            >{}...
        ))
    >;

    // unfold_super_into
    template <template <typename ...> typename destination_type, typename T>
    constexpr decltype(auto) unfold_super_into(T && value) {
        return destination_type<T&&>(fwd(value));
    }
    template <template <typename ...> typename destination_type, typename ... Ts>
    constexpr decltype(auto) unfold_super_into(destination_type<Ts...> && value) {
        return fwd(value);
        // return destination_type<Ts&&...>(static_cast<Ts&&>(value)...);
    }
    template <template <typename ...> typename destination_type, template <typename ...> typename origin_type, typename ... Ts>
    constexpr decltype(auto) unfold_super_into(origin_type<Ts...> && value) {
        return destination_type<Ts&&...>(static_cast<Ts&&>(value)...);
    }

    // make_flatten_super
    template <template <typename ...> typename super_type, typename... Ts>
    constexpr auto make_flatten_super(Ts && ... args) {

        auto flatten_args_as_tuple = std::tuple_cat(unfold_super_into<std::tuple>(fwd(args))...);
        constexpr auto flatten_args_as_tuple_index_sequence = std::make_index_sequence<
            std::tuple_size_v<std::remove_cvref_t<decltype(flatten_args_as_tuple)>>
        >{};

        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return super_type{
                std::get<indexes>(std::move(flatten_args_as_tuple))...
            };
        }(flatten_args_as_tuple_index_sequence);
    }

    // fwd_nodes_as_tuple
    template <template <typename...> typename destination_type, typename T>
    constexpr auto fwd_nodes_into(T && value) {
        return destination_type<T&&>{ fwd(value) };
    }
    template <template <typename...> typename destination_type, typename T>
    requires csl::wf::details::mp::InstanceOf<csl::wf::route, T>
    constexpr auto fwd_nodes_into(T && value) {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return destination_type<typename T::template node_t<indexes>&&...>{
                csl::wf::get<indexes>(fwd(value))...
            };
        }(std::make_index_sequence<T::size>{});
    }
}
// detections
namespace csl::wf::details::mp::detect {
    // avoid recursive concepts
    // could use std::detected_t (library fundamentals TS v2)

    // have_pipe_operator
    template <typename T, typename U, typename = void>
    struct have_pipe_operator : std::false_type{};
    template <typename T, typename U>
    struct have_pipe_operator<T, U, std::void_t<decltype(std::declval<T>() | std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_pipe_operator_v = have_pipe_operator<T, U>::value;

    // have_shift_equal_operator
    template <typename T, typename U, typename = void>
    struct have_shift_equal_operator : std::false_type{};
    template <typename T, typename U>
    struct have_shift_equal_operator<T, U, std::void_t<decltype(std::declval<T>() >>= std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_shift_equal_operator_v = have_shift_equal_operator<T, U>::value;

    // have_plus_operator
    template <typename T, typename U, typename = void>
    struct have_plus_operator : std::false_type{};
    template <typename T, typename U>
    struct have_plus_operator<T, U, std::void_t<decltype(std::declval<T>() + std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_plus_operator_v = have_plus_operator<T, U>::value;

    // have_multiply_operator
    template <typename T, typename U, typename = void>
    struct have_multiply_operator : std::false_type{};
    template <typename T, typename U>
    struct have_multiply_operator<T, U, std::void_t<decltype(std::declval<T>() * std::declval<U>())>>
    : std::true_type{};
    template <typename T, typename U>
    constexpr bool have_multiply_operator_v = have_multiply_operator<T, U>::value;
}
// eDSL utility
namespace csl::wf {
    // make_continuation
    template <typename ... Ts>
    constexpr auto make_continuation(Ts && ... nodes) {
        return csl::wf::route{ fwd(nodes)... };
    }
    template <typename ... Ts>
    requires (csl::wf::details::mp::InstanceOf<csl::wf::route, Ts> or ...)
    constexpr auto make_continuation(Ts && ... nodes) {
        auto route_elements = std::tuple_cat(
            csl::wf::details::mp::fwd_nodes_into<std::tuple>(fwd(nodes))...
        );
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return csl::wf::route{
                std::get<indexes>(std::move(route_elements))...
            };
        }(std::make_index_sequence<std::tuple_size_v<decltype(route_elements)>>{});
    }

    // make_condition
    template <typename ... Ts>
    constexpr auto make_condition(Ts && ... nodes) {
        return csl::wf::details::overload { fwd(nodes)... };
    }
    template <typename ... Ts>
    requires (csl::wf::details::mp::InstanceOf<csl::wf::details::overload, Ts> or ...)
    constexpr auto make_condition(Ts && ... nodes) {
        return csl::wf::details::mp::make_flatten_super<csl::wf::details::overload>(
            fwd(nodes)...
        );
    }

    // make_repetition
    template <auto times, typename F>
    constexpr decltype(auto) make_repetition(F && func) {
        return repeater_factory<times>::make(fwd(func));
    }
}
// eDSL
namespace csl::wf::operators {
    
    // operator|
    template <typename lhs_t, typename rhs_t>
    constexpr auto operator|(lhs_t && lhs, rhs_t && rhs) {
        return make_condition(fwd(lhs), fwd(rhs));
    }

    // operator>>=
    template <typename lhs_t, typename rhs_t>
    constexpr auto operator>>=(lhs_t && lhs, rhs_t && rhs) {
        return make_continuation(fwd(lhs), fwd(rhs));
    }
}

// namespace aggregation
namespace csl::wf {
    using namespace csl::wf::mp;
}

#undef fwd

// todo : function should be able to return a mp::ttps + return value
//  route :
//      - invoke
//      - same for apply ?
//  => as invoke_policy : invoke | apply ?
//     or automated : is_applyable ? apply : invoke
//
// todo : better error messages

// extensions :
// - coroutines, futures
// - async generators as entry-point for redundant calls
// - network ?
