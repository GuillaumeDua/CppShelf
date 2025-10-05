#pragma once

// [Cpp Shelf Library] mp - metaprogramming utility
// under MIT License - Copyright (c) 2021-2025 Guillaume Dua
// see https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE

// About [tuple-like]:
//
//  A given tuple-like type T is considered valid if the following concepts evaluate to true
//  - csl::mp::concepts::tuple_like<T> evaluates to true (based on P2165).
//  - csl::mp::concepts::uniqued<T> -> tuple_element<indexes, T>... contains no duplicates
//  If a given tuple-like type T instanciation is considered not valid,
//  then LESS performant algorithms implementations may be selected to provide similar functionalities, as a best-effort.

// About [csl::mp::tuple] vs. other [tuple-like]
//
//  For a given type T, if csl::mp::concepts::tuple evaluates to true (csl::mp::is_tuple is std::true_type)
//  then MORE performant algorithms implementation may be selected.

// About [algorithms]:
//
//  All algorithms are partitioned in two groups:
//      [type-traits]: contains, count/count_if, find/find_if, etc.
//      [functions]: cat, tie, apply, for_each, etc.
//
//  Known limitation:
//      [rebind] and other algorithms relying on it only supports a restricted set of tuple-likes types ([std] and [csl::mp] ones)

// About [conversion]: using [csl::mp::tuple] as a drop-in replacement for [std::tuple]
//  As <tuple> is -isystem, implicit members conversions do not produce warnings
//  The cmake option and pp-definition `CSL_MP_TUPLE__IMPLICIT_CONVERSION` toggles this behavior on/off,
//  if three values are allowed:
//
//  - NONE (or undefined)   : no conversion allowed
//  - SAFE                  : let the compiler warn/error on narrowing conversions
//  - UNSAFE                : drop-in replacement for [std::tuple], silent all conversions
//
//  Given two different [tuple] instances T1 and T2
//  - std::common_reference_t<T1, T2>
//  - std::equality_comparable_with<T1, T2>
//  - std::three_way_comparable_with<T1, T2>
//  - Any construction of T1 from a possibly-cvref-qualified T2 value

// --- Handle preprocessor options
#define CSL_MP_TUPLE__IMPLICIT_CONVERSION_NONE      0
#define CSL_MP_TUPLE__IMPLICIT_CONVERSION_SAFE      1
#define CSL_MP_TUPLE__IMPLICIT_CONVERSION_UNSAFE    2

#if not defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION)
#   define CSL_MP_TUPLE__IMPLICIT_CONVERSION CSL_MP_TUPLE__IMPLICIT_CONVERSION_NONE
#elif CSL_MP_TUPLE__IMPLICIT_CONVERSION != CSL_MP_TUPLE__IMPLICIT_CONVERSION_NONE \
  && CSL_MP_TUPLE__IMPLICIT_CONVERSION != CSL_MP_TUPLE__IMPLICIT_CONVERSION_SAFE \
  && CSL_MP_TUPLE__IMPLICIT_CONVERSION != CSL_MP_TUPLE__IMPLICIT_CONVERSION_UNSAFE
#   error "CSL_MP_TUPLE__IMPLICIT_CONVERSION: expect either 0 (none), 1 (allow only safe ops), or 2 (on, allow unsafe)"
#endif

#define STRINGIFY_DETAIL(x) #x
#define STRINGIFY(x) STRINGIFY_DETAIL(x)
#   pragma message("CSL_MP_TUPLE__IMPLICIT_CONVERSION = " STRINGIFY(CSL_MP_TUPLE__IMPLICIT_CONVERSION))
#undef STRINGIFY
#undef STRINGIFY_DETAIL

// ---

#if not __cplusplus >= 202002L
# error "csl/mp.hpp requires C++20 or greater"
#endif

#include <compare>
#include <type_traits>
#include <concepts>
#include <algorithm>
#include <functional>

#define csl_fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__) // NOLINT(*-macro-*)

// Drop-in replacement for std::tuple
//  As <tuple> is -isystem, implicit members conversions do not produce warnings
//  The cmake option and pp-definition `CSL_MP_TUPLE__IMPLICIT_CONVERSION=0/1` toggles this behavior off/on
//
// NOTE: Implement such a macro as a function would return a ref on a temporary
// template <typename T>
// [[nodiscard]] constexpr static auto fwd_maybe_cast(std::convertible_to<T> auto && value) noexcept -> decltype(auto) {
//     #if CSL_MP_TUPLE__IMPLICIT_CONVERSION
//         return static_cast<T&&>(value);
//     // QUESTION: if SAFE, then safe_cast<T>(value) to prevent narrowing conversions ?
//     //   But costly: check value against numeric_limits min/max, etc. -> out of scope
//     #else
//         return std::forward<decltype(value)>(value);
//     #endif
// }
#if CSL_MP_TUPLE__IMPLICIT_CONVERSION
#  define csl_fwd_maybe_cast(T, value) static_cast<T&&>(value) // NOLINT(*-macro-*)
#else
#  define csl_fwd_maybe_cast(T, value) static_cast<decltype(value)&&>(value)  // NOLINT(*-macro-*)
#endif

#if defined(__clang__)
#   define csl_compiler_is_clang
#elif defined(__GNUC__) || defined(__GNUG__)
#   define csl_compiler_is_gcc
#elif defined(_MSC_VER)
#   define csl_compiler_is_msvc
#else
#   define csl_compiler_is_unknown
#endif

// deprecated by P2593R0 - Allowing static_assert(false)
namespace csl::mp::inline deprecated_by_P2593R0 {
    template <typename ...>
    struct [[deprecated("Prefer P2593R0 - Allowing static_assert(false)")]] dependent_false : std::false_type{};
    template <typename ... Ts> // NOTE: for NTTP, use decltype(value)
    constexpr static auto dependent_false_v = dependent_false<Ts...>::value;
}
// P0887 - The identity metafunction
namespace csl::mp::inline P0887 {
#if defined(__cpp_lib_type_identity)
    template <typename T>
    using type_identity = typename std::type_identity<T>;
#else
    template <typename T>
    struct type_identity{ using type = T; };
#endif
    template <typename T>
    using type_identity_t = typename type_identity<T>::type;
}

namespace csl::mp::concepts::inline fake_p2481_alternative {
    // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2481r2.html
    //  rather than T auto && or auto &&&, this will do the job for now
    template <typename T, typename U>
    concept fwd_ref =
            std::is_reference_v<T>
        and std::same_as<U, std::remove_cvref_t<T>>
    ;
}
namespace csl::mp::inline P0318 {
#if defined(__cpp_lib_unwrap_ref)
    
    template <class T>
    using unwrap_reference = std::unwrap_reference<T>;
    template <class T>
    using unwrap_reference_t = std::unwrap_reference_t<T>;

    template <class T>
    using unwrap_ref_decay = std::unwrap_ref_decay<T>;
    template <class T>
    using unwrap_ref_decay_t = std::unwrap_ref_decay_t<T>;

#else

    template <class T>
    struct unwrap_reference { using type = T; };
    template <class T>
    struct unwrap_reference<std::reference_wrapper<T>> { using type = T&; };
    template <class T>
    using unwrap_reference_t = unwrap_reference<T>::type;

    template<class T>
    struct unwrap_ref_decay : unwrap_reference<std::decay_t<T>> {};
    template <class T>
    using unwrap_ref_decay_t = typename unwrap_ref_decay<T>::type;

#endif
}

#include <array>
namespace csl::mp::concepts {
    // Limitation: universal template parameters (nttps, etc.) -> std::array, std::integer_sequence, etc.
    template <typename T, template <typename...> typename ttp>
    concept instance = requires {
        []<typename ... Ts>(type_identity<ttp<Ts...>>){
        }(type_identity<std::remove_cvref_t<T>>{});
    };

    template <typename T>
    concept std_array = requires {
        []<typename value_type, std::size_t N>(type_identity<std::array<value_type, N>>){
        }(type_identity<std::remove_cvref_t<T>>{});
    };
}
namespace csl::mp {
    // value_type<sequence_type>
    template <typename T>
    struct value_type : type_identity<typename T::value_type>{};
    template <typename T, std::size_t N>
    struct value_type<T[N]>: type_identity<T>{}; // NOLINT(*-c-arrays)
    template <typename T>
    using value_type_t = typename value_type<T>::type;

    //  example: csl::mp::bind_front<std::is_same, int>::value<int> == true
    template <template <typename ...> typename trait, typename ... Ts>
    struct bind_front {
        template <typename ... Us>
        using type = trait<Ts..., Us...>::type;
        template <typename ... Us>
        constexpr static auto value = trait<Ts..., Us...>::value;
    };
    template <template <typename ...> typename trait, typename ... Ts>
    struct bind_back {
        template <typename ... Us>
        using type = trait<Us..., Ts...>::type;
        template <typename ... Us>
        constexpr static auto value = trait<Us..., Ts...>::value;
    };
}

#include <utility>
//  csl::mp relies on STL's tuplelike API, rather than adapt csl::mp to it
//  - Need to check if worthy from performances perspective
//    consider using https://github.com/JPenuchot/ctbench, and/or https://build-bench.com
namespace csl::mp {
    // NOTE: std::remove_reference should be enough here, as the standard already removes const/volatile qualifiers

    template </*tuple-like*/ typename T> struct size : std::tuple_size<std::remove_cvref_t<T>>{};
    template </*tuple-like*/ typename T> constexpr auto size_v = size<T>::value;

    template <std::size_t I, /*tuple-like*/ typename T> struct element : std::tuple_element<I, std::remove_cvref_t<T>>{};
    template <std::size_t I, /*tuple-like*/ typename T> using element_t = typename element<I, T>::type;

    template <std::size_t I, /*tuple-like*/ typename T> struct member_value : type_identity<decltype(
        get<I>(std::declval<T>())
    )>{};
    template <std::size_t I, /*tuple-like*/ typename T> using member_value_t = typename member_value<I, T>::type;
}

// --- sequence ---
namespace csl::mp {

    // size
    template <typename T, T ... values>
    struct size<std::integer_sequence<T, values...>>
    : std::integral_constant<std::size_t, sizeof...(values)> // ::size()
    {};

    // element
    template <std::size_t I, typename T, T ... values>
    struct element<I, std::integer_sequence<T, values...>>
    : std::type_identity<T>
    {};

    // member_value
    template <std::size_t I, typename T, T ... values>
    struct member_value<I, std::integer_sequence<T, values...>>
    : std::type_identity<T>
    {};

    // is_sequence
    template <typename T>
    struct is_sequence : std::false_type{};
    template <typename T, T ... values>
    struct is_sequence<std::integer_sequence<T, values...>> : std::true_type{};
    template <typename T>
    constexpr static inline auto is_sequence_v = is_sequence<T>::value;

    // to_tuplelike
    template <
        typename T
    // TODO(Guillaume) universal TTP: to = std::array | std::tuple | std::pair etc.
    >
    struct to_tuplelike;
    template <typename T, T ... values>
    struct to_tuplelike<std::integer_sequence<T, values...>>{
        using type = std::array<T, sizeof...(values)>;
        constexpr static auto value = type{ values... };
    };
    template <typename T>
    using to_tuplelike_t = to_tuplelike<T>::type;
    template <typename T>
    constexpr static inline auto to_tuplelike_v = to_tuplelike<T>::value;

    // at<index>
    template <std::size_t, typename>
    struct at;
    template <std::size_t index, typename T, T ... values>
    struct at<index, std::integer_sequence<T, values...>> : std::integral_constant<
        T,
        std::get<index>(
            to_tuplelike_v<std::integer_sequence<T, values...>>
        )
    >{};
    template <std::size_t index, typename T>
    constexpr static inline auto at_v = at<index, T>::value;

    namespace seq::concepts {
        template <typename T>
        concept sequence = is_sequence_v<std::remove_cvref_t<T>>;

        template <typename T>
        concept empty = sequence<T> and std::remove_cvref_t<T>::size() == 0;
        template <typename T>
        concept not_empty = sequence<T> and std::remove_cvref_t<T>::size() not_eq 0;
        template <typename T, std::size_t N>
        concept sized = sequence<T> and std::remove_cvref_t<T>::size() == N;
        template <typename T, std::size_t N>
        concept sized_at_least = sequence<T> and std::remove_cvref_t<T>::size() >= N;
    }

    // reverse
    template <typename T>
    struct reverse;
    // Assuming 0..N : std::integer_sequence<T, (sizeof...(values) - 1 - values)...>
    template <seq::concepts::sequence T>
    struct reverse<T> : type_identity<
        decltype(
            []<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return std::integer_sequence<
                    typename T::value_type,
                    at_v<T::size() - 1 - indexes, T>...
                >{};
            }(std::make_index_sequence<T::size()>{})
        )
    >{};

    template <typename T>
    using reverse_t = reverse<T>::type;
    template <std::size_t I>
    using make_reverse_index_sequence = reverse_t<std::make_index_sequence<I>>;

    // get<index>(seq)
    template <std::size_t index, typename T, T ... values>
    constexpr decltype(auto) get(std::integer_sequence<T, values...>) noexcept {
        return at_v<index, std::integer_sequence<T, values...>>;
    }
}

// P2165 - tuple-like
//  https://en.cppreference.com/w/cpp/utility/tuple/tuple-like.html
namespace csl::mp::concepts::P2165 {
	template <typename T, std::size_t N>
    concept tuple_element =
            requires { std::tuple_size<T>{}; }
        and std::tuple_size_v<T> > N
        and requires(T t) {
            typename std::tuple_element_t<N, std::remove_const_t<T>>;
            { get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T>&>;
        }
    ;
    namespace details {
        // QUICK-FIX: Clang >= 18.1.8 Same mangled name error
        template <typename T>
        constexpr static auto valid_tuple_elements_v = []<std::size_t... I>(std::index_sequence<I...>) constexpr {
            return (true and ... and tuple_element<T, I>);
        }(std::make_index_sequence<std::tuple_size_v<T>>{});
    }
    template <typename T>
    concept tuple_like =
        not std::is_reference_v<T>
        and requires {
            typename std::tuple_size<T>::type;
            requires std::same_as<std::remove_const_t<decltype(std::tuple_size_v<T>)>, std::size_t>;
        }
        // QUICK-FIX: Clang >= 18.1.8 Same mangled name error
        and details::valid_tuple_elements_v<T>
        // and []<std::size_t... I>(std::index_sequence<I...>) constexpr {
        //     return (tuple_element<T, I> && ...);
        // }(std::make_index_sequence<std::size_v<T>>{})
    ;
    template <typename T>
    concept pair_like = tuple_like<T> and std::tuple_size_v<T> == 2;
}

namespace csl::mp::concepts {
	template <typename T, std::size_t N>
    concept tuple_element = P2165::tuple_element<std::remove_cvref_t<T>, N>;
    // NOTE: as details::valid_tuple_elements is costly, could be refactored with a less restrictive check
    //  empty or not_empty -> tuple_size and tuple_element<0, T> not the other elements
    template <typename T>
    concept tuple_like = P2165::tuple_like<std::remove_cvref_t<T>>;
    template <typename T>
    concept pair_like = P2165::pair_like<std::remove_cvref_t<T>>;

    template <typename T>
    concept empty = tuple_like<T> and csl::mp::size_v<T> == 0;
    template <typename T>
    concept not_empty = tuple_like<T> and not empty<T>;
    template <typename T, std::size_t N>
    concept sized = tuple_like<T> and csl::mp::size_v<T> == N;
    template <typename T, std::size_t N>
    concept sized_at_least = tuple_like<T> and csl::mp::size_v<T> >= N;
}

// P1450 - Enriching type modification traits https://github.com/cplusplus/papers/issues/216
namespace csl::mp::inline P1450 {
    // P1450 copy_ref
    template <typename from, typename to>
    struct copy_ref : std::remove_reference<to>{};
    template <typename from, typename to>
    struct copy_ref<from&, to> : std::add_lvalue_reference<to>{};
    template <typename from, typename to>
    struct copy_ref<from&&, to> : std::add_rvalue_reference<std::remove_reference_t<to>>{};
    template <typename from, typename to>
    using copy_ref_t = typename copy_ref<from, to>::type;

    // P1450 - add cv - impl detail (also for ref-qualified types)
    template <typename T> struct add_const : type_identity<const T>{};
    template <typename T> struct add_const<T&> : type_identity<const T&>{};
    template <typename T> struct add_const<T&&> : type_identity<const T&&>{};
    template <typename T> using add_const_t = typename add_const<T>::type;

    template <typename T> struct add_volatile : type_identity<volatile T>{};
    template <typename T> struct add_volatile<T&> : type_identity<volatile T&>{};
    template <typename T> struct add_volatile<T&&> : type_identity<volatile T&&>{};
    template <typename T> using add_volatile_t  = typename add_volatile<T>::type;

    template <typename T> struct add_cv : add_const<typename add_volatile<T>::type>{};
    template <typename T> using add_cv_t = typename add_cv<T>::type;

    // P1450 copy_cv
    template <typename from, typename to>
    struct copy_cv : std::remove_cv<to>{};
    template <typename from, typename to> requires (std::is_reference_v<from>)
    struct copy_cv<from, to> : copy_cv<std::remove_reference_t<from>, to>{};
    template <typename from, typename to>
    struct copy_cv<const volatile from, to> : add_cv<to>{};
    template <typename from, typename to>
    struct copy_cv<const from, to> : add_const<to>{};
    template <typename from, typename to>
    struct copy_cv<volatile from, to> : add_volatile<to>{};
    template <typename from, typename to>
    using copy_cv_t = typename copy_cv<from, to>::type;

    // P1450 copy_cvref
    template <typename from, typename to>
    struct copy_cvref : copy_cv<from, copy_ref_t<from, to>>{};
    template <typename from, typename to>
    using copy_cvref_t = typename copy_cvref<from, to>::type;
}
namespace csl::mp::inline P2445 {

    // #if defined(__cpp_lib_forward_like) && __cpp_lib_forward_like >= 202207L
    // using std::forward_like;
    // #else
    template<class T, class U>
    constexpr auto && forward_like(U&& x) noexcept
    {
        constexpr bool is_adding_const = std::is_const_v<std::remove_reference_t<T>>;
        if constexpr (std::is_lvalue_reference_v<T&&>)
        {
            if constexpr (is_adding_const)
                return std::as_const(x);
            else
                return static_cast<U&>(x);
        }
        else
        {
            if constexpr (is_adding_const)
                return std::move(std::as_const(x));
            else
                return std::move(x);
        }
    }
    // #endif
}

namespace csl::mp::inline indexing {

    template <std::size_t N>
    struct index_t: std::integral_constant<std::size_t, N>{};
    template <std::size_t N>
    constexpr static index_t<N> const index = {};

    inline namespace literals {

        namespace details {
            template <std::size_t base, char... c> constexpr auto char_sequence_to_base()
            {
                auto value = 0ULL;
                ((value = value * base + (c - '0')), ...);
                return value;
            }
        }

        template <char... c> constexpr auto operator""_index() noexcept {
            return index<details::char_sequence_to_base<10, c...>()>; // NOLINT(*-magic-numbers)
        }
    }
}

// see https://en.cppreference.com/w/cpp/standard_library/synth-three-way
namespace csl::mp::details::inline compare {

    constexpr auto synth_three_way = []<class T, class U>(const T& t, const U& u)
    requires requires
        {
            { t < u } -> std::convertible_to<bool>;
            { u < t } -> std::convertible_to<bool>;
        }
    {
        if constexpr (std::three_way_comparable_with<T, U>)
            return t <=> u;
        else
        {
            if (t < u)
                return std::weak_ordering::less;
            if (u < t)
                return std::weak_ordering::greater;
            return std::weak_ordering::equivalent;
        }
    };
    template <class T, class U = T>
    using synth_three_way_result = decltype(synth_three_way(std::declval<T&>(), std::declval<U&>()));
}

// tuple_storage
namespace csl::mp::details {

    // Member storage: associates an index and/or a type with a value
    // - mp::index_t<I>        : lookup by index
    // - mp::type_identity<T>  : lookup by type
    template <std::size_t I, typename T>
    struct tuple_member {

        constexpr static auto index = I;
        using type = T;

        T value;

        // TODO(Guillaume): Benchmark if worthy vs. plain static_cast
        // index-to-type mapping
        constexpr static tuple_member<I, T> deduce_type(mp::index_t<I>) noexcept;
        // type-to-index mapping (repetitions: clashes are handled downstream)
        constexpr static tuple_member<I, T> deduce_index(mp::type_identity<T>) noexcept;
    };
    template <std::size_t I, typename T>
    constexpr static auto tuple_member_index = tuple_member<I, T>::index;
    template <std::size_t I, typename T>
    using tuple_member_type = typename tuple_member<I, T>::type;

    template <typename T>
    struct is_tuple_member : std::false_type{};
    template <std::size_t I, typename T>
    struct is_tuple_member<tuple_member<I, T>> : std::true_type{};
    template <typename T>
    constexpr static auto is_tuple_member_v = is_tuple_member<T>::value;

    namespace concepts {
        template <typename T>
        concept tuple_member = is_tuple_member_v<std::remove_cvref_t<T>>;
    }

    // Limitation: csl::mp::concepts::instance nttps
    //
    // [[nodiscard]] constexpr static
    // auto tuple_member_value(csl::mp::concepts::instance<tuple_member> auto && te) noexcept -> decltype(auto) {
    //     using result_type = copy_cvref_t<
    //         decltype(te),
    //         typename std::remove_cvref_t<decltype(te)>::type
    //     >;
    //     return static_cast<result_type>(te.value);
    // }
    template <std::size_t I, typename T>
    [[nodiscard]] constexpr static
    T & tuple_member_value(tuple_member<I, T> & te) noexcept { return te.value; }
    template <std::size_t I, typename T>
    [[nodiscard]] constexpr static
    const T & tuple_member_value(const tuple_member<I, T> & te) noexcept { return te.value; }
    template <std::size_t I, typename T>
    [[nodiscard]] constexpr static
    T && tuple_member_value(tuple_member<I, T> && te) noexcept { return static_cast<T&&>(te.value); } // NOLINT(*-not-moved)
    template <std::size_t I, typename T>
    [[nodiscard]] constexpr static
    const T && tuple_member_value(const tuple_member<I, T> && te) noexcept { return static_cast<const T&&>(te.value); }

    template <typename ...>
    struct tuple_storage;
    template <>
    struct tuple_storage<>{};
    template <std::size_t ... indexes, typename ... Ts>
    struct tuple_storage<tuple_member<indexes, Ts>...>
    : tuple_member<indexes, Ts>... 
    {
    // tuple-element indexing
        using tuple_member<indexes, Ts>::deduce_type...;
        using tuple_member<indexes, Ts>::deduce_index...;

        template <std::size_t I>
        using by_index_ = decltype(deduce_type(index_t<I>{}));
        template <typename T>
        using by_type_ = decltype(deduce_index(type_identity<T>{}));

    // Conversion support: unsafe use are handled by the compiler -Wconversion
    #if not CSL_MP_TUPLE__IMPLICIT_CONVERSION
        constexpr explicit tuple_storage(std::convertible_to<Ts> auto && ... args) // NOLINT(*-missing-std-forward)
        noexcept((true and ... and std::is_nothrow_constructible_v<Ts, decltype(args)>))
        requires (sizeof...(Ts) == sizeof...(args)
            and (true and ... and std::constructible_from<Ts, decltype(args)>)
        )
        : tuple_member<indexes, Ts>{
            csl_fwd(args)
        }...
        {}
    #else
        template <typename ...>
        friend struct tuple_storage;
    // #endif

        template <typename ... Us>
        constexpr explicit tuple_storage(Us && ... args) // NOLINT(*-missing-std-forward)
        noexcept((true and ... and std::is_nothrow_constructible_v<Ts, Us&&>))
        requires (sizeof...(Ts) == sizeof...(Us)
            and (true and ... and std::constructible_from<Ts, Us&&>)
        )
        : tuple_member<indexes, Ts>{

            // Drop-in replacement for std::tuple
            //  As <tuple> is -isystem, implicit members conversions do not produce warnings
            //  The cmake option and pp-definition `CSL_MP_TUPLE__IMPLICIT_CONVERSION=0|1` toggles this behavior off/on
            //
            // csl_fwd_maybe_cast(Ts, csl_fwd(args))
            static_cast<copy_cvref_t<decltype(args), Ts>>(args)
        }...
        {}

    # if defined(csl_compiler_is_gcc) // up to at least gcc-13.3.0
        // QUICK-FIX: for https://gcc.gnu.org/bugzilla/show_bug.cgi?id=120500
        //  MVE: https://godbolt.org/z/8oEW71xv8
        template <std::size_t ... indexes_, typename ... Us>
        constexpr explicit
        tuple_storage(tuple_storage<tuple_member<indexes_, Us>...> && other)
    # else
        template <typename ... Us>
        constexpr explicit
        tuple_storage(tuple_storage<tuple_member<indexes, Us>...> && other)
    # endif
        noexcept((true and ... and std::is_nothrow_constructible_v<Ts, Us &&>))
        requires (
            sizeof...(Ts) == sizeof...(Us)
        and (true and ... and std::constructible_from<Ts, Us &&>)
        )
        : tuple_storage{
            static_cast<tuple_member<indexes, Us>&&>(std::move(other)).value...
        }
        {}
        template <typename ... Us>
        constexpr explicit 
        tuple_storage(const tuple_storage<tuple_member<indexes, Us>...> & other)
        noexcept((true and ... and std::is_nothrow_constructible_v<Ts, const Us &>))
        requires (
            sizeof...(Ts) == sizeof...(Us)
        and (true and ... and std::constructible_from<Ts, const Us &>)
        )
        : tuple_storage{
            static_cast<const tuple_member<indexes, Us>&>(other).value...
        }
        {}
    #endif

        constexpr tuple_storage() = default;
        constexpr ~tuple_storage() = default;
        constexpr tuple_storage(tuple_storage&&) noexcept = default;
        constexpr tuple_storage(const tuple_storage &) = default;
        constexpr tuple_storage & operator=(tuple_storage &&) noexcept = default;
        constexpr tuple_storage & operator=(const tuple_storage &) = default;
    };

    template <typename sequence_type, typename ... Ts>
    struct make_tuple_storage;
    template <std::size_t ... indexes, typename ... Ts>
    struct make_tuple_storage<std::index_sequence<indexes...>, Ts...> : type_identity<
        typename mp::details::tuple_storage<
            mp::details::tuple_member<indexes, Ts>...
        >
    >{};
    template <
        csl::mp::seq::concepts::sequence sequence_type,
        typename ... Ts
    >
    using make_tuple_storage_t = typename make_tuple_storage<sequence_type, Ts...>::type;
}
// tuple: API traits
namespace csl::mp {

    template <typename ... Ts>
    struct tuple;

    // is_tuple
    template <typename>
    struct is_tuple : std::false_type{};
    template <typename ... Ts>
    struct is_tuple<tuple<Ts...>> : std::true_type{};
    template <typename T>
    constexpr static bool is_tuple_v = is_tuple<T>::value;

    namespace concepts {
        template <typename T> concept tuple = is_tuple_v<std::remove_cvref_t<T>>;
    }

    // REFACTO:
    //  template <typename T>
    //  struct size : csl::mp::size<std::remove_cvref_t<T>>{};
    //
    //  same for element -> std::tuple_element
    //
    // as it's already done for STL interops afterward

    // tuple_size
    // template <typename>
    // struct tuple_size;
    // template <typename ... Ts>
    // struct tuple_size<tuple<Ts...>> : std::integral_constant<
    //     std::size_t, sizeof...(Ts)
    // >{};
    // template <typename tuple_type>
    // constexpr std::size_t size_v = tuple_size<tuple_type>::value;

    namespace details {
        constexpr static std::size_t npos = static_cast<std::size_t>(-1);
    }

    // tuple_common_reference
    template <
        concepts::tuple T,
        concepts::tuple U,
        template <typename> class TQual,
        template <typename> class UQual
    >
    struct tuple_common_reference;

    template <
        typename ... Ts,
        typename ... Us,
        template <typename> class TQual,
        template <typename> class UQual
    >
    requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::common_reference_with<TQual<Ts>, UQual<Us>>)
    struct tuple_common_reference<
        tuple<Ts...>, tuple<Us...>,
        TQual, UQual
    >
    {
        using type = tuple<
            std::common_reference_t<
                TQual<Ts>,
                UQual<Us>
            >...
        >;
    };
    // TODO(Guillaume): concepts::tuplelike instead of concepts::tuple
    template <
        concepts::tuple T,
        concepts::tuple U,
        template <typename> class TQual,
        template <typename> class UQual
    >
    requires (concepts::tuple<T> or concepts::tuple<U>)
    and std::same_as<T, std::remove_cvref_t<T>>
    and std::same_as<U, std::remove_cvref_t<U>>
    and (std::tuple_size_v<T> == std::tuple_size_v<U>)
    using tuple_common_reference_t = typename tuple_common_reference<
        T, U,
        TQual, UQual
    >::type;
}

// NOTE: C++23: std::basic_common_reference<tuple-like> **should** be enough,
//  as csl::mp::tuple meets the tuplelike interface
//  see https://en.cppreference.com/w/cpp/utility/tuple/basic_common_reference
//
//  but such a specialization is provided by libstdc++'s <tuple> header,
//  which <csl/mp.hpp> is decoupled from.
//
//  Also, /usr/include/c++/14/tuple with >= C++23 `__glibcxx_tuple_like`,
//  __is_tuple_v is still a specialization for tuple, pair and array
//
// See https://eel.is/c++draft/meta.trans.other#1
//
// TODO(Guillaume) Make sure it does not clash with std::tuple when both <csl/mp.hpp> and <tuple> are included
template <
    typename ... Ts,
    typename ... Us,
    template <typename> class TQual,
    template <typename> class UQual
>
requires
    (sizeof...(Ts) == sizeof...(Us))
and (true and ... and std::common_reference_with<Ts, Us>)
struct std::basic_common_reference< // NOLINT(cert-dcl58-cpp)
    csl::mp::tuple<Ts...>,
    csl::mp::tuple<Us...>,
    TQual, UQual
>
: csl::mp::tuple_common_reference<
    csl::mp::tuple<Ts...>,
    csl::mp::tuple<Us...>,
    TQual, UQual
>
{};

// tuple
namespace csl::mp {

    // TODO(Guillaume): #285 - interop with other tuple-like (pair, array, etc.)
    // - construction
    // - assign
    // - as/cast/convert

    template <typename ... Ts>
    struct tuple
    {
        using type = tuple<Ts...>;
        constexpr static auto size = sizeof...(Ts);
        using storage_type = details::make_tuple_storage_t<std::make_index_sequence<size>, Ts...>;

    private:

        storage_type storage;

    public:

    // storage
        constexpr tuple()
        noexcept ((true and ... and std::is_nothrow_constructible_v<Ts>))
        requires  (true and ... and std::is_default_constructible_v<Ts>)
        = default;
        constexpr tuple(const tuple &)
        noexcept ((true and ... and std::is_nothrow_copy_constructible_v<Ts>))
        requires  (true and ... and std::is_copy_constructible_v<Ts>)
        = default;
        constexpr tuple(tuple &&)
        noexcept ((true and ... and std::is_nothrow_move_constructible_v<Ts>))
        requires  (true and ... and std::is_move_constructible_v<Ts>)
        = default;
        constexpr tuple & operator=(const tuple &)
        noexcept ((true and ... and std::is_nothrow_copy_assignable_v<Ts>))
        requires  (true and ... and std::is_copy_assignable_v<Ts>)
        = default;
        constexpr tuple & operator=(tuple &&)
        noexcept ((true and ... and std::is_nothrow_move_assignable_v<Ts>))
        requires  (true and ... and std::is_move_assignable_v<Ts>)
        = default;
        constexpr ~tuple() = default;

    // Converting constructors: safe use are handled by the compiler -Wconversion
    #if CSL_MP_TUPLE__IMPLICIT_CONVERSION

    // operator=
    // storage conversion
        template <std::convertible_to<Ts> ... Us>
        constexpr auto & operator=(tuple<Us...> && other)
        noexcept((true and ... and std::is_nothrow_assignable_v<Ts&, Us&&>))
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::is_assignable_v<Ts, Us&&>)
        {
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return ((get<indexes>() = std::move(other).template get<indexes>()), ...);
            }(std::make_index_sequence<sizeof...(Ts)>{});
            return *this;
        }
        template <std::convertible_to<Ts> ... Us>
        constexpr auto & operator=(const tuple<Us...> & other)
        noexcept((true and ... and std::is_nothrow_assignable_v<Ts&, const Us&>))
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::is_assignable_v<Ts&, const Us&>)
        {
            [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return ((get<indexes>() = csl_fwd(other).template get<indexes>()), ...);
            }(std::make_index_sequence<sizeof...(Ts)>{});
            return *this;
        }

    #endif

        // NOLINTBEGIN(*explicit-constructor) conditionaly explicit
        // Constructor: direct
        // NOTE: ambiguous with tuple(Ts && ... args) on tuple<const int &, const char &>
        // constexpr explicit
        // tuple(const Ts & ... args)
        // noexcept((std::is_nothrow_constructible_v<Ts, decltype(args)> and ...))
        // requires
        //     (sizeof...(Ts) not_eq 0) // disambiguate with default constructor
        // and (std::constructible_from<Ts, decltype(args)> and ...)
        // : storage{ csl_fwd(args)... }
        // {}
        constexpr explicit
        tuple(Ts && ... args)
        noexcept((std::is_nothrow_constructible_v<Ts, decltype(args)> and ...))
        requires
            (sizeof...(Ts) not_eq 0) // disambiguate with default constructor
        and (std::constructible_from<Ts, decltype(args)> and ...)
        : storage{ csl_fwd(args)... }
        {}

        // Constructor: converting (values...)
        template <typename ... Us>
        constexpr explicit(not (true and ... and std::convertible_to<Ts, Us&&>))
        tuple(Us && ... args) // NOLINT(*-forward)
        noexcept((std::is_nothrow_constructible_v<Ts, Us&&> and ...))
        requires
            (sizeof...(Ts) not_eq 0) // disambiguate with default constructor
        and (sizeof...(Ts) == sizeof...(Us))
        and (std::constructible_from<Ts, Us&&> and ...)
        : storage{ csl_fwd(args)... }
        {}

    // Converting constructors: safe use are handled by the compiler -Wconversion
    #if CSL_MP_TUPLE__IMPLICIT_CONVERSION
        template <typename ...> friend struct tuple;

        // Constructor: converting move
        template <typename ... Us>
        constexpr explicit(not (true and ... and std::convertible_to<Ts, Us&&>))
        tuple(tuple<Us...> && other)
        noexcept(std::is_nothrow_constructible_v<decltype(storage), decltype(std::move(other.storage))>)
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::constructible_from<Ts, Us&&>)
        : storage{ std::move(std::move(other).storage) }
        {}
        // Constructor: converting copy
        template <typename ... Us>
        constexpr explicit(not (true and ... and std::convertible_to<Ts, const Us &>))
        tuple(const tuple<Us...> & other)
        noexcept(std::is_nothrow_constructible_v<decltype(storage), decltype(other.storage)>)
        requires (true and ... and std::constructible_from<Ts, const Us&>)
        : storage{ other.storage }
        {}
    #endif
        // NOLINTEND(*explicit-constructor)

        // compare
        template <typename ... Us>
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::equality_comparable_with<Ts, Us>)
        constexpr auto operator==(const tuple<Us...> & other) const
        noexcept((true && ... && noexcept(std::declval<const Ts&>() == std::declval<const Us &>())))
        {
            return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return (true and ... and (get<indexes>() == other.template get<indexes>()));
            }(std::make_index_sequence<sizeof...(Ts)>{});
        }
        template <typename ... Us>
        requires
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::three_way_comparable_with<Ts, Us>)
        constexpr auto operator<=>(const tuple<Us...> & other) const
        noexcept((true && ... && noexcept(std::declval<const Ts&>() <=> std::declval<const Us &>())))
        -> std::common_comparison_category_t<
            details::compare::synth_three_way_result<Ts, Us>...
        >
        {
            using category_t = std::common_comparison_category_t<
                details::compare::synth_three_way_result<Ts, Us>...
            >;
            return [&]<std::size_t... indexes>(std::index_sequence<indexes...>) {
                category_t cmp = category_t::equivalent;
                (
                    (
                        cmp = cmp not_eq category_t::equivalent
                            ? cmp
                            : (get<indexes>() <=> other.template get<indexes>())
                    )
                , ...);
                return cmp;
            }(std::index_sequence_for<Ts...>{});
        }

    #pragma region accessors
    #pragma region tuple::get
        template <std::size_t index> requires (index >= size)
        constexpr void get() const & noexcept {
            static_assert([](){ return false; }(), "csl::mp::tuple::get<size_t>: out-of-bounds");
        }

    // P0847R7 Explicit object parameter (deducing this)
    #if defined(__cpp_explicit_this_parameter) \
          and __cpp_explicit_this_parameter >= 202110L
        template <std::size_t index> requires (index < size)
        [[nodiscard]] constexpr auto && get(this auto && self) noexcept {
            using accessor_t = copy_cvref_t<
                decltype(csl_fwd(self)),
                typename storage_type::template by_index_<index>
            >;
            return static_cast<accessor_t>(self.storage).value;
        }

        // tuple[index<N>]...
        //  as tuple[N]... would requires some tuple.template operator[]<N>()...
        template <std::size_t index> requires (index < size)
        [[nodiscard]] constexpr auto && operator[](this auto && self, index_t<index>) noexcept {
            return csl_fwd(self).template get<index>();
        }
    #else
    // clang-18.1.8 does not support __cpp_explicit_this_parameter
        template <std::size_t index> requires (index < size)
        [[nodiscard]] constexpr auto & get() & noexcept {
            using accessor = typename storage_type::template by_index_<index>;
            return static_cast<accessor&>(storage).value;
        }
        template <std::size_t index> requires (index < size)
        [[nodiscard]] constexpr const auto & get() const & noexcept {
            using accessor = typename storage_type::template by_index_<index>;
            return static_cast<const accessor&>(storage).value;
        }
        template <std::size_t index> requires (index < size)
        [[nodiscard]] constexpr auto && get() && noexcept {
            using accessor = typename storage_type::template by_index_<index>;
            return static_cast<accessor &&>(std::move(storage)).value;
        }
        template <std::size_t index> requires (index < size)
        [[nodiscard]] constexpr const auto && get() const && noexcept {
            using accessor = typename storage_type::template by_index_<index>;
            return static_cast<const accessor &&>(std::move(storage)).value;
        }
    #endif
    #pragma endregion
    #pragma endregion

    // QUESTION: storage accessors
        // use tuple_member_value ?
        // get/at/operator[] -> cvref qualifiers matrix
        // - index
        // index_of<T>
        // assign/operator=(tuple<Us...>) if (true and ... and std::assignable_to<Ts, Us>)
        // compare/operator<=>

        // visit/operator()
        // Integrate some data_store design
    };
    template <typename ... Ts>
    tuple(Ts && ...) -> tuple<std::remove_cvref_t<Ts>...>;

    // compare
    // template <typename ... Ts, typename ... Us>
    // requires (sizeof...(Ts) == sizeof...(Us))
    //      and (true and ... and std::three_way_comparable_with<Ts, Us>)
    // constexpr inline static auto operator<=>(
    //     const tuple<Ts...> & lhs,
    //     const tuple<Us...> & rhs
    // ){
    //     return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //         return ((get<indexes>(lhs) <=> get<indexes>(rhs)) <=> ...);
    //     }(std::make_index_sequence<sizeof...(Ts)>{});
    // }
}
// tuple: indexing deduction
namespace csl::mp::details::concepts {

    // deductible: type
    template <typename tuple_type, typename T>
    concept can_deduce_by_type = is_tuple_v<tuple_type>
        and requires { std::void_t<typename tuple_type::storage_type::template by_type_<T>>(); }
    ;
    // deductible: type element
    template <typename tuple_type, std::size_t I>
    concept can_deduce_by_index = is_tuple_v<tuple_type>
        and requires { std::void_t<typename tuple_type::storage_type::template by_index_<I>>(); }
    ;
}

// REFACTO: move into function algos
namespace csl::mp {
    // predicate: TTP to std::predicate adapter
    template <template <typename> class P>
    struct predicate {
        // std::predicate
        template <typename T>
        [[nodiscard]] constexpr static auto operator()()          noexcept { return P<T>::value; }
        // deduced T
        template <typename T>
        [[nodiscard]] constexpr static auto operator()(const T &) noexcept { return P<T>::value; }
    };
}

// mp algorithms
// REFACTO: naming: algos as type-traits using mp/ttp vs. functions using values
namespace csl::mp {

    template <std::size_t index, concepts::tuple_like T>
    using nth = std::tuple_element_t<index, T>;

    // is_homogeneous
    template <typename T>
    struct is_homogeneous : std::false_type{};
    template <concepts::tuple_like T>
    struct is_homogeneous<T> : std::bool_constant<
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and std::same_as<element<0, T>, element<indexes, T>>);
        }(std::make_index_sequence<size_v<T>>{})
    >{};
    template <typename T>
    constexpr bool is_homogeneous_v = is_homogeneous<T>::value;

    namespace concepts {
        template <typename T> concept homogeneous = concepts::tuple_like<T> and is_homogeneous_v<std::remove_cvref_t<T>>;
    }

    // is_constrained_by
    template <typename, template <typename> typename>
    struct is_constrained_by : std::false_type{};
    template <concepts::tuple_like T, template <typename> typename predicate>
    struct is_constrained_by<T, predicate> : std::bool_constant<
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and predicate<element<indexes, T>>::value);
        }(std::make_index_sequence<size_v<T>>{})
    >{};
    template <typename T, template <typename> typename predicate>
    constexpr auto is_constrained_by_v = is_constrained_by<T, predicate>::value;

    namespace concepts {
        template <typename T, template <typename> typename predicate>
        concept constrained_by = concepts::tuple_like<T> and is_constrained_by_v<std::remove_cvref_t<T>, predicate>;
    }

    // TODO(Guillaume) naming: namespace ? ⬆️⬇️
    //  - csl::mp::tuple_traits::is_valid
    //  - csl::mp::traits::is_valid
    //  - csl::mp::tt::is_valid

    // empty
    template <typename>
    struct empty : std::false_type{};
    template <concepts::tuple_like T>
    struct empty<T> : std::integral_constant<bool, (size_v<T> == 0)>{};
    template <typename T>
    constexpr bool empty_v = empty<T>::value;

    // WIP
    // REFACTO: first arg is tuplelike, then the rest.
    //  QUESTION: specialize for tuple<Ts...>, then other tuplelikes ?
    //      and/or rebind any tuplelike into tuple<...> for simplier impls. ?
    // REFACTO: apply/apply_enumerated/for_each/for_each_enumerated instead of IILEs + index_seq
    // FEATURE: 1 type-trait -> 1 concept
    //  type_trait -> T
    //  concepts -> maybe-cvref-qualified T
    // DESIGN: Need CT benchmark for each impl pattern
    // WIP

    // index-by-type: index of the first occurence of T, if any, and tuple_size otherwise
    //  QUESTION: should value be Index<N> rather than integral_constant<std::size_t, N> ?
    template <typename, typename>
    struct index_of{};
    template <typename tuple_type, typename T>
    requires details::concepts::can_deduce_by_type<tuple_type, T>
    struct index_of<tuple_type, T> : std::integral_constant<std::size_t,
        // QUESTION: is it worthy ? If so, would a rebind like unfold_to<tuple_type, tuple> be worth ?
        tuple_type::storage_type::template by_type_<T>::index
    >{};
    template <concepts::tuple_like tuple_type, typename T>
    requires (not details::concepts::can_deduce_by_type<tuple_type, T>)
    struct index_of<tuple_type, T> : std::integral_constant<std::size_t,
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            constexpr bool matches[] = { std::is_same_v<T, std::tuple_element_t<indexes, std::remove_cvref_t<tuple_type>>>... }; // NOLINT(*c-arrays)
            for (std::size_t i = 0; i < sizeof...(indexes); ++i)
                if (matches[i])
                    return i;
            return sizeof...(indexes);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{})
    >{};
    // struct index_of<tuple<Ts...>, T> {
    //     constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //         std::size_t pos = details::npos;
    //         (void)((pos = (pos == details::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
    //         return pos;
    //     }(std::make_index_sequence<sizeof...(Ts)>{});
    //     static_assert(value not_eq details::npos, "first_index_of : not found");
    // };
    template <concepts::tuple_like tuple_type, typename T>
    constexpr std::size_t index_of_v = index_of<tuple_type, T>::value;

    // last_index_of
    //  equivalent to (sizeof...(Ts) - index_of<T, reverse<tuple<Ts...>>>)
    template <typename, typename>
    struct last_index_of{};

    template <typename tuple_type, typename T>
    requires details::concepts::can_deduce_by_type<tuple_type, T>
    struct last_index_of<tuple_type, T> : index_of<tuple_type, T>{};

    template <concepts::tuple_like tuple_type, typename T>
    requires (not details::concepts::can_deduce_by_type<tuple_type, T>)
    // REFACTO: index_of<reverse<tuple_type>, T> ?
    struct last_index_of<tuple_type, T> : std::integral_constant<std::size_t,
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = sizeof...(indexes);
            (void)((pos = std::is_same_v<T, std::tuple_element_t<indexes, std::remove_cvref_t<tuple_type>>>
                        ? indexes
                        : pos
            ), ...);
            return pos;
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{})
    >
    {};
    template <typename tuple_type, typename T>
    constexpr std::size_t last_index_of_v = last_index_of<tuple_type, T>::value;

    // count
    template <typename, typename>
    struct count;
    template <typename tuple_type, typename T>
    requires details::concepts::can_deduce_by_type<tuple_type, T>
    struct count<tuple_type, T> : std::integral_constant<std::size_t, 1>{};
    template <concepts::tuple_like tuple_type, typename T>
    requires (not details::concepts::can_deduce_by_type<tuple_type, T>)
    struct count<tuple_type, T> : std::integral_constant<std::size_t,
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (0 + ... + 
                std::is_same_v<T, std::tuple_element_t<indexes, tuple_type>>
            );
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{})
    >{};
    template <concepts::tuple_like tuple_type, typename T>
    constexpr std::size_t count_v = count<tuple_type, T>::value;

    template <concepts::tuple_like tuple_type, template <typename...> typename predicate>
    struct count_if : std::integral_constant<std::size_t,
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (0 + ... + 
                predicate<std::tuple_element_t<indexes, tuple_type>>::value
            );
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{})
    >{};
    // QUESTION: using csl::mp::predicate<P> ? template vs. non-template wrapper for better semantic ?
    //             predicate<P>::template operator()<std::tuple_element_t<indexes, tuple_type>>()
    template <concepts::tuple_like tuple_type, template <typename...> typename predicate>
    constexpr std::size_t count_if_v = count_if<tuple_type, predicate>::value;

    // is_type_gettable: get<T>(tuple-like) would be legal and not error-prone
    //  NOTE(design) cannot detect get<T>(tuple-like): https://godbolt.org/z/vqvMYf7zc as error is handled by static-assert
    //  NOTE(naming) has_duplicate/is_uniqued won't fit as one cannot `get<int>(std::array{1})` anyway
    //
    template <typename, typename>
    struct is_type_gettable : std::false_type{};
    template <typename T, typename ... Ts>
    struct is_type_gettable<tuple<Ts...>, T> : std::bool_constant<
        details::concepts::can_deduce_by_type<tuple<Ts...>, T>
    >{};
    template <concepts::tuple_like tuple_type, typename T>
    requires (not concepts::std_array<tuple_type>)
    struct is_type_gettable<tuple_type, T> : std::bool_constant<
        count_v<std::remove_cvref_t<tuple_type>, T> == 1
    >{};
    template <concepts::tuple_like tuple_type, typename T>
    constexpr bool is_type_gettable_v = is_type_gettable<tuple_type, T>::value;

    namespace concepts {
        template <typename tuple_type, typename T>
        concept type_gettable = is_type_gettable_v<tuple_type, std::remove_cvref_t<T>>;
    }

    // is_type_gettable: conjunction<is_type_gettable<tuple-like, tuple-elements>...>
    template <typename>
    struct support_get_by_type : std::false_type{};
    template <concepts::tuple_like T>
    requires (not concepts::std_array<T>)
    struct support_get_by_type<T> : std::bool_constant<
        []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and is_type_gettable_v<
                std::remove_cvref_t<T>,
                std::tuple_element_t<indexes, std::remove_cvref_t<T>>
            >);
        }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{})
    >{};
    template <concepts::tuple_like T>
    constexpr bool support_get_by_type_v = support_get_by_type<T>::value;

    namespace concepts {
        template <typename T> concept support_get_by_type = support_get_by_type_v<std::remove_cvref_t<T>>;
    }

    // NOTE: pref. concepts::sized_at_least in most cases
    template <typename, std::size_t>
    struct is_index_gettable : std::false_type{};
    template <concepts::tuple_like tuple_type, std::size_t N>
    struct is_index_gettable<tuple_type, N> : std::integral_constant<bool, (N < std::tuple_size_v<tuple_type>)>{};
    template <concepts::tuple_like tuple_type, std::size_t N>
    constexpr auto is_index_gettable_v = is_index_gettable<tuple_type, N>::value;

    namespace concepts {
        template <typename T, std::size_t N> concept index_gettable = is_index_gettable_v<std::remove_cvref_t<T>, N>;
    }

    // is_uniqued: has no duplicate (don't matter if sorted or not)
    //  named after P2848 - std::uniqued - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2848r1.html
    template <typename>
    struct is_uniqued;
    template <typename T, std::size_t N>
    struct is_uniqued<std::array<T, N>> : std::bool_constant<(N > 1)>{};
    template <concepts::tuple_like tuple_type>
    struct is_uniqued<tuple_type> : support_get_by_type<tuple_type>{};
    template <typename tuple_type>
    constexpr bool is_uniqued_v = is_uniqued<tuple_type>::value;

    // QUESTION(perf.) faster ? array{ index_of_v<tuple_type, index>... } == array{ last_index_of_v<tuple_type, index>... }
    //
    // template <concepts::tuple_like tuple_type>
    // struct is_uniqued<tuple_type> : std::bool_constant<
    //     []<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //         using tuple_type_t = std::remove_cvref_t<tuple_type>;
    //         return (true and ... and (
    //                  index_of_v<tuple_type_t, std::tuple_element_t<indexes, std::remove_cvref_t<tuple_type>>>
    //         ==  last_index_of_v<tuple_type_t, std::tuple_element_t<indexes, std::remove_cvref_t<tuple_type>>>
    //     ));
    //     }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<tuple_type>>>{})
    // >{};

    namespace concepts {
        template <typename T>
        concept uniqued = is_uniqued_v<std::remove_cvref_t<T>>;
    }

    // unfold
    //  REFACTO: universal ttps
    template <typename, template <typename...> typename>
    struct unfold;
    template <typename ... Ts, template <typename...> typename destination>
    struct unfold<destination<Ts...>, destination> : std::type_identity<destination<Ts...>>{};
    // REFACTO: for_each_index ?
    template <concepts::tuple_like tuple_type, template <typename...> typename destination>
    struct unfold<tuple_type, destination> {
    private:
        // NOTE(impl.): using lambdas here (dependent-name and the closure-types) mess up with ADL
        template <std::size_t... Is>
        constexpr static auto helper(std::index_sequence<Is...>)
            -> destination<std::tuple_element_t<Is, tuple_type>...>;
    public:
        using type = decltype(helper(std::make_index_sequence<std::tuple_size_v<tuple_type>>{}));
    };
    template <typename tuple_type, template <typename...> typename destination>
    using unfold_t = typename unfold<tuple_type, destination>::type;

    // rebind
    template <typename T, typename... Us>
    struct rebind{
    private:
        template <template <typename ...> typename ttp, typename ... Ts>
        constexpr static auto helper(ttp<Ts...>&&) -> ttp<Us...>;
    public:
        using type = decltype(helper(
            std::declval<T>()
        ));
    };
    template <typename T, std::size_t N, typename U>
    struct rebind<std::array<T,N>, U> : std::type_identity<std::array<U,N>>{};
    template <typename... Us, typename... Ts>
    struct rebind<csl::mp::tuple<Ts...>, Us...> : std::type_identity<csl::mp::tuple<Us...>>{};
    template <typename ... Us, typename... Ts>
    struct rebind<std::tuple<Ts...>, Us...> : std::type_identity<std::tuple<Us...>>{};
    template <typename T0, typename T1, typename U0, typename U1>
    struct rebind<std::pair<T0, T1>, U0, U1> : std::type_identity<std::pair<U0, U1>>{};
    template <typename ttp, typename ... Ts>
    using rebind_t = typename rebind<ttp, Ts...>::type;

    // transform
    //  QUESTION: DRY vs. perfs. vs. API, scalability ?
    template <concepts::tuple_like tuple_type, template <typename...> typename transformation>
    struct transform {
    private:
        template <std::size_t... Is>
        constexpr static auto helper(std::index_sequence<Is...>)
            -> rebind_t<tuple_type, transformation<std::tuple_element_t<Is, tuple_type>>...>;
    public:
        using type = decltype(helper(std::make_index_sequence<std::tuple_size_v<tuple_type>>{}));
    };
    template <template <typename...> typename transformation, typename ... Ts>
    struct transform<csl::mp::tuple<Ts...>, transformation>
        : std::type_identity<csl::mp::tuple<transformation<Ts>...>>{};

    template <typename tuple_type, template <typename...> typename Transformation>
    using transform_t = typename transform<tuple_type, Transformation>::type;

    // TODO(Guillaume) regroup/move functions, partition from traits -> dedicated namespace ?
    constexpr auto tie(auto & ... values) -> csl::mp::tuple<decltype(values)...>{
        return csl::mp::tuple<decltype(values)...>{ csl_fwd(values)... };
    }
    // tie_result ?

    [[nodiscard]] constexpr auto make_tuple(auto && ... args) -> tuple<unwrap_ref_decay_t<decltype(args)>...> {
        return tuple<unwrap_ref_decay_t<decltype(args)>...>{ csl_fwd(args)... };
    }
    // make_tuple_result ?

    constexpr auto forward_as_tuple(auto && ... values) -> csl::mp::tuple<decltype(values)...>{
        return csl::mp::tuple<decltype(values)...>{ csl_fwd(values)... };
    }
    // forward_as_tuple_result ?

    // WIP --- 🏗️ --- revert API so it looks like std::ranges
    // WIP(Guillaume) tests

    // tuple_cat
    constexpr auto cat(){ return csl::mp::tuple{}; }
    constexpr auto cat(/* TODO: tuplelike */ auto && ... tuples)
    requires (sizeof...(tuples) not_eq 0)
    {
        constexpr auto size = (... + size_v<decltype(tuples)>);

        if constexpr (size == 0)
            return csl::mp::tuple<>{};
        else
        {
            // TODO(Guillaume): use `make_two_dimensions_indexes` from the old `flatten` poc ?
            constexpr auto indexes_map = [&](){

                // scenario:
                // t0 = [ a b . . . ]
                // t1 = [ . . c d . ]
                // t2 = [ . . . . e ]

                //  tuple_I    || element_I
                // --------------------------
                // [ . . . . . ][ . . . . . ]
                // --------------------------
                // [ 0 . . . . ][ 0 . . . . ]
                // [ 0 0 . . . ][ 0 1 . . . ]
                // --------------------------
                // [ 0 0 1 . . ][ 0 1 0 . . ]
                // [ 0 0 1 1 . ][ 0 1 0 1 . ]
                // --------------------------
                // [ 0 0 1 1 2 ][ 0 1 0 1 0 ]

                struct {
                    std::size_t tuple_index[size];      // NOLINT(*-avoid-c-arrays)
                    std::size_t element_index[size];    // NOLINT(*-avoid-c-arrays)
                } mapped_indexes{};

                auto create_indexes_for = [
                    &,
                    tuple_index = std::size_t{0},
                    offset = std::size_t{0}
                ]<std::size_t ... indexes>(std::index_sequence<indexes...>) mutable {

                    ((
                        (mapped_indexes.tuple_index  [offset + indexes] = tuple_index),
                        (mapped_indexes.element_index[offset + indexes] = indexes)
                    ), ...);

                    offset += sizeof...(indexes);
                    ++tuple_index;
                };

                // create indexes for each tuple
                ((create_indexes_for(std::make_index_sequence<
                    size_v<decltype(tuples)>
                >{})), ...);

                return mapped_indexes;
            }();

            return []<std::size_t ... indexes>(auto && tuple_of_tuples, std::index_sequence<indexes...>){

                // FEATURE: if only `std::` tuplelike, consider std::tuple ?
                using type = csl::mp::tuple<
                    std::tuple_element_t<
                        indexes_map.element_index[indexes],
                        std::remove_cvref_t<std::tuple_element_t<
                            indexes_map.tuple_index[indexes],
                            std::remove_cvref_t<decltype(tuple_of_tuples)>
                        >>
                    >...
                >;
                return type{
                    get<indexes_map.element_index[indexes]>(
                        get<indexes_map.tuple_index[indexes]>(
                            csl_fwd(tuple_of_tuples)
                        )
                    )...
                };
            }(
                csl::mp::forward_as_tuple(csl_fwd(tuples)...),
                std::make_index_sequence<size>{}
            );
        }
    }

    // tuple_cat_result
    template <typename ... tuple_types>
    struct cat_result : csl::mp::type_identity<
        decltype(cat(std::declval<tuple_types>()...))
    >{};
    template <typename ... tuple_types>
    using cat_result_t = typename cat_result<tuple_types...>::type;

    // contains
    // build benchmark (~ +20% perfs): https://build-bench.com/b/Ir0K0cw2wfFLyRYPYYIMVASDYQU
    template <typename tuple_type, typename T>
    struct contains;
    template <concepts::tuple_like tuple_type, typename T>
    struct contains<tuple_type, T> {
    private:
        template <std::size_t... Is>
        constexpr static auto helper(std::index_sequence<Is...>)
            -> std::disjunction<std::is_same<T, std::tuple_element_t<Is, tuple_type>>...>;
    public:
        constexpr static auto value = decltype(helper(std::make_index_sequence<std::tuple_size_v<tuple_type>>{}))::value;
    };
    template <typename tuple_type, typename T>
    requires details::concepts::can_deduce_by_type<tuple_type, T>
    struct contains<tuple_type, T> : std::true_type{};
    template <typename T, typename ... Ts>
    struct contains<tuple<Ts...>, T> : std::bool_constant<
        (false or ... or std::same_as<T, Ts>)
    >{};
    template <typename T, typename value_type, std::size_t N>
    struct contains<std::array<value_type, N>, T> : std::is_same<T, value_type>{};
    template <typename tuple_type, typename T>
    constexpr bool contains_v = contains<tuple_type, T>::value;

    // filter<trait>
    //  Supports only csl::mp::tuple and std::tuple, as array, pair, etc. won't make much sens
    template <typename, template <typename...> typename>
    struct filter;
    template <typename ... Ts, template <typename...> typename predicate>
    struct filter<tuple<Ts...>, predicate> : cat_result<
        std::conditional_t<
            predicate<Ts>::value,
            tuple<Ts>,
            tuple<>
        >...
    >{};
    //  Limitation: less performant implementation
    template <typename ... Ts, template <typename> typename predicate>
    struct filter<std::tuple<Ts...>, predicate> : unfold<
        typename filter<csl::mp::tuple<Ts...>, predicate>::type,
        std::tuple
    >{};
    template <typename tuple_type, template <typename...> typename predicate>
    using filter_t = typename filter<tuple_type, predicate>::type;

    // set_union
    template <typename, typename>
    struct set_union;
    template <concepts::tuple_like t1, concepts::tuple_like t2>
    struct set_union<t1, t2>{
    private:

        // REFACTO: cat_result<t1, filter_t<bind_front<contains, t1>::type>
        template <std::size_t... t2_Is>
        constexpr static auto helper(std::index_sequence<t2_Is...>)
            -> cat_result<
                t1,
                std::conditional_t<
                    contains_v<t1, std::tuple_element_t<t2_Is, t2>>,
                    tuple<>,
                    tuple<std::tuple_element_t<t2_Is, t2>>
                >...
            >;
        // QUESTION(design) rebind as t1 ?
    public:
        using type = decltype(helper(std::make_index_sequence<std::tuple_size_v<t2>>{}))::type;
    };
    template <typename T, typename U>
    using set_union_t = typename set_union<T, U>::type;

    // set_intersection
    template <typename, typename>
    struct set_intersection;
    template <concepts::tuple_like t1, concepts::tuple_like t2>
    struct set_intersection<t1, t2> {
    private:
        template <std::size_t... t1_Is>
        constexpr static auto helper(std::index_sequence<t1_Is...>)
            -> cat_result<
                std::conditional_t<
                    contains_v<t2, std::tuple_element_t<t1_Is, t1>>,
                    tuple<std::tuple_element_t<t1_Is, t1>>,
                    tuple<>
                >...
            >;
    public:
        using type = typename decltype(helper(std::make_index_sequence<std::tuple_size_v<t1>>{}))::type;
    };
    template <typename T, typename U>
    using set_intersection_t = typename set_intersection<T, U>::type;

    // set_difference
    //  Result in the elements from (sorted) input tuplelike T1 which are NOT found in the (sorted) tuplelike T2
    //  REFACTO: simplier, better perfs
    template <typename, typename>
    struct set_difference;
    template <concepts::tuple_like T1, concepts::tuple_like T2>
    struct set_difference<T1, T2>{
    private:
        template <std::size_t ... T1_Is, std::size_t ... T2_Is>
        consteval static auto make_mask(
            std::index_sequence<T1_Is...>,
            std::index_sequence<T2_Is...>
        ) {

            auto should_keep = [
                used = std::array<bool, std::tuple_size_v<T2>>{}
            ]<typename T1_element>() mutable{

                std::optional<std::size_t> index;
                (
                    (
                        std::is_same_v<T1_element, std::tuple_element_t<T2_Is, T2>>
                        and not used[T2_Is]
                        and not index
                        ? index = T2_Is
                        : index
                    ), ...
                );
                if (index){
                    used[index.value()] = true;
                    return false;
                }
                return true;
            };

            std::array<bool, std::tuple_size_v<T1>> result{};
            ((result[T1_Is] = should_keep.template operator()<std::tuple_element_t<T1_Is, T1>>()), ...);
            return result;
        }

        constexpr static auto mask = make_mask(
            std::make_index_sequence<std::tuple_size_v<T1>>{},
            std::make_index_sequence<std::tuple_size_v<T2>>{}
        );

        template <std::size_t... Is>
        consteval static auto helper(std::index_sequence<Is...>)
            -> cat_result<
                std::conditional_t<
                    mask[Is],
                    tuple<std::tuple_element_t<Is, T1>>,
                    tuple<>
                >...
            >;
    public:
        using type = decltype(helper(std::make_index_sequence<std::tuple_size_v<T1>>{}))::type;
    };
    template <typename T, typename U>
    using set_difference_t = typename set_difference<T, U>::type;

    // deduplicate / make_valid / make_unique
    template <typename>
    struct deduplicate;
    template <concepts::support_get_by_type T>
    struct deduplicate<T> : type_identity<T>{};
    template <typename ... Ts>
    struct deduplicate<csl::mp::tuple<Ts...>> {
    private:
        template <std::size_t... Is>
        constexpr static auto helper(std::index_sequence<Is...>)
            -> cat_result_t<
                std::conditional_t<
                    details::concepts::can_deduce_by_type<tuple<Ts...>, Ts> // not unique
                        or Is == index_of_v<tuple<Ts...>, Ts>,              // first occurence
                    tuple<Ts>,
                    tuple<>
                >...
            >
        ;
    public:
        using type = decltype(helper(std::make_index_sequence<sizeof...(Ts)>{}));
    };
    template <typename ... Ts>
    struct deduplicate<std::tuple<Ts...>> : unfold<
        typename deduplicate<csl::mp::tuple<Ts...>>::type,
        std::tuple
    >{};
    template <typename tuple_type>
    using deduplicate_t = typename deduplicate<tuple_type>::type;

    // flatten_once
    // flatten / make_flat

    // fwd_as_tuple
    // get<I>, get<T>
    // // for_each
    // template <concepts::valid_tuple tuple_type>
    // void for_each(tuple_type && value, auto && visitor){
    //     [&](){}();
    // }

    // for_each_with_index
    // template <concepts::valid_tuple tuple_type>
    // void for_each_with_index(tuple_type && value, auto && visitor){
    //     [&](){}();
    // }

    // projection: column vs. row
    // column: projection by index -> tuple{ tuple{ int, char }, tuple{ int, char } }
    //  project_by_index<0>(tuple_of_tuplelike) -> tuple{ int, int }
    //  project_by_index<1>(tuple_of_tuplelike) -> tuple{ char, char }

    // tuple_storage: if are_same<Ts...> -> use std::array instead

    // print:
    //  fmt (opt-in or detected dependencies)
    //      -> csl::ag-like compact vs. pretty printing
    //  std::ostream printing
}

// tuple: API
namespace csl::mp {

    template <std::size_t index>
    [[nodiscard]] constexpr auto get(concepts::tuple auto && value) noexcept -> decltype(auto) {
        return csl_fwd(value).template get<index>();
    }
    // TODO(Guillaume) get<T>
    //  
}

// std inter-operatiblity, structured binding
//  N4606 [namespace.std]/1 :
//      A program may add a template specialization for any standard library template to namespace std
//      only if the declaration depends on a user-defined type 
//      and the specialization meets the standard library requirements for the original template and is not explicitly prohibited.
namespace std {
// NOLINTBEGIN(cert-dcl58-cpp) Modification of 'std' namespace can result in undefined behavior
template <typename ... Ts>
struct tuple_size<csl::mp::tuple<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)>{};

// QUESTION: perfs vs. plain recursive inheritance,
//  as already provided by https://en.cppreference.com/w/cpp/utility/tuple/tuple_element ?
template <size_t index, typename ... Ts>
struct tuple_element<index, csl::mp::tuple<Ts...>> : csl::mp::tuple<Ts...>::storage_type::template by_index_<index>{};
// NOLINTEND(cert-dcl58-cpp)
}

// tuple algorithms
//  REFACTO: concepts::tuple -> tuple_like
namespace csl::mp {

    // QUESTION: primitive for
    // auto ??? (concepts::tuple auto && value, auto f){
    //     constexpr auto size = csl::mp::size_v<std::remove_cvref_t<decltype(value)>>;
    //     [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //         ((
    //             // arg<indexes...>(std::get<indexes>(csl_fwd(values))))
    //         ), ...);
    //     }(std::make_index_sequence<size>{});
    // }

    // REFACTO: for_each(F, tuple-likes...)
    // TODO(Guillaume) noexcept clauses
    // foreach
    auto for_each(concepts::tuple auto && value, auto f){
        constexpr auto size = csl::mp::size_v<std::remove_cvref_t<decltype(value)>>;
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((
                std::invoke(f, std::get<indexes>(csl_fwd(value)))
            ), ...);
        }(std::make_index_sequence<size>{});
        return f;
    }

    // QUESTION: what for f<indexes>(element) ?
    auto for_each_enumerate(concepts::tuple auto && value, auto f){
        constexpr auto size = csl::mp::size_v<std::remove_cvref_t<decltype(value)>>;
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((
                std::invoke(f, indexes, std::get<indexes>(csl_fwd(value)))
            ), ...);
        }(std::make_index_sequence<size>{});
        return f;
    }

    // apply
    namespace details::exposition_only {
        template <std::size_t... indexes>
        constexpr decltype(auto) apply(
            auto && f,
            csl::mp::concepts::tuple_like auto && value,
            std::index_sequence<indexes...>
        )
        noexcept(std::is_nothrow_invocable_v<decltype(f), decltype(get<indexes>(csl_fwd(value)))...>)
        requires std::invocable<decltype(f), decltype(get<indexes>(csl_fwd(value)))...>
        {
            return std::invoke(
                csl_fwd(f),
                get<indexes>(csl_fwd(value))...
            );
        }
    }
    constexpr decltype(auto) apply(auto && f, concepts::tuple_like auto && value)
    noexcept(noexcept(
        details::exposition_only::apply(
            csl_fwd(f), csl_fwd(value),
            std::make_index_sequence<size_v<std::remove_cvref_t<decltype(value)>>>{}
        )
    ))
    {
        return details::exposition_only::apply(
            csl_fwd(f), csl_fwd(value),
            std::make_index_sequence<size_v<std::remove_cvref_t<decltype(value)>>>{}
        );
    }

    namespace concepts {
        template <typename F, typename T>
        concept can_apply = requires {
            apply(std::declval<F>(), std::declval<T>());
        };
    }

    #pragma region fold
    // MVE: https://godbolt.org/z/z1so3dqee
    // WIP: non-mandatory init: split, head is init

    namespace details {
        template <typename F, typename T>
        struct folder {

            F f;        // Q: consider const-lvalue-reference and make stateful ops invalid to users from the API perspective ?
            T value;    // Q: requires T to be trivially movable ?

            // NOLINTBEGIN(*-not-moved)
            // left: f(f(f(f(init, x1), x2), ...), xn), where x1, x2, ..., xn are the tuple elements
            template <typename U>
            [[nodiscard]] friend constexpr auto operator<<(folder && lhs, folder<F, U> && rhs) -> decltype(auto) {
                return details::folder{ lhs.f, std::invoke(lhs.f, lhs.value, rhs.value) };
            }
            // right: f(x1, f(x2, ...f(xn, init))), where x1, x2, ..., xn are the tuple elements
            template <typename U>
            [[nodiscard]] friend constexpr auto operator>>(folder && lhs, folder<F, U> && rhs) -> decltype(auto) {
                return details::folder{ lhs.f, std::invoke(lhs.f, rhs.value, lhs.value ) };
            }
            // NOLINTEND(*-not-moved)
        };
        template <typename F, typename T>
        folder(F, T) -> folder<std::remove_cvref_t<F>, std::remove_cvref_t<T>>;
    }

    [[nodiscard]] constexpr auto fold_left(csl::mp::concepts::tuple_like auto && value, auto f, auto init)
    {
        if constexpr (0 == csl::mp::size_v<decltype(value)>)
            return init;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
            return (
                details::folder{ f, get<indexes>(csl_fwd(value)) }
                << ...
                << details::folder{ f, init }
            ).value;
        }(std::make_index_sequence<csl::mp::size_v<decltype(value)>>{});
    }
    [[nodiscard]] constexpr auto fold_right(csl::mp::concepts::tuple_like auto && value, auto f, auto init)
    {
        if constexpr (0 == csl::mp::size_v<decltype(value)>)
            return init;
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>) {
            return (
                details::folder{ f, init }
                >> ...
                >> details::folder{ f, get<indexes>(csl_fwd(value)) }
            ).value;
        }(std::make_index_sequence<csl::mp::size_v<decltype(value)>>{});
    }

    template <csl::mp::concepts::tuple_like T, typename F, typename init>
    struct fold_left_result : type_identity<decltype(
        fold_left(
            std::declval<T>(),
            std::declval<F>(),
            std::declval<init>()
        )   
    )>{};
    template <csl::mp::concepts::tuple_like T, typename F, typename init>
    using fold_left_result_t = typename fold_left_result<T, F, init>::type;

    template <csl::mp::concepts::tuple_like T, typename F, typename init>
    struct fold_right_result : type_identity<decltype(
        fold_right(
            std::declval<T>(),
            std::declval<F>(),
            std::declval<init>()
        )   
    )>{};
    template <csl::mp::concepts::tuple_like T, typename F, typename init>
    using fold_right_result_t = typename fold_right_result<T, F, init>::type;

    #pragma endregion

    template <csl::mp::concepts::tuple T>
    [[nodiscard]] constexpr auto all_of(const T & value, std::predicate auto && p){
        // REFACTO: reduce
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and std::invoke(p, get<indexes>(value)));
        }(std::make_index_sequence<csl::mp::size_v<T>>{});
    }
    template <csl::mp::concepts::tuple T>
    [[nodiscard]] constexpr auto any_of(const T & value, std::predicate auto && p){
        // REFACTO: reduce
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (false or ... or std::invoke(p, get<indexes>(value)));
        }(std::make_index_sequence<csl::mp::size_v<T>>{});
    }
    template <csl::mp::concepts::tuple T>
    [[nodiscard]] constexpr auto none_of(const T & value, std::predicate auto && p){
        // REFACTO: reduce
        // return all_of(value, std::not_fn(csl_fwd(p)));
        return not all_of(value, csl_fwd(p));
    }

    // split
    // chunk_by: <N>, predicate
}

#if defined CSL_MP_TUPLE__EXPERIMENTALE
#include <execution>
#include <future>
namespace csl::mp::inline functions::experimentale {
    template <typename ExecutionPolicy, typename F>
    requires std::is_execution_policy_v<ExecutionPolicy>
    void for_each(ExecutionPolicy &&, concepts::tuple auto &&, F &&) {
        static_assert(false, "Unsupported execution policy");
    }

    void for_each(std::execution::sequenced_policy, concepts::tuple auto && value, auto f) {
        for_each(fwd(value), f);
    }
    // most likely, a very bad idea ?
    // consider some thread with cooperative cancelation -> std::stop_token
    void for_each(std::execution::parallel_policy, concepts::tuple auto && value, auto f) {
        constexpr auto size = csl::mp::size_v<decltype(value)>;
        std::vector<std::future<void>> tasks;
        tasks.reserve(size);
        // REFACTO: transform
        for_each(csl_fwd(value), [&](auto && element){
            tasks.push_back(
                std::async(std::launch::async, f, csl_fwd(element))
            );
        });
        // join by future destructor
    }
    void for_each(std::execution::parallel_unsequenced_policy&, concepts::tuple auto && value, auto f) {
        for_each(std::execution::par, csl_fwd(value), csl_fwd(f));
    }
}
#endif

// TODO(@Guss): fmt, std formatter

// TODO(@Guss): algos eDSL (range-like?) : (pipe, shift operators, plus, minus, etc...)

// ===================
// -- OLD, to refactor
// ===================

#if defined(CSL_MP__LEGACY) && CSL_MP__LEGACY

namespace csl::mp {
    // pack
    template <typename ... Ts>
    struct pack{};

    // is_pack
    template <typename T>
    struct is_pack : std::false_type{};
    template <typename ... Ts>
    struct is_pack<pack<Ts...>> : std::true_type{};
    template <typename T>
    constexpr inline static bool is_pack_v = is_pack<T>::value;
    template <typename T>
    concept PackType = is_pack_v<T>;

    // is_same_template
    template <typename, typename>
    struct is_same_template : std::false_type{};
    template <template <typename...> typename T, typename ... Ts, typename ... Us>
    struct is_same_template<T<Ts...>, T<Us...>> : std::true_type{};
    template <typename T, typename U>
    constexpr inline static bool is_same_template_v = is_same_template<T, U>::value;
    template <typename T, typename U>
    concept SameTemplateAs = is_same_template<T, U>::value;

    // is_instance_of
    template <template <typename ...> typename pack_type, typename T>
    struct is_instance_of : std::false_type{};
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct is_instance_of<pack_type, pack_type<Ts...>> : std::true_type{};
    template <template <typename ...> typename pack_type, typename T>
    constexpr inline static bool is_instance_of_v = is_instance_of<pack_type, T>::value;
    template <template <typename ...> typename pack_type, typename T>
    concept InstanceOf = is_instance_of_v<pack_type, T>;
}
// todo :
//  check `std::make_index_sequence` performances
//  if not acceptable
//      poc, remove & replace with smthg more efficient
namespace csl::mp::details {

    template <std::size_t I, typename T>
    struct element {
        constexpr static std::size_t index = I;
        using type = T;

        // index-to-type mapping
        constexpr static element<I, T> deduce_type(std::integral_constant<std::size_t, I>);
        // type-to-index mapping
        constexpr static element<I, T> deduce_index(type_identity<T>);
    };

    template <typename ... Ts>
    struct elements_pack : Ts... {
        using Ts::deduce_type...;
        using Ts::deduce_index...;

        template <std::size_t I>
        using nth_ = decltype(deduce_type(std::integral_constant<std::size_t, I>{}));
        template <typename T>
        using index_of_ = decltype(deduce_index(type_identity<T>{}));
    };

    template <typename ... Ts>
    struct make_elements_pack {
        using type = decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return elements_pack<element<indexes, Ts>...>{};
        }(std::make_index_sequence<sizeof...(Ts)>{}));
    };
    template <typename ... Ts>
    using make_elements_pack_t = typename make_elements_pack<Ts...>::type;

    #if false
    // required for previous `nth_t` implementation
    template <std::size_t I>
    struct is_element_match {
        template <typename>
        struct type : std::false_type{};
        template <typename T>
        struct type<element<I, T>> : std::true_type{};
    };
    #endif
}
// abstraction on (ttps...|pack<ttps...>)
namespace csl::mp {

    // front / head / first_of
    template <typename>
    struct front;
    template <template <typename ...> typename pack, typename T, typename ... Ts>
    struct front<pack<T, Ts...>> : type_identity<T>{};
    template <typename T>
    using front_t = typename front<T>::type;

    // filters
    // todo : remove dependency to std::tuple
    //  using csl::mp::push_back instead
    //  or Ts... -> pack<pack<T0>, pack<>, pack<T2> ...> => flatten
    template <template <typename> typename, typename>
    struct filters;
    template <template <typename> typename filter_type, template <typename...> typename pack, typename ... Ts>
    requires requires { ((filter_type<Ts>::value) and ...); }
    struct filters<filter_type, pack<Ts...>>
        : type_identity<decltype(std::tuple_cat(std::conditional_t< // todo : cat instead
            filter_type<Ts>::value,
            std::tuple<Ts>,
            std::tuple<>
        >{}...))>{}; // todo : repack tuple into pack

    template <template <typename> typename filter_type, typename pack>
    using filters_t = typename filters<filter_type, pack>::type;

    // nth element
    //  wip benchmarks : https://www.build-bench.com/b/gysn2DDajc38cdeUbNHnjmtKKvA
#if false
    // disabled, as csl::mp::filters_t relies on tuple for now
    template <std::size_t index, typename>
    struct nth_element;
    template <std::size_t index, template <typename ...> typename pack, typename ... Ts>
    requires (index < sizeof...(Ts))
    struct nth_element<index, pack<Ts...>> : front<
        filters_t<
            details::is_element_match<index>::template type,
            details::make_element_pack_t<Ts...>
        >
    >{};
    template <std::size_t index, typename pack>
    using nth_element_t = nth_element<index, pack>::type;
    template <std::size_t index, typename pack>
    using nth_t = nth_element_t<index, pack>::type;
    template <std::size_t index, typename pack>
    constexpr inline static std::size_t nth_v = nth_element_t<index, pack>::value;
#else
    // Indexed pack
    template <typename>
    struct pack_element;
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct pack_element<pack_type<Ts...>> : details::make_elements_pack_t<Ts...>{};

    template <std::size_t I, typename T>
    using pack_element_t = typename  pack_element<T>::template nth_<I>::type;
    template <std::size_t I, typename T>
    constexpr inline static std::size_t pack_element_v = I /*Note : equivalent to pack_element<I, T>::value;*/;

    template <std::size_t I, typename T>
    using nth_t = pack_element_t<I, T>;
    template <std::size_t I, typename T>
    constexpr inline static std::size_t nth_v = pack_element_v<I, T>;

    template <typename T, typename pack_type>
    constexpr inline static std::size_t index_of_v = pack_element<pack_type>::template index_of_<T>::index;
    // rindex_of -> index_of_v<T, reverse<pack_type>>
#endif

    // todo : index_of<T, pack_t>

    // todo : no, use csl::mp instead
    //  need to define csl::Pack concept
    template <typename T>
    concept TupleType = requires { csl::mp::size_v<T>; };

    // get<index>
    template <std::size_t index, typename ... Ts>
    constexpr decltype(auto) get(Ts && ... values) noexcept {
        return std::get<index>(std::tuple<decltype(values)...>{ csl_fwd(values)...});
    }
    template <std::size_t index, TupleType T>
    constexpr decltype(auto) get(T && value) noexcept {
        return std::get<index>(value);
    }
    template <std::size_t index, typename T, auto ... values>
    constexpr decltype(auto) get(std::integer_sequence<T, values...> && value) noexcept {
        return mp::seq::get<index>(csl_fwd(value));
    }

    // todo : remove
    // deprecated, use `nth_t` instead
    // template <std::size_t index, typename... Ts>
    // struct type_at : std::tuple_element<index, std::tuple<Ts...>>{};
    // template <std::size_t index, template <typename...> typename pack, typename ...Ts>
    // struct type_at<index, pack<Ts...>> : std::tuple_element<index, std::tuple<Ts...>>{};
    // template <std::size_t index, typename... Ts>
    // using type_at_t = type_at<index, Ts...>::type;

    #pragma region is_template
    // is_template : ttps
    template <typename ...>
    struct is_template_with_ttps : std::false_type{};
    template <template <typename...> typename pack_type, typename ... Ts>
    struct is_template_with_ttps<pack_type<Ts...>> : std::true_type{};
    template <typename ... Ts>
    constexpr inline static bool is_template_with_ttps_v = is_template_with_ttps<Ts...>::value;

    template <typename T>
    concept TemplateType_ttps = is_template_with_ttps_v<T>;

    // is_template : nttps
    template <typename ...>
    struct is_template_with_nttps : std::false_type{};
    template <template <auto...> typename pack_type, auto ... values>
    struct is_template_with_nttps<pack_type<values...>> : std::true_type{};
    template <typename ... Ts>
    constexpr inline static bool is_template_with_nttps_v = is_template_with_nttps<Ts...>::value;

    template <typename T>
    concept TemplateType_nttps = is_template_with_nttps_v<T>;

    // is_template
    template <typename ... Ts>
    struct is_template {
        constexpr static bool value = is_template_with_ttps_v<Ts...> or is_template_with_nttps_v<Ts...>;
    };
    template <typename ... Ts>
    constexpr inline static bool is_template_v = is_template<Ts...>::value;

    template <typename T>
    concept TemplateType = is_template_v<T>;
    #pragma endregion

    // size
    template <typename ... ttps>
    struct size : std::integral_constant<std::size_t, sizeof...(ttps)>{};
    template <template <typename...> typename pack, typename ... ttps>
    struct size<pack<ttps...>> : size<ttps...>{};
    template <typename ... ttps>
    constexpr inline static std::size_t size_v = size<ttps...>::value;

    // contains
    template <typename T, typename ... ttps>
    struct contains : std::bool_constant<(std::is_same_v<T, ttps> or ...)>{};
    template <typename T, template <typename...> typename pack, typename ... ttps>
    struct contains<T, pack<ttps...>> : contains<T, ttps...>{};
    template <typename T, typename ... ttps>
    constexpr inline static bool contains_v = contains<T, ttps...>::value;

    // count - occurences of T in ttps...
    template <typename T, typename ... ttps>
    struct count : std::integral_constant<std::size_t, (std::size_t{std::is_same_v<T, ttps>} + ...)>{};
    template <typename T, template <typename...> typename pack, typename ... ttps>
    struct count<T, pack<ttps...>> : count<T, ttps...>{};
    template <typename T, typename ... ttps>
    constexpr inline static std::size_t count_v = count<T, ttps...>::value;

    // cat / append
    template <typename, typename...>
    struct cat;
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, Us...> : type_identity<pack<Ts..., Us...>>{};
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, pack<Us...>> : type_identity<pack<Ts..., Us...>>{};
    template <typename pack, typename... ttps>
    using cat_t = cat<pack, ttps...>;

    // unfold_into
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_into : type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_into<destination, from<Ts...>> : type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_into_t = typename unfold_into<destination, from...>::type;

    // flat_cat
    template <typename, typename ...>
    struct flat_cat;
    template <template <typename...> typename pack_type, typename ... Ts, typename ... rest>
    struct flat_cat<pack_type<Ts...>, rest...>
    : type_identity<
        unfold_into_t<pack_type, decltype(std::tuple_cat(
            std::tuple<Ts...>{},
            std::conditional_t<
                is_instance_of_v<pack_type, rest>,
                unfold_into_t<std::tuple, rest>,
                std::tuple<rest>
            >{}...
        ))>
    >{};
    template <typename pack_type, typename ... Ts>
    using flat_cat_t = typename flat_cat<pack_type, Ts...>::type;

    // flat_merge : flat_cat + deduplication
    // todo

    // flatten_once
    template <typename T>
    struct flatten_once
    : type_identity<T>{};
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct flatten_once<pack_type<Ts...>>
    : type_identity<
        unfold_into_t<pack_type, decltype(std::tuple_cat(
            std::conditional_t<
                is_instance_of_v<pack_type, Ts>,
                unfold_into_t<std::tuple, Ts>,
                std::tuple<Ts>
            >{}...
        ))>
    >{};
    template <typename T>
    using flatten_once_t = typename flatten_once<T>::type;

    // flatten
    template <typename T>
    struct flatten;
    template <typename T>
    requires (std::same_as<T, flatten_once_t<T>>)
    struct flatten<T>
    : type_identity<T>{};
    template <typename T>
    struct flatten
    : type_identity<typename flatten<flatten_once_t<T>>::type>{};
    template <typename T>
    using flatten_t = typename flatten<T>::type;

    // reverse
    template <typename>
    class reverse;
    template <template <typename ...> typename pack_type, typename ... Ts>
    class reverse<pack_type<Ts...>> {
        constexpr static auto impl() {
            return [](){
                // todo : reverse, type_at
            }();
        }
    };
    template <typename T>
    using reverse_t = typename reverse<T>::type;

#ifdef false // TODO: refactor
    // rindex_of
    template <typename T, typename ... Ts>
    struct rindex_of {
        constexpr static std::size_t value = rindex_of<T, unfold_into_t<pack, Ts...>>::value;
    };
    template <typename T, template <typename...> typename pack, typename ... Ts>
    struct rindex_of<T, pack<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = details::npos;
            ((pos = (std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq details::npos, "rindex_of : not found");
    };
    template <typename T, typename ... Ts>
    constexpr inline static std::size_t rindex_of_v = rindex_of<T, Ts...>::value;

    // index_of
    template <typename T, typename ... Ts>
    struct index_of {
        constexpr static std::size_t value = index_of<T, unfold_into_t<pack, Ts...>>::value;
    };
    template <typename T, template <typename ...> typename pack, typename ... Ts>
    struct index_of<T, pack<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = details::npos;
            ((pos = (pos == details::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq details::npos, "index_of : not found");
    };
    template <typename T, typename ... Ts>
    constexpr inline static std::size_t index_of_v = index_of<T, Ts...>::value;

    template <typename T, typename ... Ts>
    using first_index_of = index_of<T, Ts...>;
    template <typename T, typename ... Ts>
    constexpr inline static std::size_t  first_index_of_v = first_index_of<T, Ts...>::value;

    template <typename T, typename ... Ts>
    using last_index_of = rindex_of<T, Ts...>;
    template <typename T, typename ... Ts>
    constexpr inline static std::size_t last_index_of_v = last_index_of<T, Ts...>::value;
#endif

    // at
    // todo

    // transform
    template <typename T, template <typename> typename trait>
    struct transform;
    template <typename ... Ts, template <typename> typename trait>
    struct transform<pack<Ts...>, trait> : type_identity<pack<trait<Ts>...>>
    {};
    template <typename T, template <typename> typename trait>
    using transform_t = typename transform<T, trait>::type;

    // deduplicate
    //  todo : refactor :
    //      contains => index_of not_eq rindex_of
    template <typename ...>
    struct deduplicate_impl;
    template <typename T>
    struct deduplicate_impl<T> {
        using type = std::tuple<T>;
    };
    template <typename first, typename ... rest>
    struct deduplicate_impl<first, rest...> {
        using type = decltype(std::tuple_cat(
            std::conditional_t<
                contains_v<first, std::tuple<rest...>>,
                std::tuple<>,
                std::tuple<first>
            >{},
            typename deduplicate_impl<rest...>::type{}
        ));
    };

    template <typename T>
    class deduplicate;
    template <template <typename...> typename type_pack, typename ... Ts>
    class deduplicate<type_pack<Ts...>> {

        using tuple_type = typename deduplicate_impl<Ts...>::type;
    public:
        using type = unfold_into_t<type_pack, tuple_type>;
    };
    template <typename T>
    using deduplicate_t = typename deduplicate<T>::type;
}
#endif

#undef csl_fwd
#undef csl_compiler_is_clang
#undef csl_compiler_is_gcc
#undef csl_compiler_is_msvc
#undef csl_compiler_is_unknown

// wip : https://godbolt.org/z/TfMqM5TaG
// wip, benchmarked : https://godbolt.org/z/fj4z8sjzh
// wip bench : https://www.build-bench.com/b/lADLAH3QR2OEHMbbDVB2wkssuVg
