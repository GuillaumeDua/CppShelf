#pragma once
// cpp shelf library : metaprogramming utility
// under MIT License - Copyright (c) 2021 Guillaume Dua "Guss"
// https://github.com/GuillaumeDua/CppShelf/blob/main/LICENSE

// About [tuple]:
//  A given tuple T is considered valid if csl::mp::concepts::tuple<T> evaluates to true (csl::mp::is_tuple is std::true_type), and contains no duplicates.
//  If a given tuple type T instanciation is not valid, less performant algorithms may be selected to provide similar functionalities, as a best-effort.

// About [algorithms]:
//  All algorithms are partitioned in two groups:
//      [type-traits]: contains, count/count_if, find/find_if, etc.
//      [functions]: tuple_cat, etc.


#if not __cplusplus >= 202002L
# error "csl/mp.hpp requires C++20 or greater"
#endif

#include <compare>
#include <type_traits>
#include <concepts>
#include <algorithm>
#include <functional>

#define csl_fwd(...) static_cast<decltype(__VA_ARGS__) &&>(__VA_ARGS__)                     // NOLINT(cppcoreguidelines-macro-usage)
// deprecated by P2593R0 - Allowing static_assert(false)
#define csl_static_dependent_error(message) static_assert([](){ return false; }(), message) // NOLINT(cppcoreguidelines-macro-usage)

#if defined(__clang__)
#   define csl_compiler_is_clang
#elif defined(__GNUC__) || defined(__GNUG__)
#   define csl_compiler_is_gcc
#elif defined(_MSC_VER)
#   define csl_compiler_is_msvc
#else
#   define csl_compiler_is_unknown
#endif

// WIP refactoring => reorder/sort
//  sequences::* : op on sequences
//  seq types    : sequences definitions
//  tuple
//  mp::pack::* : ttps...| tttp<ttps...> -> only mp::*<tttp<ttps...>>
//
// WIP range-like API for sequence, tuple
//  views: std::make_index_sequence<4> | reverse | drop(1) | take(2) => std::integer_sequence<std::size_t, 3, 2>
//  ranges: fold, flatten, partition, split, etc.
//
// TODO: range-like tuple ? operator[], size, empty, etc.
//
// TODO: decouple-from/remove std::index_sequence, std::make_sequence

// --- sequence ---

#include <array>
namespace csl::mp::seq {

    // is_sequence
    template <typename T>
    struct is_sequence : std::false_type{};
    template <typename T, T ... values>
    struct is_sequence<std::integer_sequence<T, values...>> : std::true_type{};
    template <typename T>
    constexpr static inline auto is_sequence_v = is_sequence<T>::value;

    namespace concepts {
        template <typename T>
        concept sequence = is_sequence_v<T>;
    }

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

    // size | pref. seq::size()
    template <typename T>
    struct size;
    template <typename T, T ... values>
    struct size<std::integer_sequence<T, values...>>
    : std::integral_constant<std::size_t, sizeof...(values)>
    {};

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

    // reverse
    template <typename T>
    struct reverse;
    template <concepts::sequence T>
    struct reverse<T> : std::type_identity<
        decltype(
            []<std::size_t ... indexes>(std::index_sequence<indexes...>){
                return std::integer_sequence<
                    typename T::value_type,
                    at_v<T::size() - 1 - indexes, T>...
                >{};
            }(std::make_index_sequence<T::size()>{})
        )
        // if 0..N: std::integer_sequence<T, (sizeof...(values) - 1 - values)...>
    >{};
    // template <typename T, T ... values>
    // struct reverse<std::integer_sequence<T, values...>> : std::type_identity<
    //     decltype(
    //         []<std::size_t ... indexes>(std::index_sequence<indexes...>){
    //             constexpr auto storage = std::array{ values... };
    //             return std::integer_sequence<
    //                 T,
    //                 std::get<(sizeof...(values) - 1 - indexes)>(storage)...
    //             >{};
    //         }(std::make_index_sequence<sizeof...(values)>{})
    //     )
    //     // if 0..N: std::integer_sequence<T, (sizeof...(values) - 1 - values)...>
    // >{};
    template <typename T>
    using reverse_t = reverse<T>::type;
    template <std::size_t I>
    using make_reverse_index_sequence = reverse_t<std::make_index_sequence<I>>;

    // type_of<values...>
    template <auto value, auto ... values>
    requires (std::is_same_v<decltype(value), decltype(values)> and ...)
    struct type_of : std::type_identity<decltype(value)>{};
    template <auto ... values>
    using type_of_t = typename type_of<values...>::value;
    
    // get<index>(seq)
    template <std::size_t index, typename T, T ... values>
    constexpr decltype(auto) get(std::integer_sequence<T, values...>) noexcept {
        return at_v<index, std::integer_sequence<T, values...>>;
    }
}

// --- tuple ---

// P2165 - tuple-like
namespace csl::mp::concepts::inline P2165 {
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
        and details::valid_tuple_elements_v<T>
        // and []<std::size_t... I>(std::index_sequence<I...>) constexpr {
        //     return (tuple_element<T, I> && ...);
        // }(std::make_index_sequence<std::tuple_size_v<T>>{})
    ;
    template <typename T>
    concept pair_like = tuple_like<T> and std::tuple_size_v<T> == 2;
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
    template <typename T> struct add_const : std::type_identity<const T>{};
    template <typename T> struct add_const<T&> : std::type_identity<const T&>{};
    template <typename T> struct add_const<T&&> : std::type_identity<const T&&>{};
    template <typename T> using add_const_t = typename add_const<T>::type;

    template <typename T> struct add_volatile : std::type_identity<volatile T>{};
    template <typename T> struct add_volatile<T&> : std::type_identity<volatile T&>{};
    template <typename T> struct add_volatile<T&&> : std::type_identity<volatile T&&>{};
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
#pragma region __detail::__synth3way_t
// see https://en.cppreference.com/w/cpp/standard_library/synth-three-way
namespace csl::mp::details::compare {

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
#pragma endregion

namespace csl::mp::details {

    // Drop-in replacement for std::tuple:
    //  As <tuple> is -isystem, implicit casts do not produce warnings
    //  The cmake option and pp-definition `CSL_MP_TUPLE__IMPLICIT_CONVERSION` toggles this behavior on/off
    template <typename T>
    [[nodiscard]] constexpr static auto fwd_maybe_cast(std::convertible_to<T> auto && value) {
        return static_cast<
            #if defined(CSL_MP_TUPLE__IMPLICIT_CONVERSION) \
                    and CSL_MP_TUPLE__IMPLICIT_CONVERSION
            T
            #else
            decltype(value)
            #endif
            >(value)
        ;
    }

    // Associate an index and a type with a value
    // - mp::index_t<I>         lookup by index
    // - mp::type_identity<T>   lookup by type
    template <std::size_t I, typename T>
    struct tuple_member {
        constexpr static std::size_t index = I;
        using type = T;

        T value;

        // index-to-type mapping
        constexpr static tuple_member<I, T> deduce_type(mp::index_t<I>) noexcept;
        // type-to-index mapping (repetitions: clashes are handled downstream)
        constexpr static tuple_member<I, T> deduce_index(mp::type_identity<T>) noexcept;
    };
    template <std::size_t I, typename T>
    constexpr static auto tuple_element_index = tuple_member<I, T>::index;

    // [[nodiscard]] constexpr static
    // auto tuple_member_value(/*constrained: instance_of<tuple_member>*/ auto && te) noexcept -> decltype(auto) {
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
    T && tuple_member_value(tuple_member<I, T> && te) noexcept { return static_cast<T&&>(te.value); }
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

    // conversion support
        template <typename ...>
        friend struct tuple_storage;

        template <typename ... Us>
        constexpr explicit tuple_storage(Us && ... args) // NOLINT(*-missing-std-forward)
        noexcept((true and ... and std::is_nothrow_constructible_v<Ts, Us&&>))
        requires (sizeof...(Ts) == sizeof...(Us)
            and (true and ... and std::constructible_from<Ts, Us&&>)
        )
        : tuple_member<indexes, Ts>{
            fwd_maybe_cast<Ts>(csl_fwd(args))
        }...
        {}

    #if defined(csl_compiler_is_gcc) // up to at least gcc-13.3.0
        // quick-fix: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=120500
        //  MVE: https://godbolt.org/z/8oEW71xv8
        template <std::size_t ... indexes_, typename ... Us>
        constexpr explicit
        tuple_storage(tuple_storage<tuple_member<indexes_, Us>...> && other)
    #else
        template <typename ... Us>
        constexpr explicit
        tuple_storage(tuple_storage<tuple_member<indexes, Us>...> && other)
    #endif
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

    // tuple_size
    template <typename>
    struct tuple_size;
    template <typename ... Ts>
    struct tuple_size<tuple<Ts...>> : std::integral_constant<
        std::size_t, sizeof...(Ts)
    >{};
    template <typename tuple_type>
    constexpr std::size_t tuple_size_v = tuple_size<tuple_type>::value;

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
    and (tuple_size_v<T> == tuple_size_v<U>)
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
namespace std {

    template <
        typename ... Ts,
        typename ... Us,
        template <typename> class TQual,
        template <typename> class UQual
    >
    requires
        (sizeof...(Ts) == sizeof...(Us))
    and (true and ... and common_reference_with<Ts, Us>)
    struct basic_common_reference< // NOLINT(cert-dcl58-cpp)
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
}

namespace csl::mp {

    template <typename ... Ts>
    struct tuple
    {
        template <typename ... Us> friend struct tuple;

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
                return ((get<indexes>() = csl_fwd(other).template get<indexes>()), ...);
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

        // TODO(Guillaume): #285 - interop with other tuple-like (pair, array, etc.)

        // NOLINTBEGIN(*explicit-constructor) conditionaly explicit
        // Constructor: direct
        constexpr explicit
        tuple(const Ts & ... args)
        noexcept((std::is_nothrow_constructible_v<Ts, const Ts &> and ...))
        requires
            (sizeof...(Ts) not_eq 0) // disambiguate with default constructor
        and (std::constructible_from<Ts, const Ts &> and ...)
        : storage{ csl_fwd(args)... }
        {}
        // Constructor: converting (values...)
        template <typename ... Us>
        constexpr explicit(not (true and ... and std::convertible_to<Ts, Us&&>))
        tuple(Us && ... args)
        noexcept((std::is_nothrow_constructible_v<Ts, Us&&> and ...))
        requires
            (sizeof...(Ts) not_eq 0) // disambiguate with default constructor
        and (sizeof...(Ts) == sizeof...(Us))
        and (std::constructible_from<Ts, Us&&> and ...)
        : storage{ csl_fwd(args)... }
        {}
        // Constructor: converting move
        template <typename ... Us>
        constexpr explicit(not (true and ... and std::convertible_to<Ts, Us&&>))
        tuple(tuple<Us...> && other)
        noexcept(std::is_nothrow_constructible_v<decltype(storage), decltype(std::move(other.storage))>)
        requires 
            (sizeof...(Ts) == sizeof...(Us))
        and (true and ... and std::constructible_from<Ts, Us&&>)
        : storage{ std::move(other.storage) }
        {}
        // Constructor: converting copy
        template <typename ... Us>
        constexpr explicit(not (true and ... and std::convertible_to<Ts, const Us &>))
        tuple(const tuple<Us...> & other)
        noexcept(std::is_nothrow_constructible_v<decltype(storage), decltype(other.storage)>)
        requires (true and ... and std::constructible_from<Ts, const Us&>)
        : storage{ other.storage }
        {}
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

    // WIP ...

    // storage accessors
        // WIP: use tuple_member_value ?
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
namespace csl::mp::details {
    constexpr static std::size_t npos = static_cast<std::size_t>(-1);
}

// TODO(Guillaume): concepts::tuple<T> everywhere below ?
namespace csl::mp {

    // type-by-index
    template <std::size_t, typename>
    struct tuple_element;
    template <std::size_t index, typename ... Ts>
    struct tuple_element<index, tuple<Ts...>> : tuple<Ts...>::storage_type::template by_index_<index>{};
    template <std::size_t index, concepts::tuple tuple_type>
    using tuple_element_t = typename tuple_element<index, tuple_type>::type;

    template <std::size_t index, concepts::tuple T>
    using nth = tuple_element_t<index, T>;

    // is_valid_tuple
    template <typename>
    struct is_valid_tuple : std::false_type{};
    template <typename ... Ts>
    struct is_valid_tuple<tuple<Ts...>> : std::bool_constant<
    // no duplicates types
        (true and ... and details::concepts::can_deduce_by_type<tuple<Ts...>, Ts>)
    >{};
    template <typename T>
    constexpr bool is_valid_tuple_v = is_valid_tuple<T>::value;

    namespace concepts {
        template <typename T> concept valid_tuple = is_valid_tuple_v<T>;
    }

    // is_homogeneous
    template <typename T>
    struct is_homogeneous_tuple : std::false_type{};
    template <typename T0, typename ... Ts>
    struct is_homogeneous_tuple<tuple<T0, Ts...>> : std::bool_constant<(
        true and ... and std::same_as<T0, Ts>
    )>{};
    template <typename T>
    constexpr bool is_homogeneous_tuple_v = is_homogeneous_tuple<T>::value;

    namespace concepts {
        template <typename T> concept homogeneous_tuple = is_homogeneous_tuple_v<T>;
    }

    // is_constrained_by_tuple
    template <typename, template <typename> typename>
    struct is_constrained_by_tuple : std::false_type{};
    template <template <typename> typename predicate>
    struct is_constrained_by_tuple<tuple<>, predicate> : std::false_type{};
    template <template <typename> typename predicate, typename ... Ts>
    struct is_constrained_by_tuple<tuple<Ts...>, predicate> : std::bool_constant<
        (true and ... and predicate<Ts>::value)
    >{};
    template <typename T, template <typename> typename predicate>
    constexpr auto is_constrained_by_tuple_v = is_constrained_by_tuple<T, predicate>::value;

    namespace concepts {
        template <typename T, template <typename> typename predicate>
        concept constrained_by_tuple = concepts::tuple<T> and is_constrained_by_tuple_v<T, predicate>;
    }

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

    // TODO(Guillaume) naming: namespace tuple_traits::is_valid, etc. ? ⬆️⬇️

    // empty
    template <typename>
    struct empty;
    template <>
    struct empty<tuple<>> : std::true_type{};
    template <typename T0, typename ... Ts>
    struct empty<tuple<T0, Ts...>> : std::false_type{};
    template <typename T>
    constexpr bool empty_v = empty<T>::value;

    namespace concepts {
        template <typename T>
        concept empty_tuple = tuple<T> and empty_v<T>;
    }

    // count
    template <typename, typename>
    struct count;
    template <typename T, typename ... Ts>
    struct count<T, tuple<Ts...>> : std::integral_constant<std::size_t,
        (0 + ... + std::is_same_v<T, Ts>)
    >{};
    template <typename T, typename tuple_type>
    constexpr std::size_t count_v = count<T, tuple_type>::value;

    // count_if
    template <template <typename...> typename, typename>
    struct count_if;
    template <template <typename...> typename trait, typename ... Ts>
    struct count_if<trait, csl::mp::tuple<Ts...>> : std::integral_constant<std::size_t,
        (0 + ... + trait<Ts>::value)
    >{};
    template <template <typename...> typename trait, typename tuple_type>
    constexpr std::size_t count_if_v = count_if<trait, tuple_type>::value;

    // index-by-type
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct index_of : std::integral_constant<std::size_t,
        tuple_type::storage_type::template by_type_<T>::index
    >{};
    template <typename T, concepts::valid_tuple tuple_type>
    constexpr std::size_t index_of_v = index_of<T, tuple_type>::value;

    // unfold_into
    template <template <typename...> typename, typename>
    struct unfold_into;
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_into<destination, tuple<Ts...>> : type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, typename tuple_type>
    using unfold_into_t = typename unfold_into<destination, tuple_type>::type;

    // transform
    template <template <typename> typename, typename>
    struct transform;
    template <template <typename> typename trait, typename ... Ts>
    struct transform<trait, csl::mp::tuple<Ts...>> : type_identity<
        typename csl::mp::tuple<typename trait<Ts>::type...>
    >{};
    template <template <typename> typename trait, typename tuple_type>
    using transform_t = typename transform<trait, tuple_type>::type;

    // tuple_cat
    constexpr auto tuple_cat(){ return csl::mp::tuple{}; }
    constexpr auto tuple_cat(/* TODO: tuplelike */ auto && ... tuples)
    requires (sizeof...(tuples) not_eq 0)
    {
        constexpr auto size = (... + csl::mp::tuple_size_v<std::remove_cvref_t<decltype(tuples)>>);

        if constexpr (size == 0)
            return csl::mp::tuple<>{};
        else
        {
            // TODO(Guillaume): use `make_two_dimension_index` from the old `flatten` poc ?
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
                    csl::mp::tuple_size_v<std::remove_cvref_t<decltype(tuples)>>
                >{})), ...);

                return mapped_indexes;
            }();

            using tuple_of_tuples_t = csl::mp::tuple<std::remove_cvref_t<decltype(tuples)>...>;
            return []<std::size_t ... indexes>(tuple_of_tuples_t && tuple_of_tuples, std::index_sequence<indexes...>){

                using type = csl::mp::tuple<
                    csl::mp::tuple_element_t<
                        indexes_map.element_index[indexes],
                        std::remove_cvref_t<csl::mp::tuple_element_t<
                            indexes_map.tuple_index[indexes],
                            tuple_of_tuples_t
                        >>
                    >...
                >;
                return type{
                    get<indexes_map.element_index[indexes]>(
                        get<indexes_map.tuple_index[indexes]>(std::move(tuple_of_tuples))
                    )...
                };
            }(
                tuple_of_tuples_t{ csl_fwd(tuples)... },
                std::make_index_sequence<size>{}
            );
        }
    }

    // tuple_cat_result
    template <typename ... tuple_types>
    struct tuple_cat_result : csl::mp::type_identity<
        decltype(tuple_cat(std::declval<tuple_types>()...))
    >{};
    template <typename ... tuple_types>
    using tuple_cat_result_t = typename tuple_cat_result<tuple_types...>::type;

    // contains
    template <typename, typename>
    struct contains;
    // build benchmark (~ +20% perfs): https://build-bench.com/b/Ir0K0cw2wfFLyRYPYYIMVASDYQU
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct contains<T, tuple_type> : std::true_type{};
    template <typename T, typename ... Ts>
    struct contains<T, tuple<Ts...>> : std::bool_constant<
        (std::same_as<T, Ts> or ...)
    >{};
    template <typename T, typename tuple_type>
    constexpr bool contains_v = contains<T, tuple_type>::value;

    // has_duplicates
    template <typename>
    struct has_duplicates;
    template <typename ... Ts>
    struct has_duplicates<tuple<Ts...>> : std::negation<is_valid_tuple<tuple<Ts...>>>{};
    template <typename tuple_type>
    constexpr bool has_duplicates_v = has_duplicates<tuple_type>::value;

    // first_index_of
    template <typename, typename>
    struct first_index_of;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct first_index_of<T, tuple_type> : index_of<T, tuple_type>{};
    template <typename T, typename ... Ts>
    requires (not details::concepts::can_deduce_by_type<tuple<Ts...>, T>)
    struct first_index_of<T, tuple<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = details::npos;
            (void)((pos = (pos == details::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq details::npos, "first_index_of : not found");
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t first_index_of_v = first_index_of<T, tuple_type>::value;

    // rfirst_index_of
    template <typename, typename>
    struct rfirst_index_of;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct rfirst_index_of<T, tuple_type> : std::bool_constant<
        tuple_size_v<tuple_type> - 1 - index_of_v<T, tuple_type>
    >{};
    template <typename T, typename ... Ts>
    requires (not details::concepts::can_deduce_by_type<tuple<Ts...>, T>)
    struct rfirst_index_of<T, tuple<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = details::npos;
            (void)((pos = (pos == details::npos and std::is_same_v<T, Ts> ? indexes : pos)), ...);
            return pos;
        }(csl::mp::seq::make_reverse_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq details::npos, "rfirst_index_of : not found");
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t rfirst_index_of_v = rfirst_index_of<T, tuple_type>::value;

    // last_index_of
    //  equivalent to (sizeof...(Ts) - rfirst_index_of<T, tuple<Ts...>>)
    template <typename, typename>
    struct last_index_of;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct last_index_of<T, tuple_type> : index_of<T, tuple_type>{};
    template <typename T, typename ... Ts>
    requires (not details::concepts::can_deduce_by_type<tuple<Ts...>, T>)
    struct last_index_of<T, tuple<Ts...>> {
        constexpr static std::size_t value = []<std::size_t ... indexes>(std::index_sequence<indexes...>){
            std::size_t pos = details::npos;
            (void)((pos = std::is_same_v<T, Ts> ? indexes : pos), ...);
            return pos;
        }(std::make_index_sequence<sizeof...(Ts)>{});
        static_assert(value not_eq details::npos, "last_index_of : not found");
    };
    template <typename T, typename tuple_type>
    constexpr std::size_t last_index_of_v = last_index_of<T, tuple_type>::value;

    // set_union
    template <typename, typename>
    struct set_union;
    template <typename ... Ts, typename ... Us>
    struct set_union<tuple<Ts...>, tuple<Us...>> : std::type_identity<
        decltype(tuple_cat(
            tuple<Ts...>{},
            std::conditional_t<
                contains_v<Us, tuple<Ts...>>,
                tuple<>,
                tuple<Us>
            >{}...
        ))
    >{};
    template <typename T, typename U>
    using set_union_t = typename set_union<T, U>::type;

    // set_intersection
    template <typename, typename>
    struct set_intersection;
    template <typename ... Ts, typename ... Us>
    struct set_intersection<tuple<Ts...>, tuple<Us...>> : std::type_identity<
        decltype(tuple_cat(
            std::conditional_t<
                contains_v<Us, tuple<Ts...>>,
                tuple<Us>,
                tuple<>
            >{}...
        ))
    >{};
    template <typename T, typename U>
    using set_intersection_t = typename set_intersection<T, U>::type;

    // is_unique
    template <typename, typename>
    struct is_unique;
    template <typename T, details::concepts::can_deduce_by_type<T> tuple_type>
    struct is_unique<T, tuple_type> : std::true_type{};
    template <typename T, typename ... Ts>
    requires (not contains_v<T, tuple<Ts...>>)
    struct is_unique<T, tuple<Ts...>> : std::false_type{};
    template <typename T, typename ... Ts>
    struct is_unique<T, tuple<Ts...>> : std::bool_constant<
        (first_index_of_v<T, tuple<Ts...>> == last_index_of_v<T, tuple<Ts...>>)
    >{};
    template <typename T, typename tuple_type>
    constexpr bool is_unique_v = is_unique<T, tuple_type>::value;

    // filter<trait>
    template <typename, template <typename...> typename>
    struct filter;
    template <typename ... Ts, template <typename...> typename predicate>
    struct filter<tuple<Ts...>, predicate> : tuple_cat_result<
        std::conditional_t<
            predicate<Ts>::value,
            tuple<Ts>, tuple<>
        >...
    >{};
    template <concepts::tuple tuple_type, template <typename...> typename predicate>
    using filter_t = typename filter<tuple_type, predicate>::type;

    // deduplicate / make_valid
    // TODO(Guss): reverse prior to filtering, to preserve order
    template <typename>
    struct deduplicate;
    template <concepts::valid_tuple T>
    struct deduplicate<T> : type_identity<T>{};
    template <typename ... Ts>
    struct deduplicate<tuple<Ts...>> : 
    // equivalent to filter<tuple<Ts...>, bind_back<is_unique, tuple<Ts...>>::type>{};
    //
    //  tuple_cat_result<
    //     std::conditional_t<
    //         is_unique_v<Ts, tuple<Ts...>>,
    //         tuple<Ts>, tuple<>
    //     >...
    // >{};
    decltype([]<std::size_t ... indexes>(std::index_sequence<indexes...>){
        return tuple_cat_result<
            std::conditional_t<
                details::concepts::can_deduce_by_type<tuple<Ts...>, Ts>
                or indexes == first_index_of_v<Ts, tuple<Ts...>>,
                tuple<Ts>, tuple<>
            >...
        >{};
    }(std::make_index_sequence<tuple_size_v<tuple<Ts...>>>{})){};
    template <concepts::tuple tuple_type>
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

    // apply

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

    // size
    // - tuple<...>::size
    // - tuple_size

    // tuple_element

    template <std::size_t index>
    [[nodiscard]] constexpr auto get(concepts::tuple auto && value) noexcept -> decltype(auto) {
        return csl_fwd(value).template get<index>();
    }
    // WIP

    // make_tuple
    // forward_as_tuple
}

#pragma region std inter-operatiblity, structured binding

//  N4606 [namespace.std]/1 :
//      A program may add a template specialization for any standard library template to namespace std
//      only if the declaration depends on a user-defined type 
//      and the specialization meets the standard library requirements for the original template and is not explicitly prohibited.

#include <utility> // std::tuple_size, std::tuple_element

// NOLINTBEGIN(cert-dcl58-cpp) Modification of 'std' namespace can result in undefined behavior
template <csl::mp::concepts::tuple tuple_type>
struct std::tuple_size<tuple_type> : csl::mp::tuple_size<tuple_type>{};

template <std::size_t index, csl::mp::concepts::tuple tuple_type>
struct std::tuple_element<index, tuple_type> : csl::mp::tuple_element<index, tuple_type>{};
// NOLINTEND(cert-dcl58-cpp)
#pragma endregion

// tuple algorithms functions
namespace csl::mp::inline functions {

    // foreach
    void for_each(concepts::tuple auto && value, auto f){
        constexpr auto size = csl::mp::tuple_size_v<std::remove_cvref_t<decltype(value)>>;
        [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            ((
                std::invoke(f, std::get<indexes>(csl_fwd(value)))
            ), ...);
        }(std::make_index_sequence<size>{});
    }

    // apply

    template <concepts::tuple T>
    [[nodiscard]] constexpr auto all_of(const T & value, std::predicate auto && p){
        // REFACTO: reduce
        return [&]<std::size_t ... indexes>(std::index_sequence<indexes...>){
            return (true and ... and std::invoke(p, get<indexes>(value)));
        }(std::make_index_sequence<csl::mp::tuple_size_v<T>>{});
    }
    // TODO(Guillaume) any_of, none_of

    // split
    // chunk_by: <N>, predicate
}

namespace csl::mp::inline type_traits {
    template <template <typename ...> typename trait, typename ... Ts>
    struct bind_front {
        template <typename ... Us>
        using type = trait<Ts..., Us...>;
    };
    template <template <typename ...> typename trait, typename ... Ts>
    struct bind_back {
        template <typename ... Us>
        using type = trait<Us..., Ts...>;
    };
}

// TODO(@Guss): fmt, std formatter

// TODO(@Guss): algos eDSL (range-like?) : (pipe, shift operators, plus, minus, etc...)

// ===================
// -- OLD, to refactor
// ===================

#if false

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
        constexpr static element<I, T> deduce_index(std::type_identity<T>);
    };

    template <typename ... Ts>
    struct elements_pack : Ts... {
        using Ts::deduce_type...;
        using Ts::deduce_index...;

        template <std::size_t I>
        using nth_ = decltype(deduce_type(std::integral_constant<std::size_t, I>{}));
        template <typename T>
        using index_of_ = decltype(deduce_index(std::type_identity<T>{}));
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
    struct front<pack<T, Ts...>> : std::type_identity<T>{};
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
        : std::type_identity<decltype(std::tuple_cat(std::conditional_t< // todo : cat instead
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
    concept TupleType = requires { std::tuple_size_v<T>; };

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
    struct cat<pack<Ts...>, Us...> : std::type_identity<pack<Ts..., Us...>>{};
    template <
        template <typename...> typename pack,
        typename ... Ts,
        typename ... Us
    >
    struct cat<pack<Ts...>, pack<Us...>> : std::type_identity<pack<Ts..., Us...>>{};
    template <typename pack, typename... ttps>
    using cat_t = cat<pack, ttps...>;

    // unfold_into
    template <template <typename...> typename destination, typename ... Ts>
    struct unfold_into : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, template <typename...> typename from, typename ... Ts>
    struct unfold_into<destination, from<Ts...>> : std::type_identity<destination<Ts...>>{};
    template <template <typename...> typename destination, typename ... from>
    using unfold_into_t = typename unfold_into<destination, from...>::type;

    // flat_cat
    template <typename, typename ...>
    struct flat_cat;
    template <template <typename...> typename pack_type, typename ... Ts, typename ... rest>
    struct flat_cat<pack_type<Ts...>, rest...>
    : std::type_identity<
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
    : std::type_identity<T>{};
    template <template <typename ...> typename pack_type, typename ... Ts>
    struct flatten_once<pack_type<Ts...>>
    : std::type_identity<
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
    : std::type_identity<T>{};
    template <typename T>
    struct flatten
    : std::type_identity<typename flatten<flatten_once_t<T>>::type>{};
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
    struct transform<pack<Ts...>, trait> : std::type_identity<pack<trait<Ts>...>>
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
#undef csl_static_dependent_error
#undef csl_compiler_is_clang
#undef csl_compiler_is_gcc
#undef csl_compiler_is_msvc
#undef csl_compiler_is_unknown

// wip : https://godbolt.org/z/TfMqM5TaG
// wip, benchmarked : https://godbolt.org/z/fj4z8sjzh
// wip bench : https://www.build-bench.com/b/lADLAH3QR2OEHMbbDVB2wkssuVg
