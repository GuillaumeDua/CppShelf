#pragma once

// Evolutions :
//  - Consider P1985 "Universal template declaration"
//  - Consider P2347 "Argument type deduction for non-trailing parameter packs" to deduce a single non-trailing parameters pack for function arguments
//  - std::any_invocable ?

#include <utility>
#include <type_traits>
#include <concepts>
#include <functional>
#include <array>
#include <string_view>
#include <tuple>
#include <cmath>
#include <stdexcept>

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

// TODO : mp::ttps in args never discardable
// TODO : remove unused functions
// TODO : simplify code with std::apply(invoke<>, args),
//        or unfold_to
// TODO : poc a cleaner design ?
//  details::apply which would be unsafe but hidden
//  apply safe, part of the API
//  applyable as requires { apply }
//  nothrow_applyable as requires { apply } noexcept
// however, STL's invocable is requires { std::invoke(...) }
//  https://en.cppreference.com/w/cpp/concepts/invocable

namespace csl::wf::concepts {
    template <typename T>
    concept tuple_interface = requires { std::tuple_size<std::remove_reference_t<T>>{}; };
    template <typename T>
    concept tuple_empty = tuple_interface<T> and (std::tuple_size_v<std::remove_reference_t<T>> == 0);
    template <typename T>
    concept tuple_not_empty = tuple_interface<T> and (std::tuple_size_v<std::remove_reference_t<T>> > 0);
    template <typename T>
    concept tuple_optional = tuple_interface<T> and (std::tuple_size_v<std::remove_reference_t<T>> <= 1);

     // tuple non-owning/view
    template <typename T>
    concept tuple_view = concepts::tuple_interface<T> and
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr noexcept { // NOLINT
            return ((std::is_reference_v<std::tuple_element_t<indexes, std::remove_cvref_t<T>>> && ...));
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{})
    ;
}
// make_tuple_subview
// tuple_view_t, tuple_optional_t
namespace csl::wf::mp {
    template <
        std::size_t pos = 0,
        std::size_t count = std::string_view::npos
    >
    constexpr concepts::tuple_interface auto make_tuple_subview(concepts::tuple_interface auto && tuple_value) noexcept
    requires (pos <= std::tuple_size_v<std::remove_cvref_t<decltype(tuple_value)>>)
    {
        using remove_cvref_tuple_type = std::remove_cvref_t<decltype(tuple_value)>;
        constexpr auto tuple_size = std::tuple_size_v<remove_cvref_tuple_type>;
        constexpr auto length = std::min(count, tuple_size - pos);
        return [&tuple_value]<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr noexcept {
            return std::tuple<
                decltype(std::get<pos + indexes>(std::forward<decltype(tuple_value)>(tuple_value)))...
            >{           std::get<pos + indexes>(std::forward<decltype(tuple_value)>(tuple_value))... };
        }(std::make_index_sequence<length>{});
    }
    constexpr concepts::tuple_interface auto make_tuple_view(concepts::tuple_interface auto && tuple_value) noexcept {
        return make_tuple_subview(std::forward<decltype(tuple_value)>(tuple_value));
    }

    template <concepts::tuple_interface T>
    using tuple_view_t = decltype(make_tuple_subview(std::declval<T>()));

    template <typename T>
    using tuple_optional_t = std::conditional_t<
        std::is_same_v<T, void>,
        std::tuple<>,
        std::array<T, 1>
    >;
    template <concepts::tuple_optional T>
    using tuple_optional_underlying_t = typename std::conditional_t<
        std::is_same_v<T, std::tuple<>>,
        std::type_identity<void>,
        std::tuple_element<0, T>
    >::type;
}
// cat
// is_same_pack_as
namespace csl::wf::details::mp {
    // cat / append
    template <typename, typename...>
    struct cat;
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, Us...> : std::type_identity<pack<Ts..., Us...>>{};
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, pack<Us...>> : std::type_identity<pack<Ts..., Us...>>{};
    template <typename pack, typename... ttps>
    using cat_t = typename cat<pack, ttps...>::type;

    template <typename T, typename U>
    struct is_same_ttp : std::false_type{};
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct is_same_ttp<pack<Ts...>, pack<Us...>> : std::true_type{};
    template <typename T, typename U>
    constexpr bool is_same_ttp_v = is_same_ttp<T, U>::value;
}
// ttps, is_ttps
// args, is_args
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

    // args -> pack of args
    template <typename ...>
    struct args{};

    template <typename T>
    struct is_args : std::false_type{};
    template <typename ... Ts>
    struct is_args<args<Ts...>> : std::true_type{};
    template <typename T>
    constexpr bool is_args_v = is_args<T>::value;
}
// ttps
// tupleinterface_(not_)starting_with_ttps
// same_ttp_as
namespace csl::wf::concepts {

    template <typename T>
    concept ttps = mp::is_ttps_v<std::remove_cvref_t<T>>;
    template <typename T>
    concept args = mp::is_args_v<std::remove_cvref_t<T>>;

    template <typename T>
    concept tupleinterface_starting_with_ttps = tuple_not_empty<T> and ttps<std::tuple_element_t<0, std::remove_cvref_t<T>>>;
    template <typename T>
    concept tupleinterface_not_starting_with_ttps = tuple_interface<T> and not tupleinterface_starting_with_ttps<T>;

    template <typename T, typename U>
    concept same_ttp_as = details::mp::is_same_ttp_v<T, U>;
}

// is_(nothrow_)invocable(_r), invoke_result
// is_(nothrow_)applyable(_before|_after)
namespace csl::wf::mp {

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
    template <typename F, concepts::ttps ttps_type, typename... args_types>
    struct is_invocable<F, ttps_type, args_types...> : is_invocable<F, std::remove_cvref_t<ttps_type>, args_types...>{};
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
    template <typename F, concepts::ttps ttps_type, typename... args_types>
    struct is_nothrow_invocable<F, ttps_type, args_types...> : is_nothrow_invocable<F, std::remove_cvref_t<ttps_type>, args_types...>{};
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
    template <typename R, typename F, concepts::ttps ttps_type, typename... args_types>
    struct is_invocable_r<R, F, ttps_type, args_types...> : is_invocable_r<R, F, std::remove_cvref_t<ttps_type>, args_types...>{};
    template <typename R, typename F, typename... Ts>
    constexpr bool is_invocable_r_v = is_invocable_r<R, F, Ts...>::value;

    // is_nothrow_invocable_r<R, F, [ttps<...>,] args_types...>
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
    template <typename R, typename F, concepts::ttps ttps_type, typename... args_types>
    struct is_nothrow_invocable_r<R, F, ttps_type, args_types...> : is_nothrow_invocable_r<R, F, std::remove_cvref_t<ttps_type>, args_types...>{};
    template <typename R, typename F, typename... Ts>
    constexpr bool is_nothrow_invocable_r_v = is_nothrow_invocable_r<R, F, Ts...>::value;

    // invoke_result
    template <typename F, typename... args_types>
    struct invoke_result : std::invoke_result<F, args_types...>{};
    template <typename F, typename ... args_types>
    struct invoke_result<F, ttps<>, args_types...> : invoke_result<F, args_types...>{};
    template <typename F, typename ... ttps_args, typename ... args_types>
    struct invoke_result<F, ttps<ttps_args...>, args_types...> {
        
        static_assert(requires{
            std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...);
        }, "csl::wf::invoke_result : invalid resolution");

        using type = decltype(
            std::declval<F>().template operator()<ttps_args...>(std::declval<args_types>()...)
        );
    };
    template <typename F, concepts::ttps ttps_type, typename... args_types>
    struct invoke_result<F, ttps_type, args_types...> : invoke_result<F, std::remove_cvref_t<ttps_type>, args_types...>{};
    template <typename F, typename... Ts>
    using invoke_result_t = typename invoke_result<F, Ts...>::type;

    // ---

    // is_applyable
    template <typename F, typename...>
    struct is_applyable //: std::false_type{};
    {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename F, concepts::tuple_interface tuple_type>
    struct is_applyable<F, tuple_type> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, decltype(std::get<indexes>(std::declval<tuple_type>()))...>; // std::get to preserve cvref qualifiers
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, typename ... ttps_args, concepts::tupleinterface_not_starting_with_ttps tuple_type>
    struct is_applyable<F, ttps<ttps_args...>, tuple_type> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, ttps<ttps_args...>, decltype(std::get<indexes>(std::declval<tuple_type>()))...>; // std::get to preserve cvref qualifiers
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, concepts::ttps ttps_type, concepts::tupleinterface_not_starting_with_ttps tuple_type>
    struct is_applyable<F, ttps_type, tuple_type> : is_applyable<F, std::remove_cvref_t<ttps_type>, tuple_type>{};
    template <typename F, typename... Ts>
    constexpr bool is_applyable_v = is_applyable<F, Ts...>::value;

    // is_nothrow_applyable
    template <typename F, typename...>
    struct is_nothrow_applyable //: std::false_type{};
    {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename F, concepts::tuple_interface tuple_type>
    struct is_nothrow_applyable<F, tuple_type> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, typename ... ttps_args, concepts::tupleinterface_not_starting_with_ttps tuple_type>
    struct is_nothrow_applyable<F, ttps<ttps_args...>, tuple_type> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, ttps<ttps_args...>, decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, concepts::ttps ttps_type, concepts::tupleinterface_not_starting_with_ttps tuple_type>
    struct is_nothrow_applyable<F, ttps_type, tuple_type> : is_nothrow_applyable<F, std::remove_cvref_t<ttps_type>, tuple_type>{};
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_applyable_v = is_nothrow_applyable<F, Ts...>::value;

    // is_applyable_before (ttps cannot be part of tuple)
    template <typename F, typename...>
    struct is_applyable_before //: std::false_type{};
    {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    // template <typename ... f_ts, typename F, concepts::tupleinterface_not_starting_with_ttps tuple_type, typename ... func_args_t>
    template <typename ... f_ts, typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_before<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, ttps<f_ts...>, decltype(std::get<indexes>(std::declval<tuple_type>()))..., func_args_t...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    // todo : deal with ttps concat at invoke level ?
    //  - template <ts...> + f(ttps<us...>) => f<ts..., us...>()
    // template <typename ... f_ts, typename F, concepts::tupleinterface_starting_with_ttps tuple_type, typename ... func_args_t>
    // struct is_applyable_before<F, ttps<f_ts...>, tuple_type, func_args_t...> {
    //     constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
    //         using ttps_type = details::mp::cat<std::tuple_element_t<0, tuple_type>, ttps<f_ts...>>;
    //         return is_invocable_v<F, ttps_type, decltype(std::get<indexes + 1>(std::declval<tuple_type>()))..., func_args_t...>;
    //     }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>> - 1>{});
    // };
    template <typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_before<F, tuple_type, func_args_t...>
    : is_applyable_before<F, ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_applyable_before_v = is_applyable_before<F, Ts...>::value;

    // is_nothrow_applyable_before (ttps cannot be part of tuple)
    template <typename F, typename...>
    struct is_nothrow_applyable_before {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_before<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, ttps<f_ts...>, decltype(std::get<indexes>(std::declval<tuple_type>()))..., func_args_t...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_before<F, tuple_type, func_args_t...>
    : is_nothrow_applyable_before<F, mp::ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_applyable_before_v = is_nothrow_applyable_before<F, Ts...>::value;

    // is_applyable_after (ttps cannot be part of tuple)
    template <typename F, typename...>
    struct is_applyable_after {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_after<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_invocable_v<F, ttps<f_ts...>, func_args_t..., decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_applyable_after<F, tuple_type, func_args_t...>
    : is_applyable_after<F, mp::ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_applyable_after_v = is_applyable_after<F, Ts...>::value;

    // is_nothrow_applyable_after (ttps cannot be part of tuple)
    template <typename F, typename...>
    struct is_nothrow_applyable_after {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename ... f_ts, typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_after<F, ttps<f_ts...>, tuple_type, func_args_t...> {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            return is_nothrow_invocable_v<F, ttps<f_ts...>, func_args_t..., decltype(std::get<indexes>(std::declval<tuple_type>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{});
    };
    template <typename F, concepts::tuple_interface tuple_type, typename ... func_args_t>
    struct is_nothrow_applyable_after<F, tuple_type, func_args_t...>
    : is_nothrow_applyable_after<F, mp::ttps<>, tuple_type, func_args_t...>
    {};
    template <typename F, typename... Ts>
    constexpr bool is_nothrow_applyable_after_v = is_nothrow_applyable_after<F, Ts...>::value;

    // apply_result
    template <typename F, typename...>
    struct apply_result
    {
        static_assert([]() constexpr { return false; }(), "invalid arguments");
    };
    template <typename F, concepts::tuple_interface tuple_type>
    struct apply_result<F, tuple_type> : decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return std::type_identity<
                typename mp::invoke_result_t<F, decltype(std::get<indexes>(std::declval<tuple_type>()))...>
            >{};
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{}))
    {};
    template <typename F, typename ... ttps_args, concepts::tupleinterface_not_starting_with_ttps tuple_type>
    struct apply_result<F, ttps<ttps_args...>, tuple_type> : decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return std::type_identity<
                typename mp::invoke_result_t<F, ttps<ttps_args...>, decltype(std::get<indexes>(std::declval<tuple_type>()))...>
            >{};
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{}))
    {};
    template <typename F, concepts::ttps ttps_type, concepts::tupleinterface_not_starting_with_ttps tuple_type>
    struct apply_result<F, ttps_type, tuple_type> : apply_result<F, std::remove_cvref_t<ttps_type>, tuple_type>{};
    template <typename F, typename ... Ts>
    using apply_result_t = typename apply_result<F, Ts...>::type;
}
// is(_nothrow_)invocable_with
namespace csl::wf::mp {
    // Extension to handle both `ttps` and `args` as pack
    // more convenient for pack_traits - like filters - applications
    //
    // Less restrictive than `is_applyable`, as `args` does not match TupleInterface.
    // Also, easier to handle types that does not fit in std::tuple, array and paires, like `void`.

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
    template <typename ... ttps_args, typename F, typename ... args_types>
    requires
        mp::is_invocable_v<F&&, mp::ttps<ttps_args...>, args_types&&...>
    constexpr decltype(auto) invoke(F && f, mp::ttps<ttps_args...>, args_types&& ... args)
    noexcept(mp::is_nothrow_invocable_v<F&&, mp::ttps<ttps_args...>, args_types&&...>)
    {
        return invoke<ttps_args...>(fwd(f), fwd(args)...);
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
    template <typename ... f_ts, typename F, concepts::tupleinterface_not_starting_with_ttps args_as_tuple_t>
    requires mp::is_applyable_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t>
    constexpr decltype(auto) apply(F && f, args_as_tuple_t&& args)
    noexcept(mp::is_nothrow_applyable_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_invocable_v<F&&, mp::ttps<f_ts...>, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), std::get<indexes>(fwd(args))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }
    template <concepts::ttps ttps, typename F, concepts::tupleinterface_not_starting_with_ttps args_as_tuple_t>
    requires mp::is_applyable_v<F&&, ttps, args_as_tuple_t>
    constexpr decltype(auto) apply(F && f, ttps, args_as_tuple_t&& args)
    noexcept(mp::is_nothrow_applyable_v<F&&, ttps, args_as_tuple_t>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_invocable_v<F&&, ttps, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))...>)
        -> decltype(auto)
        {
            return invoke(std::forward<F>(f), ttps{}, std::get<indexes>(fwd(args))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }
    template <typename F, concepts::tupleinterface_starting_with_ttps args_as_tuple_t>
    constexpr decltype(auto) apply(F && f, args_as_tuple_t && args)
    noexcept(mp::is_nothrow_applyable_v<F&&, decltype(fwd(args))>)
    requires mp::is_applyable_v<F&&, decltype(fwd(args))>
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_invocable_v<F&&, decltype(std::get<indexes>(std::declval<decltype(args)>()))...>)
        -> decltype(auto)
        {
            return invoke(std::forward<F>(f), std::get<indexes>(fwd(args))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<decltype(args)>>>{});
    }

    // apply_before
    template <typename ... f_ts, typename F, concepts::tuple_interface args_as_tuple_t, typename ... func_args_t>
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
    template <typename ... f_ts, typename F, concepts::tuple_interface args_as_tuple_t, typename ... func_args_t>
    requires mp::is_applyable_after_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>
    constexpr decltype(auto) apply_after(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args)
    noexcept(mp::is_nothrow_applyable_after_v<F&&, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::is_nothrow_applyable_after_v<F, mp::ttps<f_ts...>, args_as_tuple_t, func_args_t...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), fwd(func_args)..., fwd(std::get<indexes>(fwd(args)))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }
}
namespace csl::wf::fold_policy {
    struct right {
        template <typename T, csl::wf::concepts::same_ttp_as<T> U>
        using fold_t = csl::wf::details::mp::cat_t<T, U>;

        constexpr static decltype(auto) fold_values(
            csl::wf::concepts::tuple_interface auto && lhs,
            csl::wf::concepts::tuple_interface auto && rhs
        )
        {
            return std::tuple_cat(fwd(lhs), fwd(rhs));
        }
    };
    struct left {
        template <typename T, csl::wf::concepts::same_ttp_as<T> U>
        using fold_t = csl::wf::details::mp::cat_t<U, T>;

        constexpr static decltype(auto) fold_values(
            csl::wf::concepts::tuple_interface auto && lhs,
            csl::wf::concepts::tuple_interface auto && rhs
        )
        {
            return std::tuple_cat(fwd(rhs), fwd(lhs));
        }
    };
}
// binder
// front_binder
// back_binder
// bind_front
// bind_back
namespace csl::wf {

    template <
        typename ttps_binding_policy,
        typename args_binding_policy = ttps_binding_policy
    >
    struct binding_policy {
        template <
            csl::wf::concepts::ttps bounded_ttps, csl::wf::concepts::ttps ttps,
            typename F,
            csl::wf::concepts::tuple_interface bounded_args, csl::wf::concepts::tuple_interface args
        >
        constexpr static bool is_invocable_v = csl::wf::mp::is_applyable_v<
            F,
            typename ttps_binding_policy::template fold_t<bounded_ttps, ttps>,
            typename args_binding_policy::template fold_t<mp::tuple_view_t<bounded_args>, args>
        >;

        template <
            csl::wf::concepts::ttps bounded_ttps, csl::wf::concepts::ttps ttps,
            typename F,
            csl::wf::concepts::tuple_interface bounded_args, csl::wf::concepts::tuple_interface args
        >
        constexpr static bool is_nothrow_invocable_v = csl::wf::mp::is_nothrow_applyable_v<
            F,
            typename ttps_binding_policy::template fold_t<bounded_ttps, ttps>,
            typename args_binding_policy::template fold_t<mp::tuple_view_t<bounded_args>, args>
        >;

        template <csl::wf::concepts::ttps bounded_ttps, csl::wf::concepts::ttps ttps>
        constexpr static decltype(auto) invoke(
            auto f,
            csl::wf::concepts::tuple_interface auto bounded_args,
            csl::wf::concepts::tuple_interface auto args
        ){
            return csl::wf::apply(
                fwd(f),
                typename ttps_binding_policy::template fold_t<bounded_ttps, ttps>{},
                args_binding_policy::fold_values(fwd(bounded_args), fwd(args))
            );
        }
    };

    using front_binding_policy = binding_policy<fold_policy::right>;
    using back_binding_policy = binding_policy<fold_policy::left>;

    // binder
    template <
        typename invoke_policy,
        typename F,
        concepts::ttps ttps_bounded_ts,
        concepts::args args_bounded_ts
    >
    class binder;
    template <
        typename invoke_policy,
        typename F,
        typename ... ttps_bounded_ts,
        typename ... args_bounded_ts
    >
    class binder<invoke_policy, F, mp::ttps<ttps_bounded_ts...>, mp::args<args_bounded_ts...>> {

        using type = binder<invoke_policy, F, mp::ttps<ttps_bounded_ts...>, mp::args<args_bounded_ts...>>;

        F f;
        using bounded_args_storage_type = std::tuple<args_bounded_ts...>;
        bounded_args_storage_type bounded_arguments;

    public:

        #pragma region basics
        constexpr binder(invoke_policy, auto && f_arg, mp::ttps<ttps_bounded_ts...>, auto && ... args)
        noexcept(
            std::is_nothrow_constructible_v<F, decltype(f_arg)> and
            std::is_nothrow_constructible_v<bounded_args_storage_type, decltype(args)...>
        )
        requires (
            sizeof...(args) == sizeof...(args_bounded_ts) and
            std::constructible_from<F, decltype(f_arg)> and
            std::constructible_from<bounded_args_storage_type, decltype(args)...>
        )
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}
        constexpr binder(invoke_policy, auto && f_arg, auto && ... args)
        noexcept(
            std::is_nothrow_constructible_v<F, decltype(f_arg)> and
            std::is_nothrow_constructible_v<bounded_args_storage_type, decltype(args)...>
        )
        requires (
            sizeof...(args) == sizeof...(args_bounded_ts) and
            std::constructible_from<F, decltype(f_arg)> and
            std::constructible_from<bounded_args_storage_type, decltype(args)...>
        )
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}
        constexpr binder(auto && f_arg, mp::ttps<ttps_bounded_ts...>, auto && ... args)
        noexcept(
            std::is_nothrow_constructible_v<F, decltype(f_arg)> and
            std::is_nothrow_constructible_v<bounded_args_storage_type, decltype(args)...>
        )
        requires (
            sizeof...(args) == sizeof...(args_bounded_ts) and
            std::constructible_from<F, decltype(f_arg)> and
            std::constructible_from<bounded_args_storage_type, decltype(args)...>
        )
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}
        constexpr explicit binder(auto && f_arg, auto && ... args)
        noexcept(
            std::is_nothrow_constructible_v<F, decltype(f_arg)> and
            std::is_nothrow_constructible_v<bounded_args_storage_type, decltype(args)...>
        )
        requires (
            sizeof...(args) == sizeof...(args_bounded_ts) and
            std::constructible_from<F, decltype(f_arg)> and
            std::constructible_from<bounded_args_storage_type, decltype(args)...>
        )
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}

        constexpr binder() = delete;
        constexpr binder(binder&&) noexcept = default;
        constexpr binder(const binder&) = default;
        constexpr binder & operator=(binder &&) noexcept = default;
        constexpr binder & operator=(const binder &) = default;
        constexpr ~binder() = default;

        // Fix GCC illed evaluation or `std::get<0>(value) == std::get<0>(other)`
        // constexpr auto operator==(const binder & other) const noexcept -> bool = default;
        constexpr auto operator==(const binder & other) const noexcept -> bool 
        requires std::equality_comparable<F> and std::equality_comparable<bounded_args_storage_type> {
            return f == other.f and bounded_arguments == other.bounded_arguments;
        };

        constexpr void swap(type && other)
        noexcept (
            std::is_nothrow_swappable_v<decltype(f)> and
            std::is_nothrow_swappable_v<decltype(bounded_arguments)>
        )
        requires
            std::is_swappable_v<decltype(f)> and
            std::is_swappable_v<decltype(bounded_arguments)>
        {
            std::swap(f, other.f);
            std::swap(bounded_arguments, other.bounded_arguments);
        }

        #pragma endregion

        #pragma region operator()
        template <typename ... ttps, typename ... parameters_t>
        requires invoke_policy::template is_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            F&,
            bounded_args_storage_type&, std::tuple<parameters_t&&...>
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) &
        noexcept(invoke_policy::template is_nothrow_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            F&,
            bounded_args_storage_type&, std::tuple<parameters_t&&...>
        >)
        {
            return invoke_policy::template invoke<
                mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>
            >(f, bounded_arguments, std::forward_as_tuple(std::forward<decltype(parameters)>(parameters)...));
        }

        template <typename ... ttps, typename ... parameters_t>
        requires invoke_policy::template is_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            const F&,
            const bounded_args_storage_type&, std::tuple<parameters_t&&...>
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) const &
        noexcept(invoke_policy::template is_nothrow_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            const F&,
            const bounded_args_storage_type&, std::tuple<parameters_t&&...>
        >)
        {
            return invoke_policy::template invoke<
                mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>
            >(f, bounded_arguments, std::forward_as_tuple(std::forward<decltype(parameters)>(parameters)...));
        }

        template <typename ... ttps, typename ... parameters_t>
        requires invoke_policy::template is_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            F&&,
            bounded_args_storage_type&&, std::tuple<parameters_t&&...>
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) &&
        noexcept(invoke_policy::template is_nothrow_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            F&&,
            bounded_args_storage_type&&, std::tuple<parameters_t&&...>
        >)
        {
            return invoke_policy::template invoke<
                mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>
            >(std::move(f), std::move(bounded_arguments), std::forward_as_tuple(std::forward<decltype(parameters)>(parameters)...));
        }

        template <typename ... ttps, typename ... parameters_t>
        requires invoke_policy::template is_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            const F&&,
            const bounded_args_storage_type&&, std::tuple<parameters_t&&...>
        >
        constexpr decltype(auto) operator()(parameters_t && ... parameters) const &&
        noexcept(invoke_policy::template is_nothrow_invocable_v<
            mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>,
            const F&&,
            const bounded_args_storage_type&&, std::tuple<parameters_t&&...>
        >) {
            return invoke_policy::template invoke<
                mp::ttps<ttps_bounded_ts...>, mp::ttps<ttps...>
            >(std::move(f), std::move(bounded_arguments), std::forward_as_tuple(std::forward<decltype(parameters)>(parameters)...));
        }

        // template <typename ... ttps, typename ... parameters_t>
        // constexpr decltype(auto) operator()(parameters_t && ... parameters) {
        //     static_assert([](){ return false; }(), "front_binder::operator() : no overload candidates matched");
        // }
        #pragma endregion
    
        constexpr const auto &  underlying_function() const noexcept { return f; }
    };
    template <
        typename invoke_policy,
        typename F,
        typename ... ttps_bounded_ts,
        typename ... args_bounded_ts
    >
    binder(invoke_policy, F&&, mp::ttps<ttps_bounded_ts...>, args_bounded_ts&&...)
    -> binder<invoke_policy, std::decay_t<F>, mp::ttps<ttps_bounded_ts...>, mp::args<std::decay_t<args_bounded_ts>...>>;
    template <
        typename invoke_policy,
        typename F,
        typename ... args_bounded_ts
    >
    binder(invoke_policy, F&&, args_bounded_ts&&...)
    -> binder<invoke_policy, std::decay_t<F>, mp::ttps<>, mp::args<std::decay_t<args_bounded_ts>...>>;

    // front_binder
    template <
        typename F,
        concepts::ttps ttps_ts,
        concepts::args args_ts
    >
    struct front_binder : binder<front_binding_policy, F, ttps_ts, args_ts>{
        using binder_type = binder<front_binding_policy, F, ttps_ts, args_ts>;
        explicit constexpr front_binder(auto && ... args)
        noexcept(std::is_nothrow_constructible_v<binder_type, decltype(args)...>)
        : binder_type{ std::forward<decltype(args)>(args)... }
        {}

        using binder_type::operator();
    };
    template <
        typename F, 
        typename ... ttps_ts,
        typename ... args_ts
    >
    front_binder(F&&, mp::ttps<ttps_ts...>, args_ts&&...) -> front_binder<std::decay_t<F>, mp::ttps<ttps_ts...>, mp::args<std::decay_t<args_ts>...>>;
    template <typename F, typename ... args_ts>
    front_binder(F&&, args_ts&&...) -> front_binder<std::decay_t<F>, mp::ttps<>, mp::args<std::decay_t<args_ts>...>>;

    // back_binder
    template <
        typename F,
        concepts::ttps ttps_ts,
        concepts::args args_ts
    >
    struct back_binder : binder<back_binding_policy, F, ttps_ts, args_ts>{
        using binder_type = binder<back_binding_policy, F, ttps_ts, args_ts>;
        explicit constexpr back_binder(auto && ... args)
        noexcept(std::is_nothrow_constructible_v<binder_type, decltype(args)...>)
        : binder_type{ std::forward<decltype(args)>(args)... }
        {}

        using binder_type::operator();
    };
    template <
        typename F, 
        typename ... ttps_ts,
        typename ... args_ts
    >
    back_binder(F&&, mp::ttps<ttps_ts...>, args_ts&&...) -> back_binder<std::decay_t<F>, mp::ttps<ttps_ts...>, mp::args<std::decay_t<args_ts>...>>;
    template <typename F, typename ... args_ts>
    back_binder(F&&, args_ts&&...) -> back_binder<std::decay_t<F>, mp::ttps<>, mp::args<std::decay_t<args_ts>...>>;

    // binder_front
    //  same as `std::bind_front`, but also bound/allow ttps
    //  (waiting for proposal p1985 to extend this to nttps ...)
    template <typename ... ttps_bounded_ts, typename F, typename ... args_bounded_ts>
    constexpr auto bind_front(F&& f, args_bounded_ts && ... args) {
        // front_binder factory.
        // produces the same behavior as std::bind_front (cvref-qualifiers correctness)
        using bind_front_t = front_binder<
            std::remove_cvref_t<F>,
            mp::ttps<ttps_bounded_ts...>,
            mp::args<std::remove_cvref_t<args_bounded_ts>...>
        >;
        return bind_front_t{
            std::forward<F>(f),
            std::forward<args_bounded_ts>(args)...
        };
    }
    template <typename ... ttps_bounded_ts, typename F, typename ... args_bounded_ts>
    constexpr auto bind_front(F&& f, mp::ttps<ttps_bounded_ts...>, args_bounded_ts && ... args) {
        return bind_front<ttps_bounded_ts...>(fwd(f), fwd(args)...);
    }
    // bind_back
    //  same as `std::bind_back`, but also bound/allow ttps
    //  (waiting for proposal p1985 to extend this to nttps ...)
    template <typename ... ttps_bounded_ts, typename F, typename ... args_bounded_ts>
    constexpr auto bind_back(F&& f, args_bounded_ts && ... args) {
        // back_binder factory.
        // produces the same behavior as std::bind_back (cvref-qualifiers correctness)
        using bind_back_t = back_binder<
            std::remove_cvref_t<F>,
            mp::ttps<ttps_bounded_ts...>,
            mp::args<std::remove_cvref_t<args_bounded_ts>...>
        >;
        return bind_back_t{
            std::forward<F>(f),
            std::forward<args_bounded_ts>(args)...
        };
    }
    template <typename ... ttps_bounded_ts, typename F, typename ... args_bounded_ts>
    constexpr auto bind_back(F&& f, mp::ttps<ttps_bounded_ts...>, args_bounded_ts && ... args) {
        return bind_back<ttps_bounded_ts...>(fwd(f), fwd(args)...);
    }    
}
// function_view
// function_ref
namespace csl::wf {
    // function_view
    template <typename F> requires (not std::is_reference_v<F>)
    struct function_view {
        
        using type = F;

        constexpr explicit function_view(auto && value)
        noexcept(std::is_nothrow_constructible_v<F, decltype(value)>)
        requires (not std::same_as<function_view, std::remove_cvref_t<decltype(value)>>)
        : storage{ std::forward<F&>(value) }
        {
            if constexpr (std::equality_comparable_with<decltype(value), decltype(nullptr)>) {
                if (value == nullptr)
                    throw std::invalid_argument{"function_view<T> : nullptr"};
            }
        }
        constexpr function_view(const function_view & other) noexcept = default;
        constexpr function_view(function_view &&) noexcept = default;
        constexpr function_view() = default;
        constexpr ~function_view() = default;
        constexpr function_view & operator=(function_view &&) noexcept = default;
        constexpr function_view & operator=(const function_view &) noexcept = default;

        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) &
        noexcept(csl::wf::mp::is_nothrow_invocable_v<F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<F&>(storage), args...);
        }
        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) &&
        noexcept(csl::wf::mp::is_nothrow_invocable_v<F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<F&&>(storage), args...);
        }
        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) const &
        noexcept(csl::wf::mp::is_nothrow_invocable_v<const F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <const F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<const F&>(storage), args...);
        }
        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) const &&
        noexcept(csl::wf::mp::is_nothrow_invocable_v<const F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <const F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<const F&&>(storage), args...);
        }

        // storage accessors
        constexpr explicit operator const F&() const noexcept {
            return storage;
        }
        constexpr explicit operator F&() noexcept {
            return storage;
        }
        constexpr const F& get() const noexcept {
            return storage;
        }
        constexpr F& get() noexcept {
            return storage;
        }

    private:
        std::add_lvalue_reference_t<F> storage;
    };
    template <typename F>
    function_view(F&) -> function_view<F>;
    template <typename F>
    function_view(F&&) -> function_view<F>;

    // function ref
    // - improvement to std::reference_wrapper::operator()
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0792r2.html
    template <typename F> requires (not std::is_reference_v<F>)
    struct function_ref {
        
        using type = F;

        constexpr explicit function_ref(auto && value)
        noexcept(std::is_nothrow_constructible_v<storage_type, decltype(std::addressof(value))>)
        requires (not std::same_as<function_ref, std::remove_cvref_t<decltype(value)>>)
        : storage{ std::addressof(std::forward<F&>(value)) }
        {
            if constexpr (std::equality_comparable_with<decltype(value), decltype(nullptr)>) {
                if (value == nullptr)
                    throw std::invalid_argument{"function_ref<T> : nullptr"};
            }
        }
        constexpr function_ref(const function_ref & other) noexcept = default;
        constexpr function_ref(function_ref &&) noexcept = default;
        constexpr function_ref() = delete;
        constexpr ~function_ref() = default;

        constexpr function_ref & operator=(function_ref &&) noexcept = default;
        constexpr function_ref & operator=(const function_ref &) noexcept = default;

        constexpr void swap(function_ref & other) noexcept {
            std::swap(storage, other.storage);
        }

        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) &
        noexcept(csl::wf::mp::is_nothrow_invocable_v<F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<F&>(*storage), args...);
        }
        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) &&
        noexcept(csl::wf::mp::is_nothrow_invocable_v<F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<F&&>(*storage), args...);
        }
        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) const &
        noexcept(csl::wf::mp::is_nothrow_invocable_v<const F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <const F&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<const F&>(*storage), args...);
        }
        template <typename ... ttps_args>
        constexpr decltype(auto) operator()(auto && ... args) const &&
        noexcept(csl::wf::mp::is_nothrow_invocable_v<const F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>)
        requires csl::wf::mp::is_invocable_v        <const F&&, csl::wf::mp::ttps<ttps_args...>, decltype(args)...>
        {
            return csl::wf::invoke<ttps_args...>(std::forward<const F&&>(*storage), args...);
        }

        // storage accessors
        constexpr explicit operator const F&() const noexcept {
            return *storage;
        }
        constexpr explicit operator F&() noexcept {
            return *storage;
        }
        constexpr const F& get() const noexcept {
            return *storage;
        }
        constexpr F& get() noexcept {
            return *storage;
        }

    private:
        using storage_type = std::add_pointer_t<F>;
        storage_type storage = nullptr;
    };
    template <typename F>
    function_ref(F&) -> function_ref<F>;
    template <typename F>
    function_ref(F&&) -> function_ref<F>;

    template <typename F>
    constexpr void swap(function_ref<F> & lhs, function_ref<F> & rhs) {
        lhs.swap(rhs);
    }
}
// mp::are_unique_v<Ts..>
// mp::is_instance_of<pack<...>, T>
// mp::unfold_to<dest<...>, T>
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

    template <typename T, template <typename...> typename type>
    concept InstanceOf = is_instance_of_v<type, std::remove_cvref_t<T>>;
    template <typename T, template <typename...> typename type>
    concept NotInstanceOf = not is_instance_of_v<type, std::remove_cvref_t<T>>;

    // unfold_to
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_to : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_to<destination, from<Ts...>> : unfold_to<destination, Ts...>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_to_t = typename unfold_to<destination, from...>::type;

    // unfold_tuple_to
    template <template <typename...> typename destination, concepts::tuple_interface T>
    struct unfold_tuple_to {
        using type = typename decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            using result_t = destination<std::tuple_element_t<indexes, T>...>;
            return std::type_identity<result_t>{};
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{}))::type;
    };
    template <template <typename...> typename destination, concepts::tuple_interface T>
    using unfold_tuple_to_t = typename unfold_tuple_to<destination, T>::type;
}
// invocation utilities
// - apply_with_or_discard
// - invoke_into_tuple(_t)
// - node_invoke
// - node_invoke_result(_t)
namespace csl::wf::details {
    // todo :
    // - multiples return values => csl::wf::mp::args<...> with tuple-like storage ? (or strong-type for std::tuple...)

    // apply_with_or_discard
    // TODO : remove ?
    template <typename F, csl::wf::concepts::tuple_interface tuple_type>
    requires
        wf::mp::is_applyable_v<F&&, tuple_type&&>
    constexpr decltype(auto) apply_with_or_discard(F && functor, tuple_type && args_as_tuple)
    noexcept(wf::mp::is_nothrow_applyable_v<F&&, tuple_type&&>)
    {
        return wf::apply(fwd(functor), fwd(args_as_tuple));
    }
    template <typename F, csl::wf::concepts::tuple_interface tuple_type>
    requires
        (not wf::mp::is_applyable_v<F&&, tuple_type&&>)
        and wf::mp::is_invocable_v<F&&>
    constexpr decltype(auto) apply_with_or_discard(F && functor, tuple_type &&)
    noexcept(wf::mp::is_nothrow_invocable_v<F&&>)
    {
        // static_assert(wf::mp::is_invocable_v<F&&>, "csl::wf::details::apply_with_or_discard : invalid overload (fallback : discard scenario)");
        return wf::invoke(fwd(functor));
    }
    // constexpr decltype(auto) apply_with_or_discard(auto &&, auto &&) {
    //     static_assert([](){ return false; }(), "csl::wf::details::apply_with_or_discard : invalid overload");
    // }
    
    // invoke_into_tuple
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
    template <typename F, typename ... args_ts>
    requires requires {
        invoke_into_tuple(std::declval<F>(), std::declval<args_ts...>());
    }
    using invoke_into_tuple_result_t = decltype(invoke_into_tuple(std::declval<F>(), std::declval<args_ts...>()));
}

#pragma region route/chain
// invocation solver, match policy (allow_discard or nodiscard)
namespace csl::wf::details {
    template <typename policy>
    struct invoke_solver {

        constexpr static auto node_invoke(auto && f, auto && args)
        noexcept (csl::wf::mp::is_nothrow_applyable_v<decltype(f), decltype(fwd(args))>)
        requires policy::template should_apply_v<decltype(f), decltype(fwd(args))>
        {
            return details::invoke_into_tuple([&](){
                return csl::wf::apply(fwd(f), fwd(args));
            });
            }
        constexpr static auto node_invoke(auto && f, auto && args)
        noexcept (csl::wf::mp::is_nothrow_invocable_v<decltype(f)>)
        requires policy::template should_invoke_v<decltype(f), decltype(args)>
        {   // discard args
            return details::invoke_into_tuple([&](){
                return csl::wf::invoke(fwd(f));
            });
        }
    };
}
// unfold operators helper (for evaluation only, never defined)
// TODO : ttps<...> cannot be discard
namespace csl::wf::details::invoke_policy::allow_discard {

    struct identity {
        template <typename F, typename args_ts>
        constexpr static bool should_invoke_v = csl::wf::mp::is_invocable_v<F> and
        (
            csl::wf::concepts::tuple_empty<args_ts> or
            (
                csl::wf::concepts::tuple_not_empty<args_ts> and
                (not csl::wf::mp::is_applyable_v<F, args_ts>)
            )
        );
        template <typename F, typename args_ts>
        constexpr static bool should_apply_v = csl::wf::concepts::tuple_not_empty<args_ts> and csl::wf::mp::is_applyable_v<F, args_ts>;
    };

    namespace unfold_operators::eval {
        // evaluation only, should not be call
        constexpr auto operator>>(auto && args, auto && f)
        noexcept (csl::wf::mp::is_nothrow_applyable_v<decltype(f), decltype(args)>)
        requires identity::template should_apply_v<decltype(f), decltype(args)>
        {
            return csl::wf::mp::tuple_optional_t<csl::wf::mp::apply_result_t<decltype(f), decltype(args)>>{};
        }
        constexpr auto operator>>(auto && args, auto && f)
        noexcept (csl::wf::mp::is_nothrow_invocable_v<decltype(f)>)
        requires identity::template should_invoke_v<decltype(f), decltype(args)>
        {
            return csl::wf::mp::tuple_optional_t<csl::wf::mp::invoke_result_t<decltype(f)>>{};
        }
    }
    namespace unfold_operators {

        using invoke_solver_t = csl::wf::details::invoke_solver<identity>;

        constexpr auto operator>>=(csl::wf::concepts::tuple_interface auto && args, auto && f)
        noexcept(noexcept(invoke_solver_t::template node_invoke(fwd(f), fwd(args))))
        requires requires{invoke_solver_t::template node_invoke(fwd(f), fwd(args));}
        {
            return invoke_solver_t::template node_invoke(fwd(f), fwd(args));
        }
    }
}
namespace csl::wf::details::invoke_policy::nodiscard {

    struct identity {
        template <typename F, typename args_ts>
        constexpr static bool should_invoke_v = csl::wf::concepts::tuple_empty<args_ts> and csl::wf::mp::is_invocable_v<F>;
        template <typename F, typename args_ts>
        constexpr static bool should_apply_v = csl::wf::concepts::tuple_not_empty<args_ts> and csl::wf::mp::is_applyable_v<F, args_ts>;
    };

    namespace unfold_operators::eval {
        // evaluation only, should not be call
        constexpr auto operator>>(auto && args, auto && f)
        noexcept (csl::wf::mp::is_nothrow_applyable_v<decltype(f), decltype(fwd(args))>)
        requires identity::template should_apply_v<decltype(f), decltype(fwd(args))>
        {
            return csl::wf::mp::tuple_optional_t<csl::wf::mp::apply_result_t<decltype(f), decltype(args)>>{};
        }
        constexpr auto operator>>(auto && args, auto && f)
        noexcept (csl::wf::mp::is_nothrow_invocable_v<decltype(f)>)
        requires identity::template should_invoke_v<decltype(f), decltype(args)>
        {
            return csl::wf::mp::tuple_optional_t<csl::wf::mp::invoke_result_t<decltype(f)>>{};
        }
    }
    namespace unfold_operators {

        using invoke_solver_t = csl::wf::details::invoke_solver<identity>;

        constexpr auto operator>>=(csl::wf::concepts::tuple_interface auto && args, auto && f)
        noexcept(noexcept(invoke_solver_t::template node_invoke(fwd(f), fwd(args))))
        requires requires{invoke_solver_t::template node_invoke(fwd(f), fwd(args));}
        {
            return invoke_solver_t::template node_invoke(fwd(f), fwd(args));
        }
    }
}
// public invoke policies
namespace csl::wf::invoke_policy {
    using allow_discard = csl::wf::details::invoke_policy::allow_discard::identity;
    using nodiscard = csl::wf::details::invoke_policy::nodiscard::identity;
}

// chain traits (details)
// - is_chain_invocable
// - is_chain_nodiscard_invocable
// - is_chain_nothrow_invocable
// - is_chain_nothrow_nodiscard_invocable
// - chain_invoke_result
// - chain_invoke_nodiscard_result
//
// todo : multiple args (forward_as_tuple or args_ts<...>)
// todo : merge expansion lambdas ?
namespace csl::wf::details::mp {

    // is_chain_invocable
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    struct is_chain_invocable {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>) constexpr {
            namespace invoke_policy = details::invoke_policy::allow_discard;
            using namespace invoke_policy::unfold_operators::eval;
            return requires { (std::declval<Args>() >> ... >> std::get<indexes>(std::declval<Fs>())); };
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Fs>>>{});
    };
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    constexpr bool is_chain_invocable_v = is_chain_invocable<Fs, Args>::value;

    // is_chain_nodiscard_invocable
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    struct is_chain_nodiscard_invocable {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            namespace invoke_policy = details::invoke_policy::nodiscard;
            using namespace invoke_policy::unfold_operators::eval;
            return requires { (std::declval<Args>() >> ... >> std::get<indexes>(std::declval<Fs>())); };
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Fs>>>{});
    };
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    constexpr bool is_chain_nodiscard_invocable_v = is_chain_nodiscard_invocable<Fs, Args>::value;

    // is_chain_nothrow_invocable
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    struct is_chain_nothrow_invocable {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            namespace invoke_policy = details::invoke_policy::allow_discard;
            using namespace invoke_policy::unfold_operators::eval;
            if constexpr (not is_chain_invocable_v<Fs, Args>)
                return false;
            else
                return noexcept((std::declval<Args>() >> ... >> std::get<indexes>(std::declval<Fs>())));

        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Fs>>>{});
    };
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    constexpr bool is_chain_nothrow_invocable_v = is_chain_nothrow_invocable<Fs, Args>::value;

    // is_chain_nothrow_nodiscard_invocable
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    struct is_chain_nothrow_nodiscard_invocable {
        constexpr static bool value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            namespace invoke_policy = details::invoke_policy::nodiscard;
            using namespace invoke_policy::unfold_operators::eval;
            if constexpr (not is_chain_nodiscard_invocable_v<Fs, Args>)
                return false;
            else
                return noexcept((std::declval<Args>() >> ... >> std::get<indexes>(std::declval<Fs>())));

        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Fs>>>{});
    };
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    constexpr bool is_chain_nothrow_nodiscard_invocable_v = is_chain_nothrow_nodiscard_invocable<Fs, Args>::value;

    // chain_invoke_result
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    requires is_chain_invocable_v<Fs, Args>
    class chain_invoke_result {
        using tuple_optional_type = typename decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            namespace invoke_policy = details::invoke_policy::allow_discard;
            using namespace invoke_policy::unfold_operators::eval;
            return std::type_identity<
                decltype((std::declval<Args>() >> ... >> std::get<indexes>(std::declval<Fs>())))
            >{};
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Fs>>>{}))::type;

    public:
        using type = csl::wf::mp::tuple_optional_underlying_t<tuple_optional_type>;
    };
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    using chain_invoke_result_t = typename chain_invoke_result<Fs, Args>::type;

    // chain_invoke_nodiscard_result
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    requires is_chain_nodiscard_invocable_v<Fs, Args>
    class chain_invoke_nodiscard_result {
        using tuple_optional_type = typename decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            namespace invoke_policy = details::invoke_policy::nodiscard;
            using namespace invoke_policy::unfold_operators::eval;
            return std::type_identity<
                decltype((std::declval<Args>() >> ... >> std::get<indexes>(std::declval<Fs>())))
            >{};
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Fs>>>{}))::type;
    public:
        using type = csl::wf::mp::tuple_optional_underlying_t<tuple_optional_type>;
    };
    template <
        csl::wf::concepts::tuple_interface Fs,
        csl::wf::concepts::tuple_interface Args
    >
    using chain_invoke_nodiscard_result_t = typename chain_invoke_nodiscard_result<Fs, Args>::type;
}
// chain traits
namespace csl::wf::mp {
    template <typename ... Fs>
    struct chain_trait {

        using types = std::tuple<std::add_rvalue_reference_t<Fs>...>;
        static_assert(concepts::tuple_view<types>);

        template <typename ... args_ts>
        constexpr static bool is_invocable = details::mp::is_chain_invocable_v<types, std::tuple<args_ts...>>;
        template <typename ... args_ts>
        constexpr static bool is_nothrow_invocable = details::mp::is_chain_nothrow_invocable_v<types, std::tuple<args_ts...>>;
        template <typename ... args_ts>
        constexpr static bool is_nodiscard_invocable = details::mp::is_chain_nodiscard_invocable_v<types, std::tuple<args_ts...>>;
        template <typename ... args_ts>
        constexpr static bool is_nothrow_nodiscard_invocable = details::mp::is_chain_nothrow_nodiscard_invocable_v<types, std::tuple<args_ts...>>;

        template <typename ... args_ts>
        using invoke_result_t = details::mp::chain_invoke_result_t<types, std::tuple<args_ts...>>;
        template <typename ... args_ts>
        using nodiscard_invoke_result_t = details::mp::chain_invoke_nodiscard_result_t<types, std::tuple<args_ts...>>;
    };
}
// chain_invoke / route
// TODO :
//  always owning ? mix owning and non-owning functors in storage ?
//  if non-owning, value semantic correctness for route/binder storage
namespace csl::wf {

    // TODO : ttps
    // TODO : use csl::wf::chain_trait ?
    // TODO : noexcept
    // TODO : execution_policy

    constexpr auto chain_invoke(
        csl::wf::concepts::tuple_interface auto && funcs,
        csl::wf::concepts::tuple_interface auto && args
    )
    noexcept(csl::wf::details::mp::is_chain_nothrow_invocable_v<decltype(funcs), decltype(args)>)
    requires csl::wf::details::mp::is_chain_invocable_v<decltype(funcs), decltype(args)>
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            using namespace details::invoke_policy::allow_discard::unfold_operators;
            concepts::tuple_optional auto result = (fwd(args) >>= ... >>= std::get<indexes>(fwd(funcs))); // Error : incompatible argument
            
            constexpr auto result_size = std::tuple_size_v<std::remove_cvref_t<decltype(result)>>;
            if constexpr (result_size not_eq 0)
                return std::get<0>(result);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(funcs)>>>{});
    }
    constexpr auto chain_invoke_nodiscard(
        csl::wf::concepts::tuple_interface auto && funcs,
        csl::wf::concepts::tuple_interface auto && args
    )
    noexcept(csl::wf::details::mp::is_chain_nothrow_nodiscard_invocable_v<decltype(funcs), decltype(args)>)
    requires csl::wf::details::mp::is_chain_nodiscard_invocable_v<decltype(funcs), decltype(args)>
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            using namespace details::invoke_policy::nodiscard::unfold_operators;
            concepts::tuple_optional auto result = (fwd(args) >>= ... >>= std::get<indexes>(fwd(funcs))); // Error : incompatible argument
            
            constexpr auto result_size = std::tuple_size_v<std::remove_cvref_t<decltype(result)>>;
            if constexpr (result_size not_eq 0)
                return std::get<0>(result);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(funcs)>>>{});
    }
    // todo : use this instead of requires requires { chain_invoke }
    template <typename ... Ts>
    concept chain_invocable = requires {
        chain_invoke(std::declval<Ts>()...);
    };

    // todo : owning by default, non-owning (opt-out) policy ?
    // todo : allow `sizeof...(Fs) == 0` ? -> with no valid operator() candidate then
    template <typename ... Fs>
    struct route {
        static_assert(sizeof...(Fs) not_eq 0,               "csl::wf::binder : binds nothing");
        static_assert((not std::is_reference_v<Fs> && ...), "csl::wf::binder : non-owning");

        using storage_type = std::tuple<Fs...>;

        constexpr explicit route(auto && ... args)
        : storage{ fwd(args)... }
        {}

        // todo : ttps
        #pragma region operator()
        constexpr decltype(auto) operator()(auto && ... args) &
        noexcept(details::mp::is_chain_nothrow_invocable_v<
            mp::tuple_view_t<storage_type&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >)
        requires details::mp::is_chain_invocable_v<
            mp::tuple_view_t<storage_type&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >
        {
            return chain_invoke(
                mp::make_tuple_view(storage),
                std::forward_as_tuple(fwd(args)...)
            );
        }

        constexpr decltype(auto) operator()(auto && ... args) &&
        noexcept(details::mp::is_chain_nothrow_invocable_v<
            mp::tuple_view_t<storage_type &&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >)
        requires details::mp::is_chain_invocable_v<
            mp::tuple_view_t<storage_type &&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >
        {
            return chain_invoke(
                mp::make_tuple_view(fwd(storage)),
                std::forward_as_tuple(fwd(args)...)
            );
        }

        constexpr decltype(auto) operator()(auto && ... args) const &
        noexcept(details::mp::is_chain_nothrow_invocable_v<
            mp::tuple_view_t<const storage_type&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >)
        requires details::mp::is_chain_invocable_v<
            mp::tuple_view_t<const storage_type&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >
        {
            return chain_invoke(
                mp::make_tuple_view(static_cast<const storage_type &>(storage)),
                std::forward_as_tuple(fwd(args)...)
            );
        }

        constexpr decltype(auto) operator()(auto && ... args) const &&
        noexcept(details::mp::is_chain_nothrow_invocable_v<
            mp::tuple_view_t<const storage_type&&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >)
        requires details::mp::is_chain_invocable_v<
            mp::tuple_view_t<const storage_type&&>,
            decltype(std::forward_as_tuple(fwd(args)...))
        >
        {
            return chain_invoke(
                mp::make_tuple_view(static_cast<const storage_type &&>(storage)),
                std::forward_as_tuple(fwd(args)...)
            );
        }
        #pragma endregion

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

    template <std::size_t index, csl::wf::details::mp::InstanceOf<csl::wf::route> T>
    constexpr decltype(auto) get(T && value) noexcept {
        return fwd(value).template at<index>();
    }
}
#pragma endregion

// flattening utility
// - mp::flatten_of_t<pack<...>, Ts...>
// - mp::unfold_super_into<pack<...>, Ts...>
// - mp::make_flatten_super(super<...>, Ts&&...)
namespace csl::wf::details::mp {

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
    template <
        template <typename ...> typename destination_type,
        csl::wf::details::mp::InstanceOf<destination_type> T
    >
    constexpr decltype(auto) unfold_super_into(T && value) {
        return fwd(value);
        // return destination_type<Ts&&...>(static_cast<Ts&&>(value)...);
    }
    // TODO : refactoring : consider merging theses function + nested lambda to deal with cvref-qualifiers
    template <template <typename ...> typename destination_type, template <typename ...> typename origin_type, typename ... Ts>
    constexpr decltype(auto) unfold_super_into(origin_type<Ts...> && value) {
        return destination_type<Ts&&...>(static_cast<Ts&&>(value)...);
    }
    template <template <typename ...> typename destination_type, template <typename ...> typename origin_type, typename ... Ts>
    constexpr decltype(auto) unfold_super_into(origin_type<Ts...> & value) {
        return destination_type<Ts&...>(static_cast<Ts&>(value)...);
    }
    template <template <typename ...> typename destination_type, template <typename ...> typename origin_type, typename ... Ts>
    constexpr decltype(auto) unfold_super_into(const origin_type<Ts...> && value) {
        return destination_type<const Ts&&...>(static_cast<const Ts&&>(value)...);
    }
    template <template <typename ...> typename destination_type, template <typename ...> typename origin_type, typename ... Ts>
    constexpr decltype(auto) unfold_super_into(const origin_type<Ts...> & value) {
        return destination_type<const Ts&...>(static_cast<const Ts&>(value)...);
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
    template <
        template <typename...> typename destination_type,
        csl::wf::details::mp::InstanceOf<csl::wf::route> T
    >
    constexpr auto fwd_nodes_into(T && value) {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return destination_type<typename T::template node_t<indexes>&&...>{
                csl::wf::get<indexes>(fwd(value))...
            };
        }(std::make_index_sequence<T::size>{});
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

#pragma region repeat
// literals
namespace csl::wf::details::literals {
    template <std::integral T>
    constexpr auto char_to_b10_integral(char value) -> T
    {
        if (value < '0' or value > '9')
            throw std::out_of_range("not decimal value");
        return value - '0';
    }

    template <std::integral T>
    constexpr auto char_pack_to_integral(auto ... values) -> T
    {
        static_assert(std::numeric_limits<T>::digits10 > sizeof...(values), "out of numerical range");
        T result{ 0 };
        ((result = result * 10 + (values - '0')), ...); //NOLINT
        return result;
    }
}
// literals (compile-time) 123_times
namespace csl::wf::literals {
    template <auto value>
    using times = std::integral_constant<decltype(value), value>;

    template <char... chars_values>
    constexpr auto operator"" _times() -> times<details::literals::char_pack_to_integral<std::size_t>(chars_values...)>
    {
        return {};
    }
}
// repeat
// - invoke_n_times
// - repeater
namespace csl::wf {
    // invoke_n_times
    template <auto times, typename ... ttps_args, typename F, typename ... Ts>
    requires(not csl::wf::mp::is_invocable_v<F&&, mp::ttps<ttps_args...>, Ts &&...>) 
    constexpr decltype(auto) invoke_n_times(F && func, Ts && ... args) = delete;

    template <auto times, typename ... ttps_args, typename F, typename ... Ts>
    constexpr decltype(auto) invoke_n_times(F && func, Ts && ... args)
    noexcept(noexcept(csl::wf::invoke(fwd(func), fwd(args)...)))
    requires(std::is_void_v<csl::wf::mp::invoke_result_t<F&&, mp::ttps<ttps_args...>, Ts&&...>>)
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((static_cast<void>(indexes), csl::wf::invoke<ttps_args...>(fwd(func), fwd(args)...)), ...);
        }(std::make_index_sequence<times>{});
    }
    
    template <auto times, typename ... ttps_args, typename F, typename ... Ts>
    constexpr decltype(auto) invoke_n_times(F && func, Ts && ... args)
    noexcept(noexcept(csl::wf::invoke<ttps_args...>(fwd(func), fwd(args)...)))
    {
        using invoke_result_t = csl::wf::mp::invoke_result_t<F&&, mp::ttps<ttps_args...>, decltype(args)...>;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return std::array<invoke_result_t, times>{
                (static_cast<void>(indexes), csl::wf::invoke<ttps_args...>(fwd(func), fwd(args)...))...
            };
        }(std::make_index_sequence<times>{});
    }

    // repeater
    //  todo : cx vs. rt => + type erasure
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
        friend struct repeater;

        template <auto other_times, typename other_F>
        explicit constexpr repeater(repeater<other_times, other_F> && func)
        : storage{ std::move(func.storage) }
        {
            static_assert(std::same_as<decltype(other_times), decltype(times_)>);
        }
        template <auto other_times, typename other_F>
        explicit constexpr repeater(const repeater<other_times, other_F> & func)
        : storage{ func.storage }
        {
            static_assert(std::same_as<decltype(other_times), decltype(times_)>);
        }
    #pragma endregion

        template <typename ... ttps_args, typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) &
        noexcept(noexcept(invoke_n_times<times, ttps_args...>(std::declval<F&>(), fwd(args)...)))
        requires requires { invoke_n_times<times, ttps_args...>(std::declval<F&>(), fwd(args)...); }
        {
            return invoke_n_times<times, ttps_args...>(storage, fwd(args)...);
        }
        template <typename ... ttps_args, typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) const &
        noexcept(noexcept(invoke_n_times<times, ttps_args...>(std::declval<const F&>(), fwd(args)...)))
        requires requires { invoke_n_times<times, ttps_args...>(std::declval<const F&>(), fwd(args)...); }
        {
            return invoke_n_times<times, ttps_args...>(storage, fwd(args)...);
        }
        template <typename ... ttps_args, typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) &&
        noexcept(noexcept(invoke_n_times<times, ttps_args...>(std::declval<F&&>(), fwd(args)...)))
        requires requires { invoke_n_times<times, ttps_args...>(std::declval<F&&>(), fwd(args)...); }
        {
            return invoke_n_times<times, ttps_args...>(std::move(storage), fwd(args)...);
        }
        template <typename ... ttps_args, typename ... args_ts>
        constexpr decltype(auto) operator()(args_ts && ... args) const &&
        noexcept(noexcept(invoke_n_times<times, ttps_args...>(std::declval<const F&&>(), fwd(args)...)))
        requires requires { invoke_n_times<times, ttps_args...>(std::declval<const F&&>(), fwd(args)...); }
        {
            return invoke_n_times<times, ttps_args...>(std::move(storage), fwd(args)...);
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
        static constexpr auto make(F && arg)
        {
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
#pragma endregion

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
    requires (csl::wf::details::mp::is_instance_of_v<csl::wf::route, std::remove_cvref_t<Ts>> or ...)
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
    requires (csl::wf::details::mp::is_instance_of_v<csl::wf::details::overload, std::remove_cvref_t<Ts>> or ...)
    constexpr auto make_condition(Ts && ... nodes) {
        return csl::wf::details::mp::make_flatten_super<csl::wf::details::overload>(
            fwd(nodes)...
        );
    }

    // make_repetition
    // todo :   3_times (user-defined literals)
    //          times(3) tag for rt::repeater factory
    // or : repeater(N) -> repeater<N> with implicit cast to integral constant ?
    template <auto times, typename F>
    constexpr decltype(auto) make_repetition(F && func) {
        return repeater_factory<times>::make(fwd(func));
    }
}
// eDSL
namespace csl::wf::operators {
    // todo : protect injection against overload ambiguities

    // factories ------------------------------------------
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

    // operator*
    // todo : integral_constant::value ?
    template <typename lhs_t, auto times>
    constexpr auto operator*(lhs_t && lhs, std::integral_constant<decltype(times), times>) {
        return make_repetition<times>(fwd(lhs));
    }

    // views/refs -----------------------------------------
    struct ref_tag_t{}   constexpr ref;
    struct cref_tag_t{}  constexpr cref;
    struct view_tag_t{}  constexpr view;

    // ref
    template <details::mp::NotInstanceOf<csl::wf::function_ref> F>
    constexpr auto operator|(F && value, const ref_tag_t &)
    noexcept(std::is_nothrow_constructible_v<csl::wf::function_ref<std::remove_reference_t<F>>, F&&>)
    requires(std::is_constructible_v<csl::wf::function_ref<std::remove_reference_t<F>>, F&&>)
    {
        return csl::wf::function_ref<std::remove_reference_t<F>>{
            std::forward<F>(value)
        };
    }
    template <
        details::mp::InstanceOf<csl::wf::function_ref> F,
        typename reference_tag_t
    >
    constexpr auto operator|(F && value, const reference_tag_t &) noexcept
    requires (
        std::same_as<reference_tag_t, ref_tag_t> or
        std::same_as<reference_tag_t, cref_tag_t>
    )
    {
        return csl::wf::function_ref{ std::forward<F>(value) };
    }

    // cref
    template <details::mp::NotInstanceOf<csl::wf::function_ref> F>
    constexpr auto operator|(F && value, const cref_tag_t &)
    noexcept(std::is_nothrow_constructible_v<csl::wf::function_ref<const std::remove_reference_t<F>>, const F&&>)
    requires(std::is_constructible_v<csl::wf::function_ref<const std::remove_reference_t<F>>, const F&&>)
    {
        return csl::wf::function_ref<const std::remove_reference_t<F>>{
            std::forward<const F>(value)
        };
    }

    // view
    template <details::mp::NotInstanceOf<csl::wf::function_view> F>
    constexpr auto operator|(F && value, const view_tag_t &)
    noexcept(std::is_nothrow_constructible_v<csl::wf::function_view<std::remove_reference_t<F>>, F&&>)
    requires(std::is_constructible_v<csl::wf::function_view<std::remove_reference_t<F>>, F&&>)
    {
        return csl::wf::function_view<std::remove_reference_t<F>>{
            std::forward<F>(value)
        };
    }
    template <details::mp::InstanceOf<csl::wf::function_view> F>
    constexpr auto operator|(F && value, const view_tag_t &) noexcept
    {
        return csl::wf::function_view{ std::forward<F>(value) };
    }
}

// namespace aggregation
namespace csl::wf {
    using namespace csl::wf::mp;
}

#undef fwd

// todo : function should be able to return a mp::ttps + return value ?
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
 