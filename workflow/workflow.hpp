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

#ifdef CPP_SHELVE_STANDALONE_EDIT__
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

    template <template <typename> typename first_trait, template <typename> typename... traits>
    struct merge_traits_t {
        template <typename T>
        using type = typename merge_traits_t<traits...>::template type<first_trait<T>>;
    };
    template <template <typename> typename first_trait>
    struct merge_traits_t<first_trait> {
        template <typename T>
        using type = first_trait<T>;
    };

    template <template <typename...> class base_type, typename... Ts>
    class partial {
        // differs type instanciation with partial template-type parameters
        template <typename ... Us>
        struct impl {
            // workaround for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59498
            using type = base_type<Ts..., Us...>;
        };
        template <typename U>
        struct impl<U> {
            using type = base_type<Ts..., U>;
        };

    public:
        template <typename... Us>
        requires (sizeof...(Us) >= 1)
        using type = impl<Us...>::type;
    };
    template <template <typename...> class base_type, typename... Ts>
    struct partial_t {
        template <typename... Us>
        using type = typename partial<base_type, Ts...>::type<Us...>::type;
    };

    template <typename ... ttps>
    struct ttps_pack {};
}
namespace workflow::mp::transpose_qualifier {
    template <typename from_type, typename to_type>
    using const_t = std::conditional_t<
        std::is_const_v<std::remove_reference_t<from_type>>,
        std::add_const_t<to_type>,
        std::type_identity_t<to_type>
    >;
    template <typename from_type, typename to_type>
    using volatile_t = std::conditional_t<
        std::is_volatile_v<std::remove_reference_t<from_type>>,
        std::add_volatile_t<to_type>,
        std::type_identity_t<to_type>
    >;
    template <typename from_type, typename to_type>
    using lvalue_ref_t = std::conditional_t<
        std::is_lvalue_reference_v<from_type>,
        std::add_lvalue_reference_t<to_type>,
        std::type_identity_t<to_type>
    >;
    template <typename from_type, typename to_type>
    using rvalue_ref_t = std::conditional_t<
        std::is_rvalue_reference_v<from_type>,
        std::add_rvalue_reference_t<to_type>,
        std::type_identity_t<to_type>
    >;
    template <typename from_type, typename to_type>
    using cvref_t = typename workflow::mp::merge_traits_t<
        workflow::mp::partial<const_t, from_type>::template type,
        workflow::mp::partial<volatile_t, from_type>::template type,
        workflow::mp::partial<lvalue_ref_t, from_type>::template type,
        workflow::mp::partial<rvalue_ref_t, from_type>::template type
    >::type<to_type>;

    static_assert(std::is_same_v<
        const int &,
        cvref_t<const char &, int>
    >);
}
namespace workflow::mp::cast {
    // does not work :
    // - inaccessible bases
    // - 
    template <typename to_type>
    decltype(auto) static_cast_preserving_cvref(auto && value) {
        using transpose_cv_ref_t = workflow::mp::transpose_qualifier::cvref_t<decltype(value), to_type>;
        return static_cast<transpose_cv_ref_t>(value);
    }
}
namespace workflow::type_traits {

    template <typename T>
    constexpr bool is_ttps_pack_v = false;
    template <typename ... Ts>
    constexpr bool is_ttps_pack_v<ttps_pack<Ts...>> = true;

    template <typename F, typename ... args_t>
    constexpr bool is_invocable_v = std::is_invocable_v<F, args_t...>;
    template <typename F, typename ... ttps_args_t, typename ... args_t>
    constexpr bool is_invocable_v<F, ttps_pack<ttps_args_t...>, args_t...> =
        requires{ std::declval<F>().template operator()<ttps_args_t...>(std::declval<args_t>()...); }
    ;

    template <typename F, typename ... args_t>
    constexpr bool is_nothrow_invocable_v = std::is_nothrow_invocable_v<F, args_t...>;
    template <typename F, typename ... ttps_args_t, typename ... args_t>
    requires
        requires{ std::declval<F>().template operator()<ttps_args_t...>(std::declval<args_t>()...); }
    constexpr bool is_nothrow_invocable_v<F, ttps_pack<ttps_args_t...>, args_t...> =
        noexcept( std::declval<F>().template operator()<ttps_args_t...>(std::declval<args_t>()...))
    ;


    // detection : using type_traits to avoid recursive concepts
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

    template <typename F, typename ... args_t>
    concept invocable = is_invocable_v<F,  args_t...>;

    template <typename F, typename ... args_t>
    concept nothrow_invocable = is_nothrow_invocable_v<F, args_t...>;

    template <typename T>
    concept no_cvref =
        (not std::is_const_v<T>) and
        (not std::is_reference_v<T>) and
        (not std::is_pointer_v<T>)
        ;
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

    // todo : mix TTP/NTTP ... (p1985)
    template <typename ... f_ts, typename F, typename ... f_args_t>
    requires
        (not std::invocable<F, f_args_t...>)
        and mp::invocable<F, mp::ttps_pack<f_ts...>, f_args_t...>
    constexpr decltype(auto) invoke(F && f, f_args_t&& ... args)
    noexcept(mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, f_args_t&&...>)
    {
        return std::forward<F>(f).template operator()<f_ts...>(std::forward<f_args_t>(args)...);
    }
    template <typename F, typename ... f_args_t>
    requires std::invocable<F, f_args_t...>
    constexpr decltype(auto) invoke(F && f, f_args_t&& ... args)
    noexcept (std::is_nothrow_invocable_v<F&&, f_args_t&&...>)
    {
        return std::invoke(std::forward<F>(f), std::forward<f_args_t>(args)...);
    }

    template <typename ... f_ts, typename F, typename args_as_tuple_t>
    constexpr decltype(auto) apply(F && f, args_as_tuple_t&& args)
    noexcept(
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool
        {
            return mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{})
    )
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), std::get<indexes>(std::forward<args_as_tuple_t>(args))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }
    template <typename ... f_ts, typename F, typename args_as_tuple_t, typename ... func_args_t>
    constexpr decltype(auto) apply_before(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args)
    noexcept(
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool
        {
            return mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))..., func_args_t&&...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{})
    )
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, decltype(std::get<indexes>(std::declval<args_as_tuple_t&&>()))..., func_args_t&&...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), std::get<indexes>(std::forward<decltype(args)>(args))..., std::forward<func_args_t>(func_args)...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }
    template <typename ... f_ts, typename F, typename args_as_tuple_t, typename ... func_args_t>
    constexpr decltype(auto) apply_after(F && f, args_as_tuple_t&& args, func_args_t&& ... func_args)
    noexcept(
        []<std::size_t ... indexes>(std::index_sequence<indexes...>) -> bool
        {
            return mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, func_args_t..., decltype(std::get<indexes>(std::forward<decltype(args)>(args)))...>;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{})
    )
    {
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>)
        noexcept(mp::nothrow_invocable<F&&, mp::ttps_pack<f_ts...>, func_args_t&&..., decltype(std::get<indexes>(std::forward<decltype(args)>(args)))...>)
        -> decltype(auto)
        {
            return invoke<f_ts...>(std::forward<F>(f), std::forward<func_args_t>(func_args)..., std::get<indexes>(std::forward<decltype(args)>(args))...);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<args_as_tuple_t>>>{});
    }

    template <typename F, typename ttps_pack_type, typename ... bounded_args_t>
    requires
            std::is_constructible_v<std::decay_t<F>, F>
        and std::is_move_constructible_v<std::decay_t<F>>
        and (std::is_constructible_v<std::decay_t<bounded_args_t>, bounded_args_t> && ...)
        and (std::is_move_constructible_v<std::decay_t<bounded_args_t>> && ...)
    class front_binder;
    template <typename F, typename ... ttps_bounded_args_t, typename ... bounded_args_t>
    class front_binder<F, mp::ttps_pack<ttps_bounded_args_t...>, bounded_args_t...> {
        using bounded_args_storage_type = std::tuple<bounded_args_t...>;
        bounded_args_storage_type bounded_arguments;
        F f;

        using type = front_binder<F, mp::ttps_pack<ttps_bounded_args_t...>, bounded_args_t...>;

    public:
        constexpr front_binder(auto && f_arg, mp::ttps_pack<ttps_bounded_args_t...>, auto && ... args)
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {}
        constexpr front_binder(auto && f_arg, auto && ... args)
        : f{std::forward<decltype(f_arg)>(f_arg)}
        , bounded_arguments{std::forward<decltype(args)>(args)...}
        {
            static_assert(sizeof...(ttps_bounded_args_t) == 0);
        }

        template <typename ... ttps>
        constexpr decltype(auto) operator()(auto && ... parameters) & {
            return apply_before<ttps_bounded_args_t..., ttps...>(f, bounded_arguments, std::forward<decltype(parameters)>(parameters)...);
        }
        template <typename ... ttps>
        constexpr decltype(auto) operator()(auto && ... parameters) const & {
            return apply_before<ttps_bounded_args_t..., ttps...>(f, bounded_arguments, std::forward<decltype(parameters)>(parameters)...);
        }
        template <typename ... ttps>
        constexpr decltype(auto) operator()(auto && ... parameters) && {
            return apply_before<ttps_bounded_args_t..., ttps...>(std::move(f), bounded_arguments, std::forward<decltype(parameters)>(parameters)...);
        }
        template <typename ... ttps>
        constexpr decltype(auto) operator()(auto && ... parameters) const && {
            return apply_before<ttps_bounded_args_t..., ttps...>(std::move(f), bounded_arguments, std::forward<decltype(parameters)>(parameters)...);
        }
    };
    template <typename F, typename ... ttps_bounded_args_t, typename ... bounded_args_t>
    front_binder(F&&, mp::ttps_pack<ttps_bounded_args_t...>, bounded_args_t&&...) -> front_binder<F, mp::ttps_pack<ttps_bounded_args_t...>, bounded_args_t...>;
    template <typename F, typename ... bounded_args_t>
    front_binder(F&&, bounded_args_t&&...) -> front_binder<F, mp::ttps_pack<>, bounded_args_t...>;

    // same as std::bind_front, but also bound/allow ttps (waiting for proposal p1985 to extend this to nttps ...)
    template <typename ... ttps, typename F, typename ... args_t>
    constexpr auto bind_front(F&& f, args_t && ... args) {
        // front_binder factory.
        // produces the same behavior as std::bind_front (cvref-qualifiers correctness)
        using bind_front_t = front_binder<std::remove_cvref_t<F>, ttps_pack<ttps...>, std::remove_cvref_t<args_t>...>;
        return bind_front_t{
            std::forward<F>(f),
            ttps_pack<ttps...>{},
            std::forward<args_t>(args)...
        };
    }

    template <typename ... ttps, typename F>
    constexpr decltype(auto) resolve_overload(F&& f) {
        if constexpr (sizeof...(ttps) not_eq 0) {
            static_assert(requires {
                std::bind_front(&std::remove_reference_t<F>::template operator()<ttps...>, std::declval<F>());
            }, "cannot resolve F::template operator()<ttps...>");
            return std::bind_front(&std::remove_reference_t<F>::template operator()<ttps...>, std::forward<F>(f));
        }
        else {
            return std::forward<F>(f);
        }
    }
    template <typename F, typename ... ttps>
    using overload_solver_t = decltype(resolve_overload<ttps...>(std::declval<F>()));

    template <typename F1, typename F2, typename ... f_args_t>
    requires
        std::invocable<F1, f_args_t&&...> and
        (
            (std::same_as<void, std::invoke_result_t<F1, f_args_t&&...>> and std::invocable<F2>) or
            std::invocable<F2, std::invoke_result_t<F1, f_args_t&&...>>
        )
    constexpr bool are_calls_chainable_v = not
        std::is_same_v<void, std::invoke_result_t<F1, f_args_t&&...>> and
        std::invocable<F2, std::invoke_result_t<F1, f_args_t&&...>>;

    constexpr decltype(auto) merge(auto && first, auto && ... functors) {
        static_assert(sizeof...(functors) not_eq 0);

        return
        [_f = std::forward<decltype(first)>(first), ..._fs = std::forward<decltype(functors)>(functors)]
        <typename ... ttps>
        (auto && ... args) constexpr mutable -> decltype(auto) {
            using namespace workflow::functional;

            auto lhs = bind_front<ttps...>(std::forward<decltype(_f)>(_f), std::forward<decltype(args)>(args)...);
            static_assert(std::invocable<decltype(lhs)>, "overload resolution failed (lhs)");
            auto rhs = merge(std::forward<decltype(_fs)>(_fs)...);

            constexpr bool is_chainable_v = []<typename T, typename ... rest> {
                return std::invocable<T, std::invoke_result_t<decltype(lhs)>>;
            }.template operator()<decltype(_fs)&&...>();

            if constexpr (is_chainable_v) {
                return std::invoke(rhs, std::invoke(lhs));
            }
            else
            {
                static_assert(std::invocable<decltype(rhs)>, "overload resolution failed (rhs)");
                std::invoke(lhs);
                return std::invoke(rhs);
            }
        };
    }
    constexpr decltype(auto) merge(auto && first) {
        return [_f = std::forward<decltype(first)>(first)]
        <typename ... ttps>
        (auto && ... args) constexpr mutable -> decltype(auto) {
            using namespace workflow::functional;
            auto f_caller = bind_front<ttps...>(std::forward<decltype(_f)>(_f), std::forward<decltype(args)>(args)...);
            static_assert(std::invocable<decltype(f_caller)>);
            return f_caller();
        };
    }
}
namespace workflow {

    // is_a vs. has_a for lambda's closure-types
    // Irrelevant :
    // - https://build-bench.com/b/qYzj51Kk-Ejs99R8tqupLEWiMgY
    // - https://quick-bench.com/q/H1RSYkflLWdm1MRmq5vRzUunxwU

    // todo :   specialize then<F1, F2>(then<T, U>&&, X&&)
    //          or just find a (clean) way to do then<Fs...>

    // functor_aggregator : avoid specialization of then<T, T>
    template <typename F1, typename F2>
    requires
        concepts::no_cvref<F1> and
        concepts::no_cvref<F2>
    struct functor_aggregator : F1, F2 {
         
        static_assert (not std::same_as<F1, F2> and not std::derived_from<F1,F2>);

        constexpr functor_aggregator(auto && f1_value, auto && f2_value)
        : F1{ std::forward<decltype(f1_value)>(f1_value) }
        , F2{ std::forward<decltype(f2_value)>(f2_value) }
        {}
    };
    template <typename F>
    requires concepts::no_cvref<F>
    struct functor_aggregator<F, F> : F {
        constexpr functor_aggregator(auto && value, auto && duplicate)
        : F{std::forward<F>(value)}
        {
            if (std::addressof(value) not_eq std::addressof(duplicate))
                throw std::runtime_error{"functor_aggregator<F, F> : impl limitation"};
        }
    };

    template <typename F1, typename F2>
    requires
        concepts::no_cvref<F1> and
        concepts::no_cvref<F2>
    class then : functor_aggregator<F1, F2> {
        // PoC : https://godbolt.org/z/ex1sMMcE9

        using type = then<F1,F2>;
        using functor_aggregator_type = functor_aggregator<F1, F2>;

        template <typename T>
        requires
            std::is_reference_v<T> and
            std::same_as<type, std::remove_cvref_t<T>> 
        using as_F1_t = workflow::mp::transpose_qualifier::cvref_t<T, F1>;

        template <typename T>
        requires
            std::is_reference_v<T> and
            std::same_as<type, std::remove_cvref_t<T>> 
        using as_F2_t = workflow::mp::transpose_qualifier::cvref_t<T, F2>;

        template <typename instance_type, typename ... f1_ts, typename ... f1_args_t>
        requires
            std::is_reference_v<instance_type> and
            std::same_as<type, std::remove_cvref_t<instance_type>> and
            requires {
                functional::invoke(
                    std::declval<as_F2_t<instance_type&&>>(),
                    functional::invoke<as_F1_t<instance_type&&>, f1_ts...>(
                        std::declval<as_F1_t<instance_type&&>>(),
                        std::declval<f1_args_t&&>()...
                    )
                );
            }
        static constexpr decltype(auto) F1_then_F2_fwd(instance_type && instance_value, f1_args_t && ... f1_args_v)
        noexcept(noexcept(
            functional::invoke(
                static_cast<as_F2_t<decltype(instance_value)>>(instance_value), 
                functional::invoke<as_F1_t<instance_type>, f1_ts...>(
                    static_cast<as_F1_t<decltype(instance_value)>>(instance_value), 
                    std::forward<f1_args_t>(f1_args_v)...
                )
            )
        ))
        {
            return functional::invoke(
                static_cast<as_F2_t<decltype(instance_value)>>(instance_value), 
                functional::invoke<as_F1_t<instance_type>, f1_ts...>(
                    static_cast<as_F1_t<decltype(instance_value)>>(instance_value), 
                    std::forward<f1_args_t>(f1_args_v)...
                )
            );
        }

        template <typename instance_type, typename ... f1_ts, typename ... f1_args_t>
        requires
            std::is_reference_v<instance_type> and
            std::same_as<type, std::remove_cvref_t<instance_type>> and
            (not requires {
                // substitution to F1_then_F2_fwd
                // todo : same functions name ?
                F1_then_F2_fwd<instance_type&&, f1_ts...>(std::declval<as_F1_t<instance_type&&>>(), std::declval<f1_args_t&&>()...);
            }) and
            requires {
                functional::invoke(std::declval<as_F2_t<instance_type&&>>());
                functional::invoke<as_F1_t<instance_type&&>, f1_ts...>(
                    std::declval<as_F1_t<instance_type&&>>(),
                    std::declval<f1_args_t&&>()...
                );
            }
        static constexpr decltype(auto) F1_then_F2_no_fwd(instance_type && instance_value, f1_args_t && ... f1_args_v)
        noexcept(noexcept(
            functional::invoke<as_F1_t<instance_type>, f1_ts...>(
                static_cast<as_F1_t<decltype(instance_value)>>(instance_value), 
                std::forward<f1_args_t>(f1_args_v)...
            ),
            functional::invoke(
                static_cast<as_F2_t<decltype(instance_value)>>(instance_value)
            )
        ))
        {
            functional::invoke<as_F1_t<instance_type>, f1_ts...>(
                static_cast<as_F1_t<decltype(instance_value)>>(instance_value), 
                std::forward<f1_args_t>(f1_args_v)...
            );
            return functional::invoke(
                static_cast<as_F2_t<decltype(instance_value)>>(instance_value)
            );
        }

    public:
        
        then() = delete;
        then(const then&) = default;                // conditionaly delete (see storage)
        constexpr then(then&&) = default;           // conditionaly delete (see storage)
        then& operator=(const then&) = default;     // conditionaly delete (see storage)
        constexpr then& operator=(then&&) = default;

        constexpr then(auto && f1_value, auto && f2_value)
        : functor_aggregator_type(std::forward<decltype(f1_value)>(f1_value), std::forward<decltype(f2_value)>(f2_value))
        {}

        // TODO : mix TTP/NTTP : p1985
        // `F2(F1());`
        #pragma region F1_then_F2_fwd
        // `F2(F1());` : arguments fwd, lvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t> // auto ... f1_vs, 
        requires 
        requires {
            F1_then_F2_fwd<type&, f1_ts...>(std::declval<type&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) &
        noexcept(noexcept(
            F1_then_F2_fwd<type&, f1_ts...>(std::declval<type&>(), std::declval<f1_args_t&&>()...)
        ))
        {
            return F1_then_F2_fwd<type&, f1_ts...>(*this, std::forward<f1_args_t>(f1_args_v)...);
        }

        // `F2(F1());` : arguments fwd, rvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t> // auto ... f1_vs, 
        requires 
        requires {
            F1_then_F2_fwd<type&&, f1_ts...>(std::declval<type&&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) &&
        noexcept(noexcept(
            F1_then_F2_fwd<type&&, f1_ts...>(std::declval<type&&>(), std::declval<f1_args_t&&>()...)
        ))
        {
            return F1_then_F2_fwd<type&&, f1_ts...>(std::move(*this), std::forward<f1_args_t>(f1_args_v)...);
        }

        // `F2(F1());` : arguments fwd, const lvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t> // auto ... f1_vs, 
        requires 
        requires {
            F1_then_F2_fwd<const type&, f1_ts...>(std::declval<const  type&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const &
        noexcept(noexcept(
            F1_then_F2_fwd<const type&, f1_ts...>(std::declval<const type&>(), std::declval<f1_args_t&&>()...)
        ))
        {
            return F1_then_F2_fwd<const type&, f1_ts...>(*this, std::forward<f1_args_t>(f1_args_v)...);
        }

        // `F2(F1());` : arguments fwd, const rvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t> // auto ... f1_vs, 
        requires 
        requires {
            F1_then_F2_fwd<const type&&, f1_ts...>(std::declval<const type&&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const &&
        noexcept(noexcept(
            F1_then_F2_fwd<const type&&, f1_ts...>(std::declval<const type&&>(), std::declval<f1_args_t&&>()...)
        ))
        {
            return F1_then_F2_fwd<const type&&, f1_ts...>(std::move(*this), std::forward<f1_args_t>(f1_args_v)...);
        }
        #pragma endregion

        // TODO : mix TTP/NTTP : p1985
        // TODO : warning on F1() return value discard
        // `F1(); F2();`
        #pragma region F1_then_F2_no_fwd
        // `F1(); F2();` : no argument fwd, lvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t>
        requires
        requires {
            F1_then_F2_no_fwd<type&, f1_ts...>(std::declval<type&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) &
        noexcept(noexcept(
            F1_then_F2_no_fwd<type&, f1_ts...>(*this, std::forward<f1_args_t>(f1_args_v)...)
        ))
        {
            return F1_then_F2_no_fwd<type&, f1_ts...>(*this, std::forward<f1_args_t>(f1_args_v)...);
        }

        // `F1(); F2();` : no argument fwd, const lvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t>
        requires
        requires {
            F1_then_F2_no_fwd<const type&, f1_ts...>(std::declval<const type&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const &
        noexcept(noexcept(
            F1_then_F2_no_fwd<const type&, f1_ts...>(*this, std::forward<f1_args_t>(f1_args_v)...)
        ))
        {
            return F1_then_F2_no_fwd<const type&, f1_ts...>(*this, std::forward<f1_args_t>(f1_args_v)...);
        }

        // `F1(); F2();` : no argument fwd, rvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t>
        requires
        requires {
            F1_then_F2_no_fwd<type&&, f1_ts...>(std::declval<type&&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) &&
        noexcept(noexcept(
            F1_then_F2_no_fwd<type&&, f1_ts...>(std::move(*this), std::forward<f1_args_t>(f1_args_v)...)
        ))
        {
            return F1_then_F2_no_fwd<type&&, f1_ts...>(std::move(*this), std::forward<f1_args_t>(f1_args_v)...);
        }

        // `F1(); F2();` : no argument fwd, const rvalue-reference
        template <typename ... f1_ts, typename ... f1_args_t>
        requires
        requires {
            F1_then_F2_no_fwd<const type&&, f1_ts...>(std::declval<const type&&>(), std::declval<f1_args_t&&>()...);
        }
        constexpr decltype(auto) operator()(f1_args_t && ... f1_args_v) const &&
        noexcept(noexcept(
            F1_then_F2_no_fwd<const type&&, f1_ts...>(std::move(*this), std::forward<f1_args_t>(f1_args_v)...)
        ))
        {
            return F1_then_F2_no_fwd<const type&&, f1_ts...>(std::move(*this), std::forward<f1_args_t>(f1_args_v)...);
        }
        #pragma endregion
    };
    template <typename F1, typename F2>
    then(F1, F2) -> then<std::remove_cvref_t<F1>, std::remove_cvref_t<F2>>;

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
        constexpr decltype(auto) operator()(f_args_t&&... f_args) const
        noexcept(noexcept(functional::invoke<F>(std::declval<F>(), std::declval<f_args_t&&>()...)))
        {

            static_assert(not std::is_const_v<decltype(_f)>);
            static_assert(not std::is_reference_v<decltype(_f)>);

            using f_invoke_result_t = std::remove_cvref_t<
                decltype(functional::invoke<F, f_ts...>(std::declval<F>(), std::declval<f_args_t>()...))
            >;

            if constexpr (std::is_same_v<void, f_invoke_result_t>)
            {
                //for (auto count : std::ranges::iota_view<std::size_t>{0, call_count}) // Clang 12.0.0 issue
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

    // todo : check for cx_repeater => need dedicated strong-type wrapper ?
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

namespace test::functional::invoke_apply {
    consteval void invocable() {

        namespace mp = workflow::functional::mp;

        auto ttps_func = []<typename ...>(){};
        ttps_func();
        ttps_func.template operator()<int, char>();

        static_assert(std::invocable<decltype(ttps_func)>);
        static_assert(mp::invocable<decltype(ttps_func), mp::ttps_pack<>>);
        static_assert(mp::invocable<decltype(ttps_func), mp::ttps_pack<int, char>>);

        static_assert(not std::is_nothrow_invocable_v<decltype(ttps_func)>);
        static_assert(not mp::nothrow_invocable<decltype(ttps_func), mp::ttps_pack<>>);
        static_assert(not mp::nothrow_invocable<decltype(ttps_func), mp::ttps_pack<int, char>>);
    }
    consteval void nothrow_invocable() {

        using namespace workflow::functional;

        auto func = []<typename T>(auto) noexcept {};

        static_assert(requires{ func.template operator()<int>('a'); });
        static_assert(mp::nothrow_invocable<decltype(func), mp::ttps_pack<int>, char>);
        static_assert(requires{  invoke<int>(func, 42);});
        static_assert(requires{  invoke([](){});});
        static_assert(requires{  apply<int>(func, std::tuple{'a'});});

        static_assert(noexcept(invoke<int>(func, 42)));
        static_assert(noexcept(apply<int>(func, std::tuple{'a'})));
    }
    consteval void apply() {

        namespace f = workflow::functional;

        auto func = []<typename>(int, std::string&&){};

        static_assert(requires{
            f::apply<int>(func, std::tuple{42, std::string{""}});
        });
        static_assert(requires{
            f::apply_after<int>(func, std::tuple{std::string{""}}, 42);
        });
        static_assert(requires{
            f::apply_before<int>(func, std::tuple{42}, std::string{""});
        });
    }
}
namespace test::details::cvref_tags {
    struct rvalue{};
    struct lvalue{};
    struct const_qualifier{};
    struct volatile_qualifier{};

    template <typename ... Ts>
    struct tags {};
}
namespace test::functional::bind {
    
    using namespace test::details;

    struct user_defined_functor {
        auto operator()() &&        -> tags<cvref_tags::rvalue> { return {}; }
        auto operator()() const &&  -> tags<cvref_tags::const_qualifier, cvref_tags::rvalue> { return {}; }
        auto operator()() &         -> tags<cvref_tags::lvalue> { return {}; }
        auto operator()() const &   -> tags<cvref_tags::const_qualifier, cvref_tags::lvalue> { return {}; }
    };
    struct user_defined_template_functor {
        template <typename, typename>
        auto operator()() &&        -> tags<cvref_tags::rvalue> { return {}; }
        template <typename, typename>
        auto operator()() const &&  -> tags<cvref_tags::const_qualifier, cvref_tags::rvalue> { return {}; }
        template <typename, typename>
        auto operator()() &         -> tags<cvref_tags::lvalue> { return {}; }
        template <typename, typename>
        auto operator()() const &   -> tags<cvref_tags::const_qualifier, cvref_tags::lvalue> { return {}; }
    };

    consteval void front_binder_t() {
        using namespace workflow::functional;

        auto lambda_value = [](int, const char &){};
        static_assert(requires{
            front_binder{std::move(lambda_value), std::declval<int>()}(std::declval<const char&>());    
        });
        static_assert(requires{
            front_binder{lambda_value, std::declval<int>()}(std::declval<const char&>());    
        });

        auto template_lambda_value = [storage = 0]<typename, typename>(int, const char &) mutable { ++storage; };
        static_assert(requires{
            front_binder{
                template_lambda_value,
                ttps_pack<short>{},
                std::declval<int>()
            }.template operator()<bool>(std::declval<const char&>());    
        });
    }
    consteval void ud_functor() {
        using namespace workflow::functional;

        static_assert(std::is_invocable_r_v<
            tags<cvref_tags::rvalue>,
            user_defined_functor
        >);
        static_assert(std::is_invocable_r_v<
            tags<cvref_tags::lvalue>,
            user_defined_functor&
        >);
        static_assert(std::is_invocable_r_v<
            tags<cvref_tags::rvalue>,
            user_defined_functor&&
        >);
        static_assert(std::is_invocable_r_v<
            tags<cvref_tags::const_qualifier, cvref_tags::lvalue>,
            const user_defined_functor&
        >);
        static_assert(std::is_invocable_r_v<
            tags<cvref_tags::const_qualifier, cvref_tags::rvalue>,
            const user_defined_functor&&
        >);
    }
    consteval void std_bind_front_ud_functor() {
        using namespace workflow::functional;

        using type = std::remove_cvref_t<decltype(std::bind_front(std::declval<user_defined_functor>()))>;

        static_assert(std::is_invocable_r_v<tags<cvref_tags::lvalue>, type &>);
        static_assert(std::is_invocable_r_v<tags<cvref_tags::rvalue>, type &&>);
        static_assert(std::is_invocable_r_v<tags<cvref_tags::const_qualifier, cvref_tags::lvalue>, const type &>);
        static_assert(std::is_invocable_r_v<tags<cvref_tags::const_qualifier, cvref_tags::rvalue>, const type &&>);
    }
    consteval void bind_front_ud_functor() {
        using namespace workflow::functional;

        using type = std::remove_cvref_t<decltype(bind_front(std::declval<user_defined_functor>()))>;

        static_assert(std::is_invocable_r_v<tags<cvref_tags::lvalue>, type &>);
        static_assert(std::is_invocable_r_v<tags<cvref_tags::rvalue>, type &&>);
        static_assert(std::is_invocable_r_v<tags<cvref_tags::const_qualifier, cvref_tags::lvalue>, const type &>);
        static_assert(std::is_invocable_r_v<tags<cvref_tags::const_qualifier, cvref_tags::rvalue>, const type &&>);
    }
    consteval void bind_front_tud_functor() {
        using namespace workflow::functional;

        using type = std::remove_cvref_t<decltype(
            bind_front<char>(std::declval<user_defined_template_functor>())
        )>;

        static_assert(std::is_same_v<
            decltype(std::declval<type&&>().template operator()<int>()),
            tags<cvref_tags::rvalue>
        >);
        static_assert(std::is_same_v<
            decltype(std::declval<type&>().template operator()<int>()),
            tags<cvref_tags::lvalue>
        >);
        static_assert(std::is_same_v<
            decltype(std::declval<const type&&>().template operator()<int>()),
            tags<cvref_tags::const_qualifier, cvref_tags::rvalue>
        >);
        static_assert(std::is_same_v<
            decltype(std::declval<const type&>().template operator()<int>()),
            tags<cvref_tags::const_qualifier, cvref_tags::lvalue>
        >);
    }
    consteval void partially_resolved_ttps() {
        using namespace workflow::functional;
        auto func = []<typename, typename, typename arg_t>(arg_t){};
        static_assert(requires {
            bind_front<int>(func).operator()<char>(42);
        });
        static_assert(requires {
            bind_front<int, char>(func)(42);
        });
    }
}

#include <memory> // `std::unique_ptr` as move-only type

namespace test {

    // workflow::then
    constexpr void F1_then_F2() {
        {   // F1(), F2()
            {   // pr_pr_value
                auto pr_pr_value = workflow::then {
                    [](){},
                    [](){}
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // l_pr_value
                const auto lvalue = [](){};
                auto l_pr_value = workflow::then {
                    lvalue,
                    [](){}
                };
                static_assert(std::is_invocable_v<decltype(l_pr_value)>);
            }
            {   // wiring mismatch
                auto pr_pr_value = workflow::then {
                    [](){ return 42; },
                    [](){}
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // mutable F1
                auto pr_pr_value = workflow::then {
                    [i = 0]() mutable { return ++i; },
                    [](){}
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // mutable F1, const route
                const auto pr_pr_value = workflow::then {
                    [i = 0]() mutable { return ++i; },
                    [](){}
                };
                static_assert(not std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // mutable F1, mutable F2
                auto pr_pr_value = workflow::then {
                    [i = 0]() mutable { return ++i; },
                    []() mutable {}
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // TTP const lhs
                const auto lhs = []<typename T>(){ return T{}; };
                const auto & lhs_ref = lhs;
                auto value = workflow::then {
                    lhs,
                    [](){}
                };
                value.template operator()<int>();

                using lhs_type = std::remove_reference_t<decltype(lhs_ref)>;
                std::invoke(&lhs_type::template operator()<int>, lhs);
                workflow::functional::invoke(&lhs_type::template operator()<int>, lhs);
            }
        }
        {   // F2(F1())
            { // pr_pr_value
                auto pr_pr_value = workflow::then {
                    [](){ return 42; },
                    [](int i){}
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // l_pr_value
                const auto lvalue = [](){ return 42;};
                auto l_pr_value = workflow::then {
                    lvalue,
                    [](int i){}
                };
                static_assert(std::is_invocable_v<decltype(l_pr_value)>);
            }
            {   // implicit cast
                auto pr_pr_value = workflow::then {
                    [](){ return 42; },
                    [](char){}
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // mutable F1, mutable route
                // lvalue-reference *this :
                auto pr_pr_value = workflow::then {
                    [i = 0]() mutable { return ++i; },
                    [](int i){ std::cout << i << '\n'; }
                };
                static_assert(std::is_invocable_v<decltype(pr_pr_value)>);

                // rvalue-reference *this :
                static_assert(std::is_invocable_v<decltype(
                        workflow::then {
                        [i = 0]() mutable { return ++i; },
                        [](int i){ std::cout << i << '\n'; }
                    }
                )>);
            }
            {   // mutable F1, const route
                const auto pr_pr_value = workflow::then {
                    [i = 0]() mutable { return ++i; },
                    [](int i){ std::cout << i << '\n'; }
                };
                static_assert(not std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // mutable F1, const F2, const route
                const auto rhs = [](int i) constexpr { std::cout << i << '\n'; };
                const auto pr_pr_value = workflow::then {
                    [i = 0]() mutable { return ++i; },
                    rhs
                };
                static_assert(not std::is_invocable_v<decltype(pr_pr_value)>);
            }
            {   // F1 -> overload
                const auto value = workflow::then {
                    [](){ return 42; },
                    workflow::functional::overload {
                        [](int){},
                        [](char){}
                    }
                };
                static_assert(std::is_invocable_v<decltype(value)>);
            }
            {   // TTP : overloaded member-function
                const auto value = workflow::then {
                    []<typename switch_type>(){
                        using return_type = std::conditional_t<switch_type::value, int, char>;
                        return return_type{};
                    },
                    [](auto && value){
                        return value;
                    }
                };
                // cannot decltype overloaded member function, std::is_invocable_r_v -> std::is_same<call, return_type>
                static_assert(std::is_same_v<
                    decltype(value.template operator()<std::true_type>()),
                    int
                >);
                static_assert(std::is_same_v<
                    decltype(value.template operator()<std::false_type>()),
                    char
                >);
            }
            {   // TTP : overload member-function -> overload
                auto route = workflow::then {
                    []<typename switch_type>(){
                        using return_type = std::conditional_t<switch_type::value, int, char>;
                        return return_type{};
                    },
                    workflow::functional::overload{
                        [](int) { return std::true_type{}; },
                        [](char){ return std::false_type{}; }
                    }
                };
                static_assert(std::is_same_v<
                    decltype(route.template operator()<std::true_type>()),
                    std::true_type
                >);
                static_assert(std::is_same_v<
                    decltype(route.template operator()<std::false_type>()),
                    std::false_type
                >);
            }
            {   // TTP : overload member-function -> overload with auto&& fallback
                auto route = workflow::then {
                    []<typename switch_type>(){
                        using return_type = std::conditional_t<switch_type::value, int, char>;
                        return return_type{};
                    },
                    workflow::functional::overload{
                        [](int) { return std::true_type{}; },
                        [](auto&&){ return std::false_type{}; }
                    }
                };
                static_assert(std::is_same_v<
                    decltype(route.template operator()<std::true_type>()),
                    std::true_type
                >);
                static_assert(std::is_same_v<
                    decltype(route.template operator()<std::false_type>()),
                    std::false_type
                >);
            }
        }
    }
    constexpr void F_then_F() {
        auto plus_one = [](int i) constexpr { return i+1; };
        auto route = workflow::then {
            plus_one,
            plus_one
        };
        static_assert(std::is_invocable_r_v<int, decltype(route), int>);
    }
    constexpr void then() {
        F1_then_F2();
        F_then_F();
    }
    
    // workflow::rt_repeater
    constexpr void rt_repeater() {
        {   // rt, not mutable
            auto value = workflow::rt_repeater{[](){}, 3};
            static_assert(std::is_invocable_v<decltype(value)>);
            static_assert(std::is_invocable_v<const decltype(value)>);
        }
        {   // rt, mutable
            auto value = workflow::rt_repeater{[]() mutable {}, 3};
            static_assert(std::is_invocable_v<decltype(value)>);
            static_assert(std::is_invocable_v<const decltype(value)>);
        }
        {   // rt, mutable, storage
            auto value = workflow::rt_repeater{[i = 0]() mutable {++i;}, 3};
            static_assert(std::is_invocable_v<decltype(value)>);
            static_assert(std::is_invocable_v<const decltype(value)>);
        }
        {   // rt, constexpr
            auto value = workflow::rt_repeater{[]() constexpr {}, 3};
            static_assert(std::is_invocable_v<decltype(value)>);
            static_assert(std::is_invocable_v<const decltype(value)>);
        }
        {   // noexcept
            auto noexcept_value = workflow::rt_repeater{[]() constexpr noexcept {}, 3};
            static_assert(std::is_nothrow_invocable_v<decltype(noexcept_value)>);
            static_assert(std::is_nothrow_invocable_v<const decltype(noexcept_value)>);

            auto except_value = workflow::rt_repeater{[]() constexpr {}, 3};
            static_assert(std::is_invocable_v<decltype(noexcept_value)>);
            static_assert(std::is_invocable_v<const decltype(noexcept_value)>);
            static_assert(not std::is_nothrow_invocable_v<decltype(except_value)>);
            static_assert(not std::is_nothrow_invocable_v<const decltype(except_value)>);
        }
    }

    // operator>>=
    constexpr void right_shift_equal_operator() {
        using namespace workflow::operators;
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
    constexpr void pipe_operator() {
        using namespace workflow::operators;
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

            using route_type = std::remove_reference_t<decltype(route)>;
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
    void star_operator_runtime() {
        using namespace workflow::operators;
        {
            auto route =
                [call_counter = 0]() mutable { return ++call_counter; } *
                3
            ;
            static_assert(std::is_invocable_r_v<std::vector<int>, decltype(route)>);
            if (route() not_eq std::vector{1,2,3})
                throw std::runtime_error{"operator*<F, std::size_t> : unexpected return values"};
        }
    }
    constexpr void star_operator() {
        using namespace workflow::operators;
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
        {   // constexpr mutable F
            using call_count_t = std::integral_constant<int, 3>;
            auto route =
                [call_counter = 0]() constexpr mutable {  return ++call_counter; } *
                call_count_t{}
            ;
            static_assert(std::is_invocable_v<decltype(route)>);
            static_assert(std::is_invocable_r_v<std::array<int, call_count_t::value>, decltype(route)>);

            if (route() not_eq std::array{1,2,3})
                throw std::runtime_error{"operator*<F, std::size_t> : unexpected return values"};
        }
        {   // constexpr route

            // todo : inherit from lambdas instead of passing it as args ?

            // using call_count_t = std::integral_constant<int, 3>;
            // constexpr auto route =
            //     []() constexpr {  return 42; } *
            //     call_count_t{}
            // ;
            // static_assert(std::is_invocable_v<decltype(route)>);
            // static_assert(std::is_invocable_r_v<std::array<int, call_count_t::value>, decltype(route)>);
            // static_assert(route() == std::array{42, 42, 42});
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
        {   // custom functor type
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
            {
                constexpr auto route = functor{} * 3;
                static_assert(std::is_invocable_v<decltype(route)>);
            }
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

    // scenario : F >>= F >>= F >>= F
    void scenario__then_F_four_times() {
        auto plus_one = [](int i) constexpr { return i+1; };
        {   // four times the same functor
            auto twice = workflow::then {
                plus_one,
                plus_one
            };
            auto four_times = workflow::then {
                twice,
                twice
            };
            if (four_times(0) not_eq 4)
                throw std::runtime_error{"workflow::then<F,F> : unexpected result / call count"};
        }
        {
            using namespace workflow::operators;
            // auto four_times = 
            //     plus_one >>=
            //     plus_one >>=
            //     plus_one >>=
            //     plus_one
            // ;
            // if (four_times(0) not_eq 4)
            //     throw std::runtime_error{"workflow::then<F,F> : unexpected result / call count"};
        }
    }
    // scenario : (F1 >>= F2) * int
    void scenario__then_into_repeat_runtime() {
        using namespace workflow::operators;
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
        {   // constexpr route
            auto lhs = (
                    [](std::string_view && value) constexpr -> std::string_view { return value; }
                |   [](auto && value) constexpr -> std::string_view
                    requires requires {
                        std::begin(value); 
                        std::end(value);
                    }
                    { return std::string_view { std::begin(value), std::end(value) };}
            );
            static_assert(std::is_invocable_r_v<std::string_view, decltype(lhs), std::string>);
            static_assert(std::is_invocable_r_v<std::string_view, decltype(lhs), std::vector<char>>);
            static_assert(std::is_invocable_r_v<std::string_view, decltype(lhs), std::string_view>);
            // static_assert(std::is_invocable_v<decltype((lhs >>= lhs)), std::string_view&&>);

            auto rhs = (
                [](auto && value) constexpr mutable noexcept
                requires (std::same_as<std::decay_t<decltype(value)>, std::string_view>)
                {
                    if (not value.empty())
                        value.remove_prefix(std::min(value.find_first_of(" ") + 1, value.size()));
                    return value;
                } * 3
            );
            static_assert(std::is_invocable_v<decltype(rhs), std::string_view&&>);
            static_assert(std::is_invocable_r_v<std::vector<std::string_view>, decltype(rhs), std::string_view&&>);
            
            constexpr auto route = (lhs >>= ([](auto && value) constexpr noexcept
                requires (std::same_as<std::decay_t<decltype(value)>, std::string_view>)
                {
                    if (not value.empty())
                        value.remove_prefix(std::min(value.find_first_of(" ") + 1, value.size()));
                    return value;
                } * 3));
            static_assert(std::is_invocable_v<decltype(route), std::string_view&&>);

            using namespace std::string_view_literals;
            if (route("a b c d"sv) not_eq std::vector<std::string_view>{
                "b c d"sv,
                "c d"sv,
                "d"sv
            })
               throw std::runtime_error{"(F1 >>= F2) * int : unexpected values"};
        }
    }
    constexpr void scenario__then_into_repeat() {
        using namespace workflow::operators;
        {   // constexpr nodes
            // TODO : remove, or dont violate constness
            // operator() of constexpr mutable lambdas result closure-type violate *this constness
        }
    }
}

auto main() -> int {

    {   // compile-time tests :

        // components
        test::then();
        test::rt_repeater();

        // operators
        test::right_shift_equal_operator();
        test::pipe_operator();
        test::star_operator();

        // scenarios
        test::scenario__then_into_repeat();
    }

    // runtime tests :
    try {
        test::scenario__then_F_four_times();
        test::star_operator_runtime();
        test::scenario__then_into_repeat_runtime();
    }
    catch (const std::exception & error) {
        std::cerr << "test failed : " << error.what() << '\n';
    }
    catch (...) {
        std::cerr << "test failed\n";
    }

    {   // invoke synthaxes ...
        auto qwe = []<typename T = char>(int){};

        using result_t   = std::invoke_result_t<decltype(&decltype(qwe):: template operator()<bool>), decltype(qwe), int>;
        using result_t_2 = std::invoke_result_t<decltype(&decltype(qwe):: template operator()<>), decltype(qwe), int>;
        using result_t_3 = std::invoke_result_t<decltype(qwe), int>;
    }
    {
        constexpr auto node_1 = [](){};
        constexpr auto node_2 = [](){};
        constexpr auto route = [
                f1 = std::move(node_1),
                f2 = std::move(node_2)
        ]() constexpr {
            f1();
            f2();
        };
        route();  
    }
}
#endif
